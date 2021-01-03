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

	INST_EQ,
	INST_GT,

	INST_HALT,

	INST_PRINT_DEBUG,
} InstType;

typedef struct inst_t {
	InstType type;
	Word operand;
} Inst;

extern const char *inst_names[];
extern const bool inst_required_operand[];

#define MAKE_INST_NOP {.type = INST_NOP}

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = {.u64 = (value)}}
#define MAKE_INST_POP  {.type = INST_POP}
#define MAKE_INST_SWAP {.type = INST_SWAP}
#define MAKE_INST_DUP(addr) {.type = INST_DUP, .operand = {.u64 = (addr)}}

#define MAKE_INST_ADD {.type = INST_ADD}
#define MAKE_INST_SUB {.type = INST_SUB}
#define MAKE_INST_MUL {.type = INST_MUL}
#define MAKE_INST_DIV {.type = INST_DIV}

#define MAKE_INST_ADDF {.type = INST_ADDF}
#define MAKE_INST_SUBF {.type = INST_SUBF}
#define MAKE_INST_MULF {.type = INST_MULF}
#define MAKE_INST_DIVF {.type = INST_DIVF}

#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = {.u64 = (addr)}}
#define MAKE_INST_JIF(addr) {.type = INST_JMP, .operand = {.u64 = (addr)}}

#define MAKE_INST_EQ {.type = INST_EQ}
#define MAKE_INST_GT {.type = INST_GT}

#define MAKE_INST_HALT {.type = INST_HALT}
#define MAKE_INST_PRINT_DEBUG {.type = INST_PRINT_DEBUG}

const char* insttype_as_cstr(InstType type);

#endif /* end of include guard: INSTRUCTIONS_H_L07SEIYC */
