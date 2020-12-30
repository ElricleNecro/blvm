#include "blvm/blvm.h"

void blvm_clean(Blvm *bl) {
	free(bl->program), bl->program_size = 0;
}

void blvm_push_inst(Blvm *bl, Inst inst) {
	assert(bl->program_size < BLISP_PROGRAM_CAPACITY);
	bl->program[bl->program_size++] = inst;
}

void blvm_load_program_from_memory(Blvm *bl, const Inst *program, Word size) {
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

	assert(fsize % sizeof(struct inst_t) == 0);

	if( fseek(file, 0, SEEK_SET) < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if( (bl->program = malloc(fsize)) == NULL ) {
		fprintf(stderr, "ERROR: Could not allocate memory for file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	bl->program_size = fread(bl->program, sizeof(struct inst_t), fsize / sizeof(struct inst_t), file);
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

void blvm_dump_stack(const Blvm *bl, FILE *stream) {
	fprintf(stream, "Stack:\n");
	if( bl->sp > 0) {
		for(Word i = 0; i < bl->sp; ++i)
			fprintf(stream, "\t%lu\n", bl->stack[i]);
	} else {
		fprintf(stream, "\t[empty]\n");
	}
}

void blvm_show_state(const Blvm *bl, FILE *stream) {
		fprintf(stream, "0x%03lX: %s", bl->ip, insttype_as_cstr(bl->program[bl->ip].type));

		if( bl->program[bl->ip].type == INST_PUSH || bl->program[bl->ip].type == INST_DUP || bl->program[bl->ip].type == INST_JMP )
			fprintf(stream, " %lu", bl->program[bl->ip].operand);

		fprintf(stream, " (sp: 0x%03lX)\n", bl->sp);
}

Trap blvm_execute_inst(Blvm *bl) {
	if( bl->ip < 0 || bl->ip >= bl->program_size )
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

		case INST_DUP:
			if( bl->sp - inst.operand <= 0 )
				return TRAP_STACK_UNDERFLOW;
			if( inst.operand < 0 )
				return TRAP_ILLEGAL_OPERAND;
			if( bl->sp >= BLISP_STACK_CAPACITY )
				return TRAP_STACK_OVERFLOW;

			bl->stack[bl->sp] = bl->stack[bl->sp - 1 - inst.operand];
			bl->sp += 1;
			bl->ip += 1;
			break;

		case INST_ADD:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2] += bl->stack[bl->sp - 1];
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_SUB:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2] -= bl->stack[bl->sp - 1];
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_MUL:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2] *= bl->stack[bl->sp - 1];
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_DIV:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			if( bl->stack[bl->sp - 1] == 0 )
				return TRAP_DIV_BY_ZERO;

			bl->stack[bl->sp - 2] /= bl->stack[bl->sp - 1];
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_JMP:
			bl->ip = inst.operand;
			break;

		case INST_JIF:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			if( bl->stack[bl->sp - 1] ) {
				bl->ip = inst.operand;
				bl->sp -= 1;
			} else {
				bl->ip += 1;
			}

			break;

		case INST_EQ:
			if( bl->sp < 2 )
				return TRAP_STACK_UNDERFLOW;

			bl->stack[bl->sp - 2] = bl->stack[bl->sp - 1] == bl->stack[bl->sp - 2];
			bl->sp -= 1;
			bl->ip += 1;
			break;

		case INST_HALT:
			bl->halt = true;
			bl->ip += 1;
			break;

		case INST_PRINT_DEBUG:
			if( bl->sp < 1 )
				return TRAP_STACK_UNDERFLOW;

			printf("%ld", bl->stack[bl->sp - 1]);
			bl->ip += 1;
			break;

		default:
			return TRAP_ILLEGAL_INST;
	}

	return TRAP_OK;
}
