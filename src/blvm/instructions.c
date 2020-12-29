#include "blvm/instructions.h"

const char* insttype_as_cstr(InstType type) {
	switch(type) {
		case INST_NOP:
			return "NOP";

		case INST_PUSH:
			return "PUSH";
		case INST_POP:
			return "POP";
		case INST_DUP:
			return "DUP";

		case INST_ADD:
			return "ADD";
		case INST_SUB:
			return "SUB";
		case INST_MUL:
			return "MUL";
		case INST_DIV:
			return "DIV";

		case INST_JMP:
			return "JMP";
		case INST_JIF:
			return "JIF";

		case INST_EQ:
			return "EQ";
		case INST_GT:
			return "GT";

		case INST_HALT:
			return "HALT";

		case INST_PRINT_DEBUG:
			return "PRINT_DEBUG";
		default:
			assert(0 && "insttype_as_cstr: Unreachable");
	}
}
