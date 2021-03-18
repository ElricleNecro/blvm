#ifndef STRINGVIEW_H_SLZDE4UB
#define STRINGVIEW_H_SLZDE4UB

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct string_view_t {
	size_t count;
	char *data;
} StringView ;

StringView cstr_as_stringview(char *cstr);
StringView stringview_ltrim(StringView sv);
StringView stringview_rtrim(StringView sv);
StringView stringview_trim(StringView sv);
StringView stringview_split(StringView *sv, const char delim);
StringView stringview_split_on_spaces(StringView *sv);
StringView stringview_memcopy(StringView sv);
bool stringview_eq(StringView a, StringView b);
bool stringview_eq_cstr(StringView a, const char *b);
bool stringview_endwith(StringView sv, const char end);
bool stringview_startwith(StringView sv, const char start);
int stringview_to_int(StringView sv);

#endif /* end of include guard: STRINGVIEW_H_SLZDE4UB */
