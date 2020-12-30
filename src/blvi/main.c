#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"

int main(int argc, const char *argv[]) {
	Args *args = Args_New();
	Args_Error err;

	int limit = -1;
	Args_Add(args, "-m", "--max", T_INT, &limit, "Maximum number of instruction to run.");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR ) {
		Args_Free(args);
		return EXIT_FAILURE;
	} else if( err == HELP ) {
		Args_Free(args);
		return EXIT_SUCCESS;
	} else if( args->rest == NULL ) {
		fprintf(stderr, "Missing arguments.");
		return EXIT_FAILURE;
	}

	const char *program = args->rest->opt;

	Blvm bl = {0};

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
