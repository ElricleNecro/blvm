#include "stringview.h"

StringView cstr_as_stringview(char *cstr) {
	return (StringView) {
		.count = strlen(cstr),
		.data = cstr,
	};
}

StringView stringview_ltrim(StringView sv) {
	size_t i = 0;

	while( i < sv.count && isspace(sv.data[i]) )
		i += 1;

	return (StringView) {
		.count = sv.count - i,
		.data = sv.data + i,
	};
}

StringView stringview_rtrim(StringView sv) {
	size_t i = 0;

	while( i < sv.count && isspace(sv.data[sv.count - 1 - i]) )
		i += 1;

	return (StringView) {
		.count = sv.count - i,
		.data = sv.data,
	};
}

StringView stringview_trim(StringView sv) {
	return stringview_ltrim(stringview_rtrim(sv));
}

StringView stringview_split(StringView *sv, const char delim) {
	size_t i =0;
	while( i < sv->count && sv->data[i] != delim )
		i += 1;

	StringView result = {
		.count = i,
		.data = sv->data,
	};

	if( i < sv->count ) {
		sv->count -= i + 1;
		sv->data += i + 1;
	} else {
		sv->count -= i;
		sv->data += i;
	}

	return result;
}

StringView stringview_split_on_spaces(StringView *sv) {
	size_t i =0;
	while( i < sv->count && ! isspace(sv->data[i]) )
		i += 1;

	StringView result = {
		.count = i,
		.data = sv->data,
	};

	if( i < sv->count ) {
		sv->count -= i + 1;
		sv->data += i + 1;
	} else {
		sv->count -= i;
		sv->data += i;
	}

	return result;
}

StringView stringview_memcopy(StringView sv) {
	StringView copy = {
		.data = malloc(sv.count * sizeof(char)),
		.count = sv.count,
	};

	memcpy(copy.data, sv.data, sv.count);

	return copy;
}

bool stringview_eq(StringView a, StringView b) {
	if( a.count != b.count )
		return false;

	return memcmp(a.data, b.data, a.count) == 0;
}

bool stringview_eq_cstr(StringView a, const char *b) {
	if( a.count != strlen(b) )
		return false;

	return memcmp(a.data, b, a.count) == 0;
}

bool stringview_endwith(StringView sv, const char end) {
	if( sv.count > 0 && sv.data[sv.count - 1] == end)
		return true;
	return false;
}

bool stringview_startwith(StringView sv, const char start) {
	if( sv.count > 0 && sv.data[0] == start)
		return true;
	return false;
}

int stringview_to_int(StringView sv) {
	int result = 0;

	for(size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++)
		result = result * 10 + sv.data[i] - '0';

	return result;
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
	if( (src = malloc((size_t)fsize)) == NULL ) {
		fprintf(stderr, "ERROR: Could not allocate memory for file '%s': %s\n", fpath, strerror(errno));
		exit(EXIT_FAILURE);
	}

	size_t n = fread(src, sizeof(char), (size_t)fsize, file);
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
