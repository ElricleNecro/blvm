#include <stdio.h>
#include <stdlib.h>

#include "Parser.h"

#include "blvm/blvm.h"
#include "blvm/records.h"
#include "blvm/stringview.h"
#include "blvm/translation.h"

int main(int argc, const char *argv[]) {
	char *output = "a.bl";
	CList include_paths_opt = NULL;

	Args *args = Args_New();
	Args_Error err;

	Args_Add(args, "-o", "--output", T_CHAR, &output, "Compiled program.");
	Args_Add(args, "-I", "--include", T_LIST, &include_paths_opt, "List of include directory to search in.");

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
	IncludeList include_paths = NULL;

	if( include_paths_opt != NULL ) {
		for(CList tmp = include_paths_opt; tmp != NULL; tmp = tmp->next) {
			if( include_paths == NULL ) {
				include_paths = malloc(sizeof(struct _include_list_t));
				include_paths->fname = tmp->opt;
				include_paths->next = NULL;
				include_paths->end = include_paths;
			} else {
				include_paths->end = malloc(sizeof(struct _include_list_t));
				include_paths->end->fname = tmp->opt;
				include_paths->end->next = NULL;
				for(IncludeList iter = include_paths->next; iter != NULL; iter = iter->next)
					iter->end = include_paths->end;
			}
		}
	}

	BlProg bl = {0};
	Records records = {0};

	bl.mem.memory_capacity = BLISP_STATIC_MEMORY_CAPACITY;

	StringView src = load_file(input);
	if( translate_source(&bl, include_paths, input, src, &records) )
		blprog_save_program_to_file(bl, output);

	blprog_clean(&bl);
	records_free(&records);
	free(src.data);

	for(IncludeList tmp = include_paths; tmp != NULL; ) {
		IncludeList free_lst = tmp;
		tmp = tmp->next;

		free(free_lst);
	}
	clist_free(include_paths_opt);
	Args_Free(args);

	return EXIT_SUCCESS;
}
