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
		const Inst inst = bl.program[idx];
		printf("%s", inst_names[inst.type]);
		if( inst_required_operand[inst.type] )
			printf(" %ld", inst.operand.u64);
		printf("\n");
	}

	blvm_clean(&bl);

	return EXIT_SUCCESS;
}
