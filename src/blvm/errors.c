#include "blvm/errors.h"

const char* trap_as_cstr(Trap trap) {
	switch(trap) {
		case TRAP_OK:
			return "OK";
		case TRAP_STACK_OVERFLOW:
			return "STACK_OVERFLOW";
		case TRAP_STACK_UNDERFLOW:
			return "STACK_UNDERFLOW";
		case TRAP_NATIVES_OVERFLOW:
			return "NATIVES_OVERFLOW";
		case TRAP_ILLEGAL_INST:
			return "ILLEGAL_INST";
		case TRAP_ILLEGAL_OPERAND:
			return "ILLEGAL_OPERAND";
		case TRAP_DIV_BY_ZERO:
			return "DIV_BY_ZERO";
		case TRAP_ILLEGAL_INST_ACCESS:
			return "ILLEGAL_INST_ACCESS";
		default:
			assert(0 && "trap_as_cstr: Unreachable");
	}
}
