#ifndef BLISP_H_LK4X8KOQ
#define BLISP_H_LK4X8KOQ

#include <assert.h>

#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "blvm/define.h"
#include "blvm/errors.h"
#include "blvm/instructions.h"

typedef struct blvm_t {
	Word stack[BLISP_STACK_CAPACITY];
	Word sp;

	Inst *program;
	Word program_size;
	Word ip;

	bool halt;
} Blisp;

void blvm_clean(Blisp *bl);

void blvm_push_inst(Blisp *bl, Inst inst);

void blvm_load_program_from_memory(Blisp *bl, const Inst *program, Word size);
void blvm_load_program_from_file(Blisp *bl, const char *fpath);
void blvm_save_program_to_file(Blisp bl, const char *fpath);

void blvm_dump_stack(const Blisp *bl, FILE *stream);
void blvm_show_state(const Blisp *bl, FILE *stream);

Trap blvm_execute_inst(Blisp *bl);

#endif /* end of include guard: BLISP_H_LK4X8KOQ */
