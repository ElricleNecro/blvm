#ifndef STRINGVIEW_H_SLZDE4UB
#define STRINGVIEW_H_SLZDE4UB

#include <ctype.h>
#include <stdbool.h>
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
bool stringview_eq(StringView a, StringView b);
int stringview_to_int(StringView sv);



#endif /* end of include guard: STRINGVIEW_H_SLZDE4UB */
