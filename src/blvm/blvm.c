#include "blvm/blvm.h"

void blvm_clean(Blvm *bl) {
	free(bl->program), bl->program_size = 0;
	free(bl->memory), bl->memory_capacity = 0;
}

bool blvm_load_program_from_file(Blvm *bl, const char *fpath) {
	FILE *file = NULL;

	if( (file = fopen(fpath, "rb")) == NULL ) {
		fprintf(stderr, "ERROR: Could not open file '%s': %s\n", fpath, strerror(errno));
		return false;
	}

	BlMeta meta = {0};
	assert(fread(&meta, sizeof(BlMeta), 1, file) == 1);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not read metadata from file '%s': %s\n", fpath, strerror(errno));
		return false;
	}

	if( meta.magic != BL_MAGIC) {
		fprintf(stderr, "ERROR: '%s' is not a valid file format (magic != %u).\n", fpath, BL_MAGIC);
		return false;
	}

	if( meta.version != BL_VERSION ) {
		fprintf(stderr, "ERROR: Unsupported file version %d.\n", meta.version);
		return false;
	}

	bl->program_size = meta.program_size;
	bl->memory_capacity = meta.memory_capacity;
	bl->ip = meta.entry_point;

	bl->program = (Inst*)malloc(bl->program_size * sizeof(struct inst_t));
	bl->memory = (uint8_t*)malloc(bl->memory_capacity * sizeof(uint8_t));

	assert(fread(bl->program, sizeof(struct inst_t), bl->program_size, file) == bl->program_size);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not read from file '%s': %s\n", fpath, strerror(errno));
		return false;
	}

	assert(fread(bl->memory, sizeof(uint8_t), meta.memory_size, file) == meta.memory_size);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not read from file '%s': %s\n", fpath, strerror(errno));
		return false;
	}

	fclose(file);

	return true;
}

Trap blvm_push_native(Blvm *bl, NativeFunctionCall func) {
	if( bl->ns >= BLISP_NATIVES_CAPACITY )
		return TRAP_NATIVES_OVERFLOW;

	bl->natives[bl->ns++] = func;

	return TRAP_OK;
}

void blvm_dump_stack(const Blvm *bl, FILE *stream) {
	fprintf(stream, "Stack:\n");
	if( bl->sp > 0) {
		for(uint64_t i = 0; i < bl->sp; ++i)
			fprintf(stream, "\t%lu %ld %0.12lf %p\n", bl->stack[i].u64, bl->stack[i].i64, bl->stack[i].f64, bl->stack[i].ptr);
	} else {
		fprintf(stream, "\t[empty]\n");
	}
}

void blvm_dump_memory(const Blvm *bl, FILE *stream) {
	fprintf(stream, "Memory (capacity: %lu):\n", bl->memory_capacity);
	for(size_t i = 0; i < bl->memory_capacity; i++) {
		fprintf(stream, "%x ", bl->memory[i]);
	}
	fprintf(stream, "\n");
}

void blvm_show_state(const Blvm *bl, FILE *stream) {
		fprintf(stream, "0x%03lX: %s", bl->ip, inst_names[bl->program[bl->ip].type]);

		if( inst_required_operand[bl->program[bl->ip].type] )
			fprintf(stream, " %lu", bl->program[bl->ip].operand.u64);

		fprintf(stream, " (sp: 0x%03lX)\n", bl->sp);
}

#ifndef OPERATION
#define OPERATION(bl, in, out, op)					\
	do {								\
		if( (bl)->sp < 2 )					\
			return TRAP_STACK_UNDERFLOW;			\
									\
		(bl)->stack[(bl)->sp - 2].out = (bl)->stack[(bl)->sp - 2].in op (bl)->stack[(bl)->sp - 1].in;	\
		(bl)->sp -= 1;						\
		(bl)->ip += 1;						\
	} while(false)
#endif

#ifndef CAST
#define CAST(bl, src, dst, cast) \
	do {										\
		if( (bl)->sp < 1 )							\
			return TRAP_STACK_UNDERFLOW;					\
											\
		(bl)->stack[(bl)->sp - 1].dst = cast (bl)->stack[(bl)->sp - 1].src;	\
											\
		(bl)->ip += 1;								\
	} while(false);
#endif

Trap blvm_execute_inst(Blvm *bl) {
	if( bl->ip >= bl->program_size )
		return TRAP_ILLEGAL_INST_ACCESS;

	Inst inst = bl->program[bl->ip];
	switch( inst.type ) {
		case INST_NOP:
			bl->ip += 1;
			break;

		case INST_PUSH:
			if( bl->sp >= BLISP_STACK_CAPACITY )
				return TRAP_STACK_OVERFLOW;

			bl->stack[bl->sp++] = inst.operand;
			bl->ip += 1;
			break;

		case INST_POP:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_SWAP:
			if( inst.operand.u64 >= bl->sp )
				return TRAP_STACK_UNDERFLOW;

			const uint64_t a = bl->sp - 1;
			const uint64_t b = bl->sp - 1 - inst.operand.u64;

			Word tmp = bl->stack[a];
			bl->stack[a] = bl->stack[b];
			bl->stack[b] = tmp;

			bl->ip += 1;
			break;

		case INST_DUP:
			if( bl->sp - inst.operand.u64 <= 0 )
				return TRAP_STACK_UNDERFLOW;
			if( bl->sp >= BLISP_STACK_CAPACITY )
				return TRAP_STACK_OVERFLOW;

			bl->stack[bl->sp] = bl->stack[bl->sp - 1 - inst.operand.u64];
			bl->sp += 1;
			bl->ip += 1;
			break;

		case INST_ADD:
			OPERATION(bl, u64, u64, +);
			break;

		case INST_SUB:
			OPERATION(bl, u64, u64, -);
			break;

		case INST_MUL:
			OPERATION(bl, u64, u64, *);
			break;

		case INST_DIV:
			if( bl->stack[bl->sp - 1].u64 == 0 )
				return TRAP_DIV_BY_ZERO;

			OPERATION(bl, u64, u64, /);
			break;

		case INST_MOD:
			OPERATION(bl, u64, u64, %);
			break;

		case INST_IMUL:
			OPERATION(bl, i64, i64, *);
			break;

		case INST_IDIV:
			if( bl->stack[bl->sp - 1].i64 == 0 )
				return TRAP_DIV_BY_ZERO;

			OPERATION(bl, i64, i64, /);
			break;

		case INST_IMOD:
			OPERATION(bl, i64, i64, %);
			break;

		case INST_ADDF:
			OPERATION(bl, f64, f64, +);
			break;

		case INST_SUBF:
			OPERATION(bl, f64, f64, -);
			break;

		case INST_MULF:
			OPERATION(bl, f64, f64, *);
			break;

		case INST_DIVF:
			if( bl->stack[bl->sp - 1].f64 == 0.0 )
				return TRAP_DIV_BY_ZERO;

			OPERATION(bl, f64, f64, /);
			break;

		case INST_JMP:
			bl->ip = inst.operand.u64;
			break;

		case INST_JIF:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			if( bl->stack[bl->sp - 1].u64 ) {
				bl->ip = inst.operand.u64;
			} else {
				bl->ip += 1;
			}

			bl->sp -= 1;

			break;

		case INST_CALL:
			if( bl->sp >= BLISP_STACK_CAPACITY )
				return TRAP_STACK_OVERFLOW;

			bl->stack[bl->sp++].u64 = bl->ip + 1;
			bl->ip = inst.operand.u64;

			break;

		case INST_RET:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->ip = bl->stack[bl->sp - 1].u64;
			bl->sp -= 1;

			break;

		case INST_NATIVE:
			if( inst.operand.u64 > bl->ns )
				return TRAP_ILLEGAL_OPERAND;

			Trap native_err = bl->natives[inst.operand.u64](bl);
			if( native_err != TRAP_OK) {
				return native_err;
			}
			bl->ip += 1;

			break;

		case INST_IEQ:
			OPERATION(bl, i64, u64, ==);
			break;

		case INST_IGT:
			OPERATION(bl, i64, u64, >);
			break;

		case INST_IGE:
			OPERATION(bl, i64, u64, >=);
			break;

		case INST_ILT:
			OPERATION(bl, i64, u64, <);
			break;

		case INST_ILE:
			OPERATION(bl, i64, u64, <=);
			break;

		case INST_INE:
			OPERATION(bl, i64, u64, !=);
			break;

		case INST_EQ:
			OPERATION(bl, u64, u64, ==);
			break;

		case INST_GT:
			OPERATION(bl, u64, u64, >);
			break;

		case INST_GE:
			OPERATION(bl, u64, u64, >=);
			break;

		case INST_LT:
			OPERATION(bl, u64, u64, <);
			break;

		case INST_LE:
			OPERATION(bl, u64, u64, <=);
			break;

		case INST_NE:
			OPERATION(bl, u64, u64, !=);
			break;

		case INST_GTF:
			OPERATION(bl, f64, u64, >);
			break;

		case INST_GEF:
			OPERATION(bl, f64, u64, >=);
			break;

		case INST_LTF:
			OPERATION(bl, f64, u64, <);
			break;

		case INST_LEF:
			OPERATION(bl, f64, u64, <=);
			break;

		case INST_NOT:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 1].u64 = ! bl->stack[bl->sp - 1].u64;
			bl->ip += 1;
			break;

		case INST_AND:
			OPERATION(bl, u64, u64, &);
			break;

		case INST_OR:
			OPERATION(bl, u64, u64, |);
			break;

		case INST_XOR:
			OPERATION(bl, u64, u64, ^);
			break;

		case INST_SHR:
			OPERATION(bl, u64, u64, >>);
			break;

		case INST_SHL:
			OPERATION(bl, u64, u64, <<);
			break;

		case INST_BNOT:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 1].u64 = ~bl->stack[bl->sp - 1].u64;

			bl->ip += 1;

			break;

		case INST_READ8: {
				if( bl->sp < 1 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 1].u64;

				if( addr >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				bl->stack[bl->sp - 1].u64 = bl->memory[addr];

				bl->ip += 1;
			}

			break;

		case INST_READ16: {
				if( bl->sp < 1 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 1].u64;

				if( addr + 1 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				bl->stack[bl->sp - 1].u64 = *(uint16_t*)(&bl->memory[addr]);

				bl->ip += 1;
			}

			break;

		case INST_READ32: {
				if( bl->sp < 1 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 1].u64;

				if( addr + 3 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				bl->stack[bl->sp - 1].u64 = *(uint32_t*)(&bl->memory[addr]);

				bl->ip += 1;
			}

			break;

		case INST_READ64: {
				if( bl->sp < 1 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 1].u64;

				if( addr + 7 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				bl->stack[bl->sp - 1].u64 = *(uint64_t*)(&bl->memory[addr]);

				bl->ip += 1;
			}

			break;

		case INST_WRITE8: {
				if( bl->sp < 2 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 2].u64;

				if( addr >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				uint8_t value = (uint8_t)bl->stack[bl->sp - 1].u64;

				bl->memory[addr] = value;

				bl->sp -= 2;
				bl->ip += 1;
			}

			break;

		case INST_WRITE16: {
				if( bl->sp < 2 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 2].u64;

				if( addr + 1 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				uint16_t value = (uint16_t)bl->stack[bl->sp - 1].u64;

				*(uint16_t*)&bl->memory[addr] = value;

				bl->sp -= 2;
				bl->ip += 1;
			}

			break;

		case INST_WRITE32: {
				if( bl->sp < 2 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 2].u64;

				if( addr + 3 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				uint32_t value = (uint32_t)bl->stack[bl->sp - 1].u64;

				*(uint32_t*)&bl->memory[addr] = value;

				bl->sp -= 2;
				bl->ip += 1;
			}

			break;

		case INST_WRITE64: {
				if( bl->sp < 2 )
					return TRAP_STACK_UNDERFLOW;

				uint64_t addr = bl->stack[bl->sp - 2].u64;

				if( addr + 7 >= bl->memory_capacity )
					return TRAP_ILLEGAL_MEMORY_ACCESS;

				uint64_t value = bl->stack[bl->sp - 1].u64;

				*(uint64_t*)&bl->memory[addr] = value;

				bl->sp -= 2;
				bl->ip += 1;
			}

			break;

		case INST_I2F:
			CAST(bl, i64, f64, (double));
			break;

		case INST_U2F:
			CAST(bl, u64, f64, (double));
			break;

		case INST_F2I:
			CAST(bl, f64, i64, (int64_t));
			break;

		case INST_F2U:
			CAST(bl, f64, u64, (uint64_t)(int64_t));
			break;

		case INST_HALT:
			bl->halt = true;
			bl->ip += 1;
			break;

		case INST_PRINT_DEBUG:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			printf("%ld %ld %f %p\n", bl->stack[bl->sp - 1].u64, bl->stack[bl->sp - 1].i64, bl->stack[bl->sp - 1].f64, bl->stack[bl->sp - 1].ptr);
			bl->ip += 1;
			break;

		case NUMBER_OF_INSTS:
		default:
			return TRAP_ILLEGAL_INST;
	}

	return TRAP_OK;
}

Trap blvm_execute_program(Blvm *bl, int limit) {
	while( limit != 0 && ! bl->halt ) {
		Trap err = blvm_execute_inst(bl);
		if( err != TRAP_OK )
			return err;

		if( limit > 0 )
			limit--;
	}

	return TRAP_OK;
}
