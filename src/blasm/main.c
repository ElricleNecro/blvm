#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "stringview.h"

Inst translate_line(StringView line) {
	line = stringview_ltrim(line);
	StringView name = stringview_split(&line, ' ');

	if( stringview_eq(name, cstr_as_stringview("push")) ) {
		line = stringview_ltrim(line);
		int operand = stringview_to_int(stringview_rtrim(line));
		return (Inst){.type = INST_PUSH, .operand = operand};
	} else if( stringview_eq(name, cstr_as_stringview("pop")) ) {
		return (Inst){.type = INST_POP,};
	} else if( stringview_eq(name, cstr_as_stringview("dup")) ) {
		line = stringview_ltrim(line);
		int operand = stringview_to_int(stringview_rtrim(line));
		return (Inst){.type = INST_DUP, .operand = operand};
	} else if( stringview_eq(name, cstr_as_stringview("add")) ) {
		return (Inst){.type = INST_ADD};
	} else if( stringview_eq(name, cstr_as_stringview("sub")) ) {
		return (Inst){.type = INST_SUB};
	} else if( stringview_eq(name, cstr_as_stringview("mul")) ) {
		return (Inst){.type = INST_MUL};
	} else if( stringview_eq(name, cstr_as_stringview("div")) ) {
		return (Inst){.type = INST_DIV};
	} else if( stringview_eq(name, cstr_as_stringview("jmp")) ) {
		line = stringview_ltrim(line);
		int operand = stringview_to_int(stringview_rtrim(line));
		return (Inst){.type = INST_JMP, .operand = operand};
	} else if( stringview_eq(name, cstr_as_stringview("jif")) ) {
		line = stringview_ltrim(line);
		int operand = stringview_to_int(stringview_rtrim(line));
		return (Inst){.type = INST_JIF, .operand = operand};
	} else if( stringview_eq(name, cstr_as_stringview("eq")) ) {
		return (Inst){.type = INST_EQ};
	} else if( stringview_eq(name, cstr_as_stringview("gt")) ) {
		return (Inst){.type = INST_GT};
	} else if( stringview_eq(name, cstr_as_stringview("halt")) ) {
		return (Inst){.type = INST_HALT};
	} else if( stringview_eq(name, cstr_as_stringview("debug")) ) {
		return (Inst){.type = INST_PRINT_DEBUG};
	} else {
		fprintf(stderr, "ERROR: unknown instruction '%.*s'\n", (int)name.count, name.data);
		exit(EXIT_FAILURE);
	}

	return (Inst){0};
}

size_t translate_source(StringView src, Inst **program) {
	size_t program_size = 0;

	while( src.count > 0 ) {
		StringView line = stringview_split(&src, '\n');
		line = stringview_trim(line);

		if( line.count <= 0 )
			continue;

		*program = realloc(*program, (program_size + 1) * sizeof(struct inst_t));
		(*program)[program_size++] = translate_line(line);
	}

	return program_size;
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

	Args_Add(args, "-o", "--output", T_CHAR, &output, "LUA configuration file.");

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

	StringView src = load_file(input);
	bl.program_size = translate_source(src, &bl.program);
	blvm_save_program_to_file(bl, output);

	blvm_clean(&bl);
	free(src.data);

	return EXIT_SUCCESS;
}
