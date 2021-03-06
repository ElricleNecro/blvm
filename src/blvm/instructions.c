#include "blvm/instructions.h"

const char *inst_names[] = {
	[INST_NOP]         = "nop",

	[INST_PUSH]        = "push",
	[INST_POP]         = "pop",
	[INST_SWAP]        = "swap",
	[INST_DUP]         = "dup",

	[INST_ADD]         = "add",
	[INST_SUB]         = "sub",
	[INST_MUL]         = "mul",
	[INST_DIV]         = "div",

	[INST_ADDF]        = "addf",
	[INST_SUBF]        = "subf",
	[INST_MULF]        = "mulf",
	[INST_DIVF]        = "divf",

	[INST_JMP]         = "jmp",
	[INST_JIF]         = "jif",

	[INST_CALL]        = "call",
	[INST_RET]         = "ret",
	[INST_NATIVE]      = "native",

	[INST_EQ]          = "eq",
	[INST_GT]          = "gt",
	[INST_GEF]         = "gef",
	[INST_NOT]         = "not",

	[INST_HALT]        = "halt",

	[INST_PRINT_DEBUG] = "print_debug",
};

const bool inst_required_operand[] = {
	[INST_NOP]         = false,

	[INST_PUSH]        = true,
	[INST_POP]         = false,
	[INST_SWAP]        = true,
	[INST_DUP]         = true,

	[INST_ADD]         = false,
	[INST_SUB]         = false,
	[INST_MUL]         = false,
	[INST_DIV]         = false,

	[INST_ADDF]        = false,
	[INST_SUBF]        = false,
	[INST_MULF]        = false,
	[INST_DIVF]        = false,

	[INST_JMP]         = true,
	[INST_JIF]         = true,

	[INST_CALL]        = true,
	[INST_RET]         = false,
	[INST_NATIVE]      = true,

	[INST_EQ]          = false,
	[INST_GT]          = false,
	[INST_GEF]         = false,
	[INST_NOT]         = false,

	[INST_HALT]        = false,

	[INST_PRINT_DEBUG] = false,
};

const char* insttype_as_cstr(InstType type) {
	switch(type) {
		case INST_NOP:
			return "nop";

		case INST_PUSH:
			return "push";
		case INST_POP:
			return "pop";
		case INST_SWAP:
			return "swap";
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

		case INST_ADDF:
			return "addf";
		case INST_SUBF:
			return "subf";
		case INST_MULF:
			return "mulf";
		case INST_DIVF:
			return "divf";

		case INST_JMP:
			return "jmp";
		case INST_JIF:
			return "jif";

		case INST_CALL:
			return "call";
		case INST_RET:
			return "ret";
		case INST_NATIVE:
			return "native";

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
