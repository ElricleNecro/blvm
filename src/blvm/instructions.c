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
	[INST_MOD]         = "mod",
	[INST_IMUL]        = "imul",
	[INST_IDIV]        = "idiv",
	[INST_IMOD]         = "mod",

	[INST_ADDF]        = "addf",
	[INST_SUBF]        = "subf",
	[INST_MULF]        = "mulf",
	[INST_DIVF]        = "divf",

	[INST_JMP]         = "jmp",
	[INST_JIF]         = "jif",

	[INST_CALL]        = "call",
	[INST_RET]         = "ret",
	[INST_NATIVE]      = "native",

	[INST_AND]         = "and",
	[INST_OR]          = "or",
	[INST_XOR]         = "xor",
	[INST_SHR]         = "shr",
	[INST_SHL]         = "shl",
	[INST_BNOT]        = "bot",

	[INST_IEQ]         = "ieq",
	[INST_IGT]         = "igt",
	[INST_IGE]         = "ige",
	[INST_ILT]         = "ilt",
	[INST_ILE]         = "ile",
	[INST_INE]         = "ine",

	[INST_EQ]          = "eq",
	[INST_GT]          = "gt",
	[INST_GE]          = "ge",
	[INST_LT]          = "lt",
	[INST_LE]          = "le",
	[INST_NE]          = "ne",

	[INST_GTF]         = "gtf",
	[INST_GEF]         = "gef",
	[INST_LTF]         = "ltf",
	[INST_LEF]         = "lef",
	[INST_NOT]         = "not",

	[INST_READ8]       = "read8",
	[INST_READ16]      = "read16",
	[INST_READ32]      = "read32",
	[INST_READ64]      = "read64",

	[INST_WRITE8]      = "write8",
	[INST_WRITE16]     = "write16",
	[INST_WRITE32]     = "write32",
	[INST_WRITE64]     = "write64",

	[INST_I2F]         = "i2f",
	[INST_U2F]         = "u2f",
	[INST_F2I]         = "f2i",
	[INST_F2U]         = "f2u",

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
	[INST_MOD]         = false,
	[INST_IMUL]        = false,
	[INST_IDIV]        = false,
	[INST_IMOD]        = false,

	[INST_ADDF]        = false,
	[INST_SUBF]        = false,
	[INST_MULF]        = false,
	[INST_DIVF]        = false,

	[INST_JMP]         = true,
	[INST_JIF]         = true,

	[INST_CALL]        = true,
	[INST_RET]         = false,
	[INST_NATIVE]      = true,

	[INST_IEQ]         = false,
	[INST_IGT]         = false,
	[INST_IGE]         = false,
	[INST_ILT]         = false,
	[INST_ILE]         = false,
	[INST_INE]         = false,

	[INST_EQ]          = false,
	[INST_GT]          = false,
	[INST_GE]          = false,
	[INST_LT]          = false,
	[INST_LE]          = false,
	[INST_NE]          = false,

	[INST_GTF]         = false,
	[INST_GEF]         = false,
	[INST_LTF]         = false,
	[INST_LEF]         = false,
	[INST_NOT]         = false,

	[INST_AND]         = false,
	[INST_OR]          = false,
	[INST_XOR]         = false,
	[INST_SHR]         = false,
	[INST_SHL]         = false,
	[INST_BNOT]        = false,

	[INST_READ8]       = false,
	[INST_READ16]      = false,
	[INST_READ32]      = false,
	[INST_READ64]      = false,

	[INST_WRITE8]      = false,
	[INST_WRITE16]     = false,
	[INST_WRITE32]     = false,
	[INST_WRITE64]     = false,

	[INST_I2F]         = false,
	[INST_U2F]         = false,
	[INST_F2I]         = false,
	[INST_F2U]         = false,

	[INST_HALT]        = false,

	[INST_PRINT_DEBUG] = false,
};
