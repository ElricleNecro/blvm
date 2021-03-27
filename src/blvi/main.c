#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"

Trap blvm_alloc(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	bl->stack[bl->sp - 1].ptr = malloc(bl->stack[bl->sp - 1].u64);

	return TRAP_OK;
}

Trap blvm_free(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	free(bl->stack[bl->sp - 1].ptr);
	bl->sp -= 1;

	return TRAP_OK;
}

Trap blvm_print_f64(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	printf("%lf\n", bl->stack[bl->sp - 1].f64);
	bl->sp -= 1;

	return TRAP_OK;
}

Trap blvm_print_i64(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	printf("%ld\n", bl->stack[bl->sp - 1].i64);
	bl->sp -= 1;

	return TRAP_OK;
}

Trap blvm_print_u64(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	printf("%lu\n", bl->stack[bl->sp - 1].u64);
	bl->sp -= 1;

	return TRAP_OK;
}

Trap blvm_print_ptr(Blvm *bl) {
	if( bl->sp < 1 )
		return TRAP_STACK_UNDERFLOW;

	printf("%p\n", bl->stack[bl->sp - 1].ptr);
	bl->sp -= 1;

	return TRAP_OK;
}

Trap blvm_print_mem(Blvm *bl) {
	if( bl->sp < 2 )
		return TRAP_STACK_UNDERFLOW;

	uint64_t count = bl->stack[bl->sp - 1].u64;
	uint64_t from  = bl->stack[bl->sp - 2].u64;

	if( from >= bl->memory_capacity || count > bl->memory_capacity )
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	if( (from + count - 1) >= bl->memory_capacity || (from + count) < from)
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	for(uint64_t addr = from; addr < (from + count); addr++) {
		printf("0x%x", bl->memory[addr]);

		if( addr < (from + count - 1) )
			printf(" ");
	}
	printf("\n");

	bl->sp -= 2;

	return TRAP_OK;
}

Trap blvm_write(Blvm *bl) {
	if( bl->sp < 2 )
		return TRAP_STACK_UNDERFLOW;

	uint64_t addr = bl->stack[bl->sp - 2].u64;
	uint64_t size = bl->stack[bl->sp - 1].u64;

	if( addr >= bl->memory_capacity || size > bl->memory_capacity )
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	if( (addr + size - 1) >= bl->memory_capacity || (addr + size) < addr)
		return TRAP_ILLEGAL_MEMORY_ACCESS;

	fwrite(&bl->memory[addr], sizeof(uint8_t), size, stdout);
	bl->sp -= 2;

	return TRAP_OK;
}

int main(int argc, const char *argv[]) {
	Args *args = Args_New();
	Args_Error err;

	int limit = -1;
	bool debug = false;
	Args_Add(args, "-l", "--limit", T_INT, &limit, "Maximum number of instruction to run.");
	Args_Add(args, "-d", "--debug-mode", T_BOOL, &debug, "Print the VM state at each step.");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR ) {
		Args_Free(args);
		return EXIT_FAILURE;
	} else if( err == HELP ) {
		Args_Free(args);
		return EXIT_SUCCESS;
	} else if( args->rest == NULL ) {
		fprintf(stderr, "Missing arguments.");
		Args_Free(args);
		return EXIT_FAILURE;
	}

	const char *program = args->rest->opt;

	Blvm bl = {0};
	Trap exc = TRAP_OK;

	if( ! blvm_load_program_from_file(&bl, program) ) {
		fprintf(stderr, "An error occured while trying to read '%s'. Exiting.", program);

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_alloc)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_free)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_f64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_i64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_u64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_ptr)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_mem)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_write)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

		blvm_clean(&bl);
		Args_Free(args);

		return EXIT_FAILURE;
	}

	if( ! debug ) {
		if( (exc = blvm_execute_program(&bl, limit)) != TRAP_OK ) {
			fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

			blvm_clean(&bl);
			Args_Free(args);

			return EXIT_FAILURE;
		}
	} else {
		while( limit != 0 && ! bl.halt ) {
			blvm_dump_stack(&bl, stderr);
			blvm_show_state(&bl, stderr);

			fputs("\n", stderr);

			if(  (exc = blvm_execute_inst(&bl)) != TRAP_OK ) {
				fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));

				blvm_clean(&bl);
				Args_Free(args);

				return EXIT_FAILURE;
			}
			if( limit > 0 )
				limit--;
		}
	}

	blvm_clean(&bl);

	Args_Free(args);

	return EXIT_SUCCESS;
}
