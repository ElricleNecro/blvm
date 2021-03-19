#include "blvm/blvm.h"

void blvm_clean(Blvm *bl) {
	free(bl->program), bl->program_size = 0;
}

void blvm_load_program_from_memory(Blvm *bl, const Inst *program, size_t size) {
	bl->program = malloc(size * sizeof(struct inst_t));
	bl->program_size = size;

	memcpy(bl->program, program, size * sizeof(struct inst_t));
}

void blvm_load_program_from_file(Blvm *bl, const char *fpath) {
	FILE *file = NULL;

	if( (file = fopen(fpath, "rb")) == NULL ) {
		fprintf(stderr, "ERROR: Could not open file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if( fseek(file, 0, SEEK_END) < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	long fsize = ftell(file);
	if( fsize < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	assert((size_t)fsize % sizeof(struct inst_t) == 0);

	if( fseek(file, 0, SEEK_SET) < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if( (bl->program = malloc((size_t)fsize)) == NULL ) {
		fprintf(stderr, "ERROR: Could not allocate memory for file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	bl->program_size = fread(bl->program, sizeof(struct inst_t), (size_t)fsize / sizeof(struct inst_t), file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not read from file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fclose(file);
}

void blvm_save_program_to_file(Blvm bl, const char *fpath) {
	FILE *file = NULL;

	if( (file = fopen(fpath, "wb")) == NULL ) {
		fprintf(stderr, "ERROR: Could not open file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fwrite(bl.program, sizeof(struct inst_t), bl.program_size, file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fclose(file);
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
			fprintf(stream, "\t%lu %ld %lf %p\n", bl->stack[i].u64, bl->stack[i].i64, bl->stack[i].f64, bl->stack[i].ptr);
	} else {
		fprintf(stream, "\t[empty]\n");
	}
}

void blvm_show_state(const Blvm *bl, FILE *stream) {
		fprintf(stream, "0x%03lX: %s", bl->ip, inst_names[bl->program[bl->ip].type]);

		if( bl->program[bl->ip].type == INST_PUSH || bl->program[bl->ip].type == INST_DUP || bl->program[bl->ip].type == INST_JMP )
			fprintf(stream, " %lu", bl->program[bl->ip].operand.u64);

		fprintf(stream, " (sp: 0x%03lX)\n", bl->sp);
}

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
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 += bl->stack[bl->sp - 1].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_SUB:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 -= bl->stack[bl->sp - 1].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_MUL:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 *= bl->stack[bl->sp - 1].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_DIV:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			if( bl->stack[bl->sp - 1].u64 == 0 )
				return TRAP_DIV_BY_ZERO;

			bl->stack[bl->sp - 2].u64 /= bl->stack[bl->sp - 1].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_ADDF:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].f64 += bl->stack[bl->sp - 1].f64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_SUBF:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].f64 -= bl->stack[bl->sp - 1].f64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_MULF:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].f64 *= bl->stack[bl->sp - 1].f64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_DIVF:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			if( bl->stack[bl->sp - 1].f64 == 0.0 )
				return TRAP_DIV_BY_ZERO;

			bl->stack[bl->sp - 2].f64 /= bl->stack[bl->sp - 1].f64;
			bl->sp -= 1;
			bl->ip += 1;
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

			bl->natives[inst.operand.u64](bl);
			bl->ip += 1;

			break;

		case INST_EQ:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 = bl->stack[bl->sp - 1].u64 == bl->stack[bl->sp - 2].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_GT:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 = bl->stack[bl->sp - 1].u64 > bl->stack[bl->sp - 2].u64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_GEF:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 = bl->stack[bl->sp - 1].f64 >= bl->stack[bl->sp - 2].f64;
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_NOT:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 1].u64 = ! bl->stack[bl->sp - 1].u64;
			bl->ip += 1;
			break;

		case INST_AND:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 &= bl->stack[bl->sp - 1].u64;

			bl->sp -= 1;
			bl->ip += 1;

			break;

		case INST_OR:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 |= bl->stack[bl->sp - 1].u64;

			bl->sp -= 1;
			bl->ip += 1;

			break;

		case INST_XOR:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 ^= bl->stack[bl->sp - 1].u64;

			bl->sp -= 1;
			bl->ip += 1;

			break;

		case INST_SHR:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 >>= bl->stack[bl->sp - 1].u64;

			bl->sp -= 1;
			bl->ip += 1;

			break;

		case INST_SHL:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2].u64 <<= bl->stack[bl->sp - 1].u64;

			bl->sp -= 1;
			bl->ip += 1;

			break;

		case INST_BNOT:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 1].u64 = ~bl->stack[bl->sp - 1].u64;

			bl->ip += 1;

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
