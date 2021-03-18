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
