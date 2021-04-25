#ifndef INSTRUCTIONS_H_L07SEIYC
#define INSTRUCTIONS_H_L07SEIYC

#include <assert.h>
#include <stdbool.h>

#include "blvm/define.h"

typedef enum insttype_e {
	INST_NOP = 0,

	INST_PUSH,
	INST_POP,
	INST_SWAP,
	INST_DUP,

	INST_ADD,
	INST_SUB,
	INST_MUL,
	INST_DIV,
	INST_MOD,
	INST_IMUL,
	INST_IDIV,
	INST_IMOD,

	INST_ADDF,
	INST_SUBF,
	INST_MULF,
	INST_DIVF,

	INST_JMP,
	INST_JIF,

	INST_CALL,
	INST_RET,
	INST_NATIVE,

	INST_IEQ,
	INST_IGT,
	INST_IGE,
	INST_ILT,
	INST_ILE,
	INST_INE,

	INST_EQ,
	INST_GT,
	INST_GE,
	INST_LT,
	INST_LE,
	INST_NE,

	INST_GTF,
	INST_GEF,
	INST_LTF,
	INST_LEF,
	INST_NOT,

	INST_AND,
	INST_OR,
	INST_XOR,
	INST_SHR,
	INST_SHL,
	INST_BNOT,

	INST_READ8,
	INST_READ16,
	INST_READ32,
	INST_READ64,

	INST_WRITE8,
	INST_WRITE16,
	INST_WRITE32,
	INST_WRITE64,

	INST_I2F,
	INST_U2F,
	INST_F2I,
	INST_F2U,

	INST_HALT,

	INST_PRINT_DEBUG,

	NUMBER_OF_INSTS
} InstType;

typedef struct inst_t {
	InstType type;
	Word operand;
} Inst;

extern const char *inst_names[];
extern const bool inst_required_operand[];

#endif /* end of include guard: INSTRUCTIONS_H_L07SEIYC */
