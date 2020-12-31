#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "stringview.h"

typedef struct label_t {
	StringView name;
	Word addr;
} Label;

typedef struct unresolved_t {
	Word addr;
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
			return records.labels[idx].addr;

	return -1;
}

void records_push_label(Records *records, StringView name, Word addr) {
	records->labels = realloc(records->labels, (records->labels_size + 1) * sizeof(struct label_t));
	records->labels[records->labels_size++] = (Label){.name = name, .addr = addr};
}

void records_push_unresolved(Records *records, Word addr, StringView name) {
	records->jmps = realloc(records->jmps, (records->jmps_size + 1) * sizeof(struct unresolved_t));
	records->jmps[records->jmps_size++] = (Unresolved){.addr = addr, .name = name};
}

void records_free(Records *records) {
	free(records->labels), records->labels = NULL, records->labels_size = 0;
}

void translate_source(Blvm *bl, StringView src, Records *records) {
	if( bl->program != NULL )
		free(bl->program), bl->program = NULL, bl->program_size = 0;

	while( src.count > 0 ) {
		StringView line = stringview_split(&src, '\n');
		line = stringview_trim(line);

		if( line.count <= 0 || *line.data == '#' )
			continue;

		line = stringview_trim(line);

		Inst inst;
		StringView name = stringview_split(&line, ' ');

		if( stringview_endwith(name, ':') ) {
			records_push_label(records, (StringView){.data = name.data, .count = name.count - 1}, bl->program_size);
			do {
				name = stringview_split(&line, ' ');
			} while( name.count <= 0 && line.count > 0 );
		}

		if( name.count <= 0 || *line.data == '#' )
			continue;

		StringView operand = stringview_trim(stringview_split(&line, '#'));
		if( stringview_eq(name, cstr_as_stringview("push")) ) {
			int value = stringview_to_int(operand);
			inst.type = INST_PUSH;
			inst.operand = value;
		} else if( stringview_eq(name, cstr_as_stringview("pop")) ) {
			inst.type = INST_POP;
		} else if( stringview_eq(name, cstr_as_stringview("dup")) ) {
			int value = stringview_to_int(operand);
			inst.type = INST_DUP;
			inst.operand = value;
		} else if( stringview_eq(name, cstr_as_stringview("add")) ) {
			inst.type = INST_ADD;
		} else if( stringview_eq(name, cstr_as_stringview("sub")) ) {
			inst.type = INST_SUB;
		} else if( stringview_eq(name, cstr_as_stringview("mul")) ) {
			inst.type = INST_MUL;
		} else if( stringview_eq(name, cstr_as_stringview("div")) ) {
			inst.type = INST_DIV;
		} else if( stringview_eq(name, cstr_as_stringview("jmp")) ) {
			inst.type = INST_JMP;
			if( operand.count > 0 && isdigit(*operand.data) ) {
				inst.operand = stringview_to_int(operand);
			} else {
				records_push_unresolved(records, bl->program_size, operand);
			}
		} else if( stringview_eq(name, cstr_as_stringview("jif")) ) {
			inst.type = INST_JIF;
			if( operand.count > 0 && isdigit(*operand.data) ) {
				inst.operand = stringview_to_int(operand);
			} else {
				records_push_unresolved(records, bl->program_size, operand);
			}
		} else if( stringview_eq(name, cstr_as_stringview("eq")) ) {
			inst.type = INST_EQ;
		} else if( stringview_eq(name, cstr_as_stringview("gt")) ) {
			inst.type = INST_GT;
		} else if( stringview_eq(name, cstr_as_stringview("halt")) ) {
			inst.type = INST_HALT;
		} else if( stringview_eq(name, cstr_as_stringview("debug")) ) {
			inst.type = INST_PRINT_DEBUG;
		} else {
			fprintf(stderr, "ERROR: unknown instruction '%.*s'\n", (int)name.count, name.data);
			exit(EXIT_FAILURE);
		}

		bl->program = realloc(bl->program, (bl->program_size + 1) * sizeof(struct inst_t));
		bl->program[bl->program_size++] = inst;
	}

	for(size_t idx = 0; idx < records->jmps_size; idx++) {
		bl->program[records->jmps[idx].addr].operand = records_find_label(*records, records->jmps[idx].name);

		if( bl->program[records->jmps[idx].addr].operand < 0 ) {
			fprintf(stderr, "ERROR: undefined label '%.*s'.\n", (int)records->jmps[idx].name.count, records->jmps[idx].name.data);
			exit(EXIT_FAILURE);
		}
	}
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

int main(int argc, const char *argv[]) {
	char *output = "a.bl";

	Args *args = Args_New();
	Args_Error err;

	Args_Add(args, "-o", "--output", T_CHAR, &output, "Compiled program.");

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
	translate_source(&bl, src, &records);
	blvm_save_program_to_file(bl, output);

	blvm_clean(&bl);
	records_free(&records);
	free(src.data);

	return EXIT_SUCCESS;
}
