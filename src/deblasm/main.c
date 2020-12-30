#include <stdio.h>
#include <stdlib.h>

#include "blvm/blvm.h"

int main(int argc, char *argv[]) {
	if( argc < 2 ) {
		fprintf(stderr, "Usage: %s [input]\n", argv[0]);
		return EXIT_FAILURE;
	}

	Blvm bl = {0};
	blvm_load_program_from_file(&bl, argv[1]);

	for(size_t idx = 0; idx < bl.program_size; idx++) {
		Inst inst = bl.program[idx];
		printf("%s", insttype_as_cstr(inst.type));
		switch( inst.type ) {
			case INST_PUSH:
			case INST_DUP:
			case INST_JMP:
			case INST_JIF:
				printf(" %ld\n", inst.operand);
				break;
			case INST_NOP:
			case INST_POP:
			case INST_ADD:
			case INST_SUB:
			case INST_MUL:
			case INST_DIV:
			case INST_EQ:
			case INST_GT:
			case INST_HALT:
			case INST_PRINT_DEBUG:
			default:
				printf("\n");
				break;
		}
	}

	blvm_clean(&bl);

	return EXIT_SUCCESS;
}
