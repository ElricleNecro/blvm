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

int main(int argc, const char *argv[]) {
	Args *args = Args_New();
	Args_Error err;

	int limit = -1;
	Args_Add(args, "-l", "--limit", T_INT, &limit, "Maximum number of instruction to run.");

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
	Trap exc = TRAP_OK;

	blvm_load_program_from_file(&bl, program);

	if( (exc = blvm_push_native(&bl, blvm_alloc)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_free)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_f64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_i64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_u64)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_push_native(&bl, blvm_print_ptr)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	if( (exc = blvm_execute_program(&bl, limit)) != TRAP_OK ) {
		fprintf(stderr, "An error occured: %s.\n", trap_as_cstr(exc));
		return EXIT_FAILURE;
	}

	blvm_clean(&bl);

	Args_Free(args);

	return EXIT_SUCCESS;
}
