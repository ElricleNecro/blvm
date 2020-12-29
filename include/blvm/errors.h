#ifndef ERRORS_H_PJILJTIW
#define ERRORS_H_PJILJTIW

#include <assert.h>

typedef enum trap_e {
	TRAP_OK = 0,

	TRAP_STACK_OVERFLOW,
	TRAP_STACK_UNDERFLOW,

	TRAP_ILLEGAL_INST,
	TRAP_ILLEGAL_INST_ACCESS,
	TRAP_ILLEGAL_OPERAND,

	TRAP_DIV_BY_ZERO,
} Trap;

const char* trap_as_cstr(Trap trap);

#endif /* end of include guard: ERRORS_H_PJILJTIW */
