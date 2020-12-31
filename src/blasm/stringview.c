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

bool stringview_eq(StringView a, StringView b) {
	if( a.count != b.count )
		return false;

	return memcmp(a.data, b.data, a.count) == 0;
}

bool stringview_endwith(StringView sv, const char end) {
	if( sv.count > 0 && sv.data[sv.count - 1] == end)
		return true;
	return false;
}

int stringview_to_int(StringView sv) {
	int result = 0;

	for(size_t i = 0; i < sv.count && isdigit(sv.data[i]); i++)
		result = result * 10 + sv.data[i] - '0';

	return result;
}
