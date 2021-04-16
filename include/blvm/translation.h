#ifndef TRANSLATION_UNIT_BLASM_H
#define TRANSLATION_UNIT_BLASM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blvm/instructions.h"
#include "blvm/records.h"
#include "blvm/stringview.h"

#ifndef BLASM_COMMENT_SYMBOL
#define BLASM_COMMENT_SYMBOL ';'
#endif

#ifndef BLASM_PREPRO_SYMBOL
#define BLASM_PREPRO_SYMBOL '%'
#endif

#ifndef BLASM_INCLUDE_LEVEL_MAX
#define BLASM_INCLUDE_LEVEL_MAX 64
#endif

typedef struct _STRUCT_OPTIONS bl_file_program {
	Inst *program;
	uint64_t program_size;
} BlProgram;

typedef struct _STRUCT_OPTIONS bl_file_memory {
	size_t memory_size;	/** Actual size of the memory */
	size_t memory_capacity;	/** Actual size of the memory */
	uint8_t *memory;	/** VM memory */
} BlMemory;

typedef struct blprog_t {
	BlProgram prog;
	BlMemory mem;

	size_t include_level; 		// To avoid include loop
	size_t entry_point;
} BlProg;

typedef struct _include_list_t {
	const char *fname;
	struct _include_list_t *next, *end;
} *IncludeList;

void blprog_save_program_to_file(BlProg bl, const char *fpath);
void blprog_clean(BlProg *bl);
char* search_file(const IncludeList paths, StringView include);
bool translate_source(BlProg *bl, const IncludeList include_paths, const char *fname, StringView src, Records *records);
bool stringview_number_litteral(StringView sv, Word *word);
bool stringview_as_insttype(StringView name, InstType *output);

#endif
