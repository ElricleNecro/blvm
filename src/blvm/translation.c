#include "blvm/translation.h"

bool file_exist(const char *name) {
	return access(name, F_OK | R_OK) == 0;
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
		.entry_point = bl.entry_point,
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

char* search_file(const IncludeList paths, StringView include) {
	char *results = NULL;

	for(IncludeList tmp = paths; tmp != NULL; tmp = tmp->next) {
		results = realloc(results, (strlen(tmp->fname) + include.count + 2) * sizeof(char));
		snprintf(results, strlen(tmp->fname) + include.count + 2, "%s/%.*s", tmp->fname, (int)include.count, include.data);

		if( file_exist(results) )
			return results;
	}

	free(results), results = NULL;
	return NULL;
}

bool translate_litteral(BlProg *bl, StringView sv, Word *word, const char *fname, const size_t line_nb) {
	if( stringview_startwith(sv, '"') ) {
		sv.count -= 1;
		sv.data += 1;

		StringView str = stringview_split(&sv, '"');

		if( bl->mem.memory_size + str.count > bl->mem.memory_capacity ) {
			fprintf(stderr, "%s:%lu: ERROR: not enough memory capacity to store the string '%.*s' (need: %lu, available: %lu).\n", fname, line_nb, (int)str.count, str.data, bl->mem.memory_size + str.count, bl->mem.memory_capacity);
			return false;
		}

		(*word).u64 = bl->mem.memory_size;

		bl->mem.memory = realloc(bl->mem.memory, (bl->mem.memory_size + str.count) * sizeof(uint8_t));

		// Copying the string into the VM memory:
		memcpy(&bl->mem.memory[bl->mem.memory_size], str.data, str.count);
		bl->mem.memory_size += str.count;
	} if( stringview_startwith(sv, '\'') ) {
		sv.data += 1;
		sv.count -= 1;

		StringView value = stringview_split(&sv, '\'');

		if( value.count != 1 ) {
			fprintf(stderr, "%s:%lu: ERROR: only character can be stored using \"'\", not '%.*s'.\n", fname, line_nb, (int)value.count, value.data);
			return false;
		}

		(*word).u64 = (uint64_t)value.data[0];
	} else {
		StringView value = stringview_split_on_spaces(&sv);
		if( !stringview_number_litteral(value, word) ) {
			return false;
		}
	}

	return true;
}

bool translate_source(BlProg *bl, const IncludeList include_paths, const char *fname, StringView src, Records *records) {
	if( bl->include_level >= BLASM_INCLUDE_LEVEL_MAX ) {
		fprintf(stderr, "%s: Too many include level (gone through %lu include).", fname, bl->include_level);
		return false;
	}

	StringView entry_point = {0};

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
			if( stringview_eq_cstr(directive, "entry_point") ) {
				StringView operand = stringview_trim(stringview_split(&line, BLASM_COMMENT_SYMBOL));
				entry_point = operand;
			} else if( stringview_eq_cstr(directive, "memory") ) {
				StringView operand = stringview_trim(stringview_split(&line, BLASM_COMMENT_SYMBOL));
				uint64_t cap = 0;

				if( !stringview_to_ulong(operand, &cap) ) {
					fprintf(stderr, "%s:%lu: ERROR: memory instruction require a long value (got '%.*s').\n", fname, line_nb, (int)operand.count, operand.data);
					return false;
				}

				bl->mem.memory_capacity = cap;
			} else if( stringview_eq_cstr(directive, "define") ) {
				// Extracting the label name:
				StringView label = stringview_split_on_spaces(&line);
				if( label.count <= 0 ) {
					// Which is required of course!
					fprintf(stderr, "%s:%lu: ERROR: define instruction require a label and a value.\n", fname, line_nb);
					return false;
				}

				line = stringview_trim(line);
				StringView operand = stringview_trim(stringview_split(&line, BLASM_COMMENT_SYMBOL));

				Word word = {0};

				if( ! translate_litteral(bl, operand, &word, fname, line_nb) ) {
					fprintf(stderr, "%s:%lu: ERROR: unable to convert '%.*s' into a valid value.\n", fname, line_nb, (int)operand.count, operand.data);
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
				bl->include_level += 1;
				if( !translate_source(bl, include_paths, include_file, included, records) ) {
					fprintf(stderr, "%s:%lu: ERROR: error in included file '%.*s'.\n", fname, line_nb, (int)name.count, name.data);

					free(included.data);
					free(include_file);
					return false;
				}
				bl->include_level -= 1;

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
				} else if( !translate_litteral(bl, operand, &inst.operand, fname, line_nb) && ! stringview_startwith(operand, '"') && ! stringview_startwith(operand, '\'') ) {
					// If we cannot convert it, it may be a label, so store it to be replaced later:
					records_push_unresolved(records, bl->prog.program_size, operand, cstr_as_stringview((char*)fname), line_nb);
				}
			}

			bl->prog.program = realloc(bl->prog.program, (bl->prog.program_size + 1) * sizeof(struct inst_t));
			bl->prog.program[bl->prog.program_size++] = inst;
		} else {
			fprintf(stderr, "%s:%lu: ERROR: unknown instruction '%.*s'\n", fname, line_nb, (int)name.count, name.data);
			return false;
		}
	}

	if( entry_point.count > 0 ) {
		Word addr = {0};
		if( stringview_number_litteral(entry_point, &addr) ) {
			bl->entry_point = addr.u64;
		} else {
			bl->entry_point = records_find_label(*records, entry_point).u64;

			if( bl->entry_point == UINT64_MAX ) {
				fprintf(stderr, "ERROR: undefined label for the entry point '%.*s'.\n", (int)entry_point.count, entry_point.data);
				return false;
			}
		}
	} else if( bl->include_level == 0 ) {
		fprintf(stderr, "%s: ERROR: No entry point was declared, this is mandatory.\n", fname);
		fprintf(stderr, "You will want to add the following instruction to your program:\n");
		fprintf(stderr, "\t- `%centry_point <label>`,\n", BLASM_PREPRO_SYMBOL);
		fprintf(stderr, "\t- `%centry_point <addr>`.\n", BLASM_PREPRO_SYMBOL);
	}

	for(size_t idx = 0; idx < records->jmps_size; idx++) {
		bl->prog.program[records->jmps[idx].addr].operand = records_find_label(*records, records->jmps[idx].name);

		if( bl->prog.program[records->jmps[idx].addr].operand.u64 == UINT64_MAX ) {
			fprintf(stderr, "%.*s:%lu: ERROR: undefined label '%.*s'.\n", (int)records->jmps[idx].fname.count, records->jmps[idx].fname.data, records->jmps[idx].src_loc, (int)records->jmps[idx].name.count, records->jmps[idx].name.data);
			return false;
		}
	}

	return true;
}

bool stringview_number_litteral(StringView sv, Word *word) {
	Word result = {0};
	char *endptr = NULL;

	if( *sv.data == '-' ){
		result.i64 = strtol(sv.data, &endptr, 0);
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
