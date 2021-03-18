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

	INST_ADDF,
	INST_SUBF,
	INST_MULF,
	INST_DIVF,

	INST_JMP,
	INST_JIF,

	INST_CALL,
	INST_RET,
	INST_NATIVE,

	INST_EQ,
	INST_GT,
	INST_GEF,
	INST_NOT,

	INST_AND,
	INST_OR,
	INST_XOR,
	INST_SHR,
	INST_SHL,
	INST_BNOT,

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
