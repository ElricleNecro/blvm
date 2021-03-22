#include "utils.h"

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

bool file_exist(const char *name) {
	return access(name, F_OK | R_OK) == 0;
}
