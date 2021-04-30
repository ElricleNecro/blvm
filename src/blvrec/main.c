#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "blvm/stringview.h"

static char *recorded = NULL;
static size_t recorded_size = 0;

Trap blvm_write(Blvm *bl) {
	if( bl->sp < 2 )
		return TRAP_STACK_UNDERFLOW;

	uint64_t addr = bl->stack[bl->sp - 2].u64;
	uint64_t size = bl->stack[bl->sp - 1].u64;

	if( addr >= bl->memory_capacity || size > bl->memory_capacity )
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	if( (addr + size - 1) >= bl->memory_capacity || (addr + size) < addr)
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	recorded = realloc(recorded, recorded_size + size);
	memcpy(recorded + recorded_size, &bl->memory[addr], size);
	recorded_size += size;

	bl->sp -= 2;

	return TRAP_OK;
}

void clean_up_bl(Blvm *bl) {
	blvm_clean(bl);
}

void clean_args(Args **args) {
	Args_Free(*args);
}

void close_file(FILE **fich) {
	if( *fich != NULL )
		fclose(*fich);
}

void stringview_free(StringView *data) {
	if( data->count > 0 ) {
		free(data->data), data->count = 0;
	}
}

int main(int argc, const char *argv[]) {
	Args *args __attribute__((__cleanup__(clean_args))) = Args_New();
	Args_Error err;

	char *output_path = NULL, *expected_path = NULL;
	Args_Add(args, "-o", "--output", T_CHAR, &output_path, "Recorded output of WRITE");
	Args_Add(args, "-e", "--expected", T_CHAR, &expected_path, "Expected recorded output of WRITE");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR ) {
		return EXIT_FAILURE;
	} else if( err == HELP ) {
		return EXIT_SUCCESS;
	} else if( args->rest == NULL ) {
		fprintf(stderr, "Missing arguments.");
		return EXIT_FAILURE;
	}

	const char *program = args->rest->opt;

	Blvm bl __attribute__((__cleanup__(clean_up_bl))) = {0};
	Trap exc = TRAP_OK;

	if( ! blvm_load_program_from_file(&bl, program) ) {
		fprintf(stderr, "An error occured while trying to read '%s'. Exiting.", program);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_write)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		return EXIT_FAILURE;
	}

	if( (exc = blvm_execute_program(&bl, -1)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		if( recorded != NULL )
			free(recorded);

		return EXIT_FAILURE;
	}

	if( output_path != NULL ) {
		FILE *output __attribute__((__cleanup__(close_file))) = NULL;
		if( (output = fopen(output_path, "wb")) == NULL ) {
			fprintf(stderr, "ERROR: could not create file '%s': %s.\n", output_path, strerror(errno));

			if( recorded != NULL )
				free(recorded);

			return EXIT_FAILURE;
		}

		fwrite(recorded, sizeof(char), recorded_size, output);
		if( ferror(output) ) {
			fprintf(stderr, "ERROR: Could not write to file '%s': %s.\n", output_path, strerror(errno));

			if( recorded != NULL )
			free(recorded);

			return EXIT_FAILURE;
		}

		fclose(output), output = NULL;
	}

	if( expected_path != NULL ) {
		StringView expected __attribute__((__cleanup__(stringview_free))) = load_file(expected_path);
		if( ! stringview_eq(expected, (StringView){ .data=recorded, .count=recorded_size }) ) {
			printf("Output does not match the expected output.\n");
			if( recorded != NULL )
				free(recorded);
			return EXIT_FAILURE;
		} else {
			printf("Output match the expected output.\n");
		}
	}

	if( recorded != NULL )
		free(recorded);

	return EXIT_SUCCESS;
}
