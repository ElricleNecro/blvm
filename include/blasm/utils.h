#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "blvm/blvm.h"
#include "stringview.h"

bool stringview_number_litteral(StringView sv, Word *word);
bool stringview_as_insttype(StringView name, InstType *output);
bool file_exist(const char *name);

#endif
