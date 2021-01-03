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
	uint64_t sp;

	Inst *program;
	uint64_t program_size;
	uint64_t ip;

	bool halt;
} Blvm;

void blvm_clean(Blvm *bl);

void blvm_load_program_from_memory(Blvm *bl, const Inst *program, size_t size);
void blvm_load_program_from_file(Blvm *bl, const char *fpath);
void blvm_save_program_to_file(Blvm bl, const char *fpath);

void blvm_dump_stack(const Blvm *bl, FILE *stream);
void blvm_show_state(const Blvm *bl, FILE *stream);

Trap blvm_execute_inst(Blvm *bl);
Trap blvm_execute_program(Blvm *bl, int limit);

#endif /* end of include guard: BLISP_H_LK4X8KOQ */
