#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "stringview.h"

#ifndef BLASM_COMMENT_SYMBOL
#define BLASM_COMMENT_SYMBOL ';'
#endif

#ifndef BLASM_PREPRO_SYMBOL
#define BLASM_PREPRO_SYMBOL '%'
#endif

typedef struct label_t {
	StringView name;
	Word word;
} Label;

typedef struct unresolved_t {
	uint64_t addr;
	StringView name;
} Unresolved;

typedef struct records_t {
	Label *labels;
	size_t labels_size;

	Unresolved *jmps;
	size_t jmps_size;
} Records;

Word records_find_label(Records records, StringView name) {
	for(size_t idx=0; idx < records.labels_size; idx++)
		if( stringview_eq(records.labels[idx].name, name) )
			return records.labels[idx].word;

	return (Word){.u64 = UINT64_MAX};
}

bool records_push_label(Records *records, StringView name, Word word) {
	for(size_t idx=0; idx < records->labels_size; idx++)
		if( stringview_eq(records->labels[idx].name, name) )
			return false;

	records->labels = realloc(records->labels, (records->labels_size + 1) * sizeof(struct label_t));
	records->labels[records->labels_size++] = (Label){.name = stringview_memcopy(name), .word = word};

	return true;
}

void records_push_unresolved(Records *records, uint64_t addr, StringView name) {
	records->jmps = realloc(records->jmps, (records->jmps_size + 1) * sizeof(struct unresolved_t));
	records->jmps[records->jmps_size++] = (Unresolved){.addr = addr, .name = stringview_memcopy(name)};
}

void records_free(Records *records) {
	for(size_t idx=0; idx < records->jmps_size; idx++)
		free(records->jmps[idx].name.data);
	free(records->jmps), records->jmps = NULL, records->jmps_size = 0;
	for(size_t idx=0; idx < records->labels_size; idx++)
		free(records->labels[idx].name.data);
	free(records->labels), records->labels = NULL, records->labels_size = 0;
}

bool stringview_number_litteral(StringView sv, Word *word) {
	Word result = {0};
	char *endptr = NULL;

	if( *sv.data == '-' ){
		result.i64 = strtoul(sv.data, &endptr, 0);
	} else {
		result.u64 = strtoul(sv.data, &endptr, 0);
	}

	if( (size_t)(endptr - sv.data) != sv.count )
		result.f64 = strtod(sv.data, &endptr);

	if( (size_t)(endptr - sv.data) != sv.count ) {
		/*fprintf(stderr, "ERROR: unrecognised number type: '%.*s', parsing stopped at character: '%c'.\n", (int)sv.count, sv.data, *endptr);*/
		/*exit(EXIT_FAILURE);*/
		return false;
	}

	*word = result;
	return true;
}

bool stringview_as_insttype(StringView name, InstType *output) {
	for(InstType type = (InstType)0; type < NUMBER_OF_INSTS; type += 1) {
		if( stringview_eq(name, cstr_as_stringview((char*)inst_names[type])) ) {
			*output = type;
			return true;
		}
	}

	return false;
}

StringView load_file(const char *fpath) {
	FILE *file = NULL;
	if( (file = fopen(fpath, "r")) == NULL ) {
		fprintf(stderr, "ERROR: Could not open file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if( fseek(file, 0, SEEK_END) < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	long fsize = ftell(file);
	if( fsize < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if( fseek(file, 0, SEEK_SET) < 0 ) {
		fprintf(stderr, "ERROR: Could not read file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char *src = NULL;
	if( (src = malloc(fsize)) == NULL ) {
		fprintf(stderr, "ERROR: Could not allocate memory for file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	size_t n = fread(src, sizeof(char), fsize, file);
	if( ferror(file) ) {
		fprintf(stderr, "ERROR: Could not read from file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fclose(file);

	return (StringView) {
		.count = n,
		.data = src,
	};
}

bool file_exist(const char *name) {
	return access(name, F_OK | R_OK) == 0;
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

bool translate_source(Blvm *bl, const CList include_paths, const char *fname, StringView src, Records *records) {
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
					fprintf(stderr, "%s:%lu: ERROR: only numbers can be associated to a label, not '%.*s'.\n", fname, line_nb, (int)directive.count, directive.data);
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
			if( !records_push_label(records, (StringView){.data = name.data, .count = name.count - 1}, (Word){.u64 = bl->program_size}) ) {
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
					records_push_unresolved(records, bl->program_size, operand);
				}
			}

			bl->program = realloc(bl->program, (bl->program_size + 1) * sizeof(struct inst_t));
			bl->program[bl->program_size++] = inst;
		} else {
			fprintf(stderr, "%s:%lu: ERROR: unknown instruction '%.*s'\n", fname, line_nb, (int)name.count, name.data);
			return false;
		}
	}

	for(size_t idx = 0; idx < records->jmps_size; idx++) {
		bl->program[records->jmps[idx].addr].operand = records_find_label(*records, records->jmps[idx].name);

		if( bl->program[records->jmps[idx].addr].operand.u64 == UINT64_MAX ) {
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

	Blvm bl = {0};
	Records records = {0};

	StringView src = load_file(input);
	if( translate_source(&bl, include_paths, input, src, &records) )
		blvm_save_program_to_file(bl, output);

	blvm_clean(&bl);
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
