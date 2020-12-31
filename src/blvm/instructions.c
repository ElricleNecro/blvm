#include "blvm/instructions.h"

const char* insttype_as_cstr(InstType type) {
	switch(type) {
		case INST_NOP:
			return "nop";

		case INST_PUSH:
			return "push";
		case INST_POP:
			return "pop";
		case INST_DUP:
			return "dup";

		case INST_ADD:
			return "add";
		case INST_SUB:
			return "sub";
		case INST_MUL:
			return "mul";
		case INST_DIV:
			return "div";

		case INST_JMP:
			return "jmp";
		case INST_JIF:
			return "jif";

		case INST_EQ:
			return "eq";
		case INST_GT:
			return "gt";

		case INST_HALT:
			return "halt";

		case INST_PRINT_DEBUG:
			return "print_debug";
		default:
			assert(0 && "insttype_as_cstr: Unreachable");
	}
}
