#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"

#include "records.h"
#include "stringview.h"
#include "utils.h"

#ifndef BLASM_COMMENT_SYMBOL
#define BLASM_COMMENT_SYMBOL ';'
#endif

#ifndef BLASM_PREPRO_SYMBOL
#define BLASM_PREPRO_SYMBOL '%'
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
} BlProg;

void blprog_clean(BlProg *bl) {
	if( bl->prog.program != NULL ) {
		free(bl->prog.program);
		bl->prog.program_size = 0;
	}

	if( bl->mem.memory != NULL ) {
		free(bl->mem.memory);
		bl->mem.memory_size = 0;
	}
}

void blprog_save_program_to_file(BlProg bl, const char *fpath) {
	FILE *file = NULL;

	if( (file = fopen(fpath, "wb")) == NULL ) {
		fprintf(stderr, "ERROR: Could not open file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	BlMeta meta = {
		.magic = BL_MAGIC,
		.version = BL_VERSION,
		.memory_size = bl.mem.memory_size,
		.memory_capacity = bl.mem.memory_capacity,
		.program_size = bl.prog.program_size,
	};

	fwrite(&meta, sizeof(BlMeta), 1, file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fwrite(bl.prog.program, sizeof(struct inst_t), bl.prog.program_size, file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fwrite(bl.mem.memory, sizeof(uint8_t), bl.mem.memory_size, file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fclose(file);
}

char* search_file(const CList paths, StringView include) {
	char *results = NULL;

	for(CList tmp = paths; tmp != NULL; tmp = tmp->next) {
		results = realloc(results, (strlen(tmp->opt) + include.count + 2) * sizeof(char));
		snprintf(results, strlen(tmp->opt) + include.count + 2, "%s/%.*s", tmp->opt, (int)include.count, include.data);

		if( file_exist(results) )
			return results;
	}

	free(results), results = NULL;
	return NULL;
}

bool translate_source(BlProg *bl, const CList include_paths, const char *fname, StringView src, Records *records) {
	for(size_t line_nb = 1; src.count > 0; line_nb++) {
		StringView line = stringview_split(&src, '\n');
		line = stringview_trim(line);

		if( line.count <= 0 || *line.data == BLASM_COMMENT_SYMBOL )
			continue;

		Inst inst = {0};
		StringView name = stringview_split_on_spaces(&line);
		line = stringview_trim(line);

		// Are we on a preprocessor directive?
		if( stringview_startwith(name, BLASM_PREPRO_SYMBOL) ) {
			// If yes, ignoring the '#':
			name.count -= 1;
			name.data += 1;

			StringView directive = stringview_trim(name);
			// On what directive are we?
			if( stringview_eq_cstr(directive, "define") ) {
				// Extracting the label name:
				StringView label = stringview_split_on_spaces(&line);
				if( label.count <= 0 ) {
					// Which is required of course!
					fprintf(stderr, "%s:%lu: ERROR: define instruction require a label and a value.\n", fname, line_nb);
					return false;
				}

				line = stringview_trim(line);
				// And getting the associated value:
				StringView value = stringview_split_on_spaces(&line);
				Word word = {0};
				if( !stringview_number_litteral(value, &word) ) {
					// As of now, only number literal are supported:
					fprintf(stderr, "%s:%lu: ERROR: only numbers and strings can be associated to a label, not '%.*s'.\n", fname, line_nb, (int)directive.count, directive.data);
					return false;
				}

				if( !records_push_label(records, label, word) ) {
					fprintf(stderr, "%s:%lu: ERROR: label '%.*s' already exist.\n", fname, line_nb, (int)label.count, label.data);
					return false;
				}
			} else if( stringview_eq_cstr(directive, "include") ) {
				name = stringview_trim(stringview_split_on_spaces(&line));

				if( !stringview_startwith(name, '"') && !stringview_startwith(name, '<') ) {
					fprintf(stderr, "%s:%lu: ERROR: include directive expect a file name between \" or <>.\n", fname, line_nb);
					return false;
				}

				// Ignoring both surrounding character:
				name.count -= 2;
				// Ignoring " or <:
				name.data += 1;

				char *include_file = search_file(include_paths, name);
				if( include_file == NULL ) {
					fprintf(stderr, "%s:%lu: ERROR: Include file '%.*s' not found.\n", fname, line_nb, (int)name.count, name.data);
					return false;
				}

				StringView included = load_file(include_file);
				if( !translate_source(bl, include_paths, include_file, included, records) ) {
					fprintf(stderr, "%s:%lu: ERROR: error in included file '%.*s'.\n", fname, line_nb, (int)name.count, name.data);

					free(included.data);
					free(include_file);
					return false;
				}

				free(included.data);
				free(include_file);
			} else {
				// Uh oh, we don't know this one...
				fprintf(stderr, "%s:%lu: ERROR: unknown directive '%.*s'.\n", fname, line_nb, (int)directive.count, directive.data);
				return false;
			}

			continue;
		}

		if( stringview_endwith(name, ':') ) {
			if( !records_push_label(records, (StringView){.data = name.data, .count = name.count - 1}, (Word){.u64 = bl->prog.program_size}) ) {
				fprintf(stderr, "%s:%lu: ERROR: label '%.*s' already exist.\n", fname, line_nb, (int)name.count-1, name.data);
				return false;
			}
			do {
				name = stringview_split_on_spaces(&line);
			} while( name.count <= 0 && line.count > 0 );
		}

		if( name.count <= 0 || *name.data == BLASM_COMMENT_SYMBOL )
			continue;

		StringView operand = stringview_trim(stringview_split(&line, BLASM_COMMENT_SYMBOL));

		// 1. Convert the token name into an instruction:
		if( stringview_as_insttype(name, &inst.type) ) {
			// 2. Does this instruction require an operand:
			if( inst_required_operand[inst.type] ) {
				// If yes, convert it into a number:
				if( operand.count <= 0 ) {
					fprintf(stderr, "%s:%lu: ERROR: instruction '%.*s' require an operand.\n", fname, line_nb, (int)name.count, name.data);
					return false;
				} else if( !stringview_number_litteral(operand, &inst.operand) ) {
					// If we cannot convert it, it may be a label, so store it to be replaced later:
					records_push_unresolved(records, bl->prog.program_size, operand);
				}
			}

			bl->prog.program = realloc(bl->prog.program, (bl->prog.program_size + 1) * sizeof(struct inst_t));
			bl->prog.program[bl->prog.program_size++] = inst;
		} else {
			fprintf(stderr, "%s:%lu: ERROR: unknown instruction '%.*s'\n", fname, line_nb, (int)name.count, name.data);
			return false;
		}
	}

	for(size_t idx = 0; idx < records->jmps_size; idx++) {
		bl->prog.program[records->jmps[idx].addr].operand = records_find_label(*records, records->jmps[idx].name);

		if( bl->prog.program[records->jmps[idx].addr].operand.u64 == UINT64_MAX ) {
			fprintf(stderr, "%s: ERROR: undefined label '%.*s'.\n", fname, (int)records->jmps[idx].name.count, records->jmps[idx].name.data);
			return false;
		}
	}

	return true;
}

int main(int argc, const char *argv[]) {
	char *output = "a.bl";
	CList include_paths = NULL;

	Args *args = Args_New();
	Args_Error err;

	Args_Add(args, "-o", "--output", T_CHAR, &output, "Compiled program.");
	Args_Add(args, "-I", "--include", T_LIST, &include_paths, "List of include directory to search in.");

	err = Args_Parse(args, argc, argv);
	if( err == TREAT_ERROR ) {
		Args_Free(args);
		return EXIT_FAILURE;
	} else if( err == HELP ) {
		Args_Free(args);
		return EXIT_SUCCESS;
	} else if( args->rest == NULL ) {
		fprintf(stderr, "Missing arguments.");
		return EXIT_FAILURE;
	}

	const char *input = args->rest->opt;

	BlProg bl = {0};
	Records records = {0};

	bl.mem.memory_capacity = BLISP_STATIC_MEMORY_CAPACITY;

	StringView src = load_file(input);
	if( translate_source(&bl, include_paths, input, src, &records) )
		blprog_save_program_to_file(bl, output);

	blprog_clean(&bl);
	records_free(&records);
	free(src.data);

	for(CList tmp = include_paths; tmp != NULL; ) {
		CList free_lst = tmp;
		tmp = tmp->next;

		free(free_lst);
	}
	Args_Free(args);

	return EXIT_SUCCESS;
}
