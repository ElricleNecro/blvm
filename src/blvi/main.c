#include <stdio.h>
#include <stdlib.h>

#include "blvm/blvm.h"

int main(int argc, const char *argv[]) {
	if( argc != 2 ) {
		fprintf(stderr, "Usage: %s [program]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *program = argv[1];

	Blisp bl = {0};

	/*blvm_load_program_from_memory(&bl, program, STATIC_ARRAY_SIZE(program));*/
	/*blvm_save_program_to_file(&bl, "fibonacci.bl");*/
	blvm_load_program_from_file(&bl, program);

	while( ! bl.halt ) {
		blvm_show_state(&bl, stdout);
		Trap exc = blvm_execute_inst(&bl);

		if( exc != TRAP_OK ) {
			fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
			blvm_dump_stack(&bl, stderr);
			return EXIT_FAILURE;
		} else {
			blvm_dump_stack(&bl, stdout);
		}
	}

	blvm_clean(&bl);

	return EXIT_SUCCESS;
}
