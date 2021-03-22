#ifndef RECORDS_H
#define RECORDS_H

#include "blvm/blvm.h"
#include "stringview.h"

typedef struct label_t {
	StringView name;
	Word word;
} Label;

typedef struct unresolved_t {
	uint64_t addr;
	StringView name;
} Unresolved;

typedef struct records_t {
	Label *labels;
	size_t labels_size;

	Unresolved *jmps;
	size_t jmps_size;
} Records;

Word records_find_label(Records records, StringView name);
bool records_push_label(Records *records, StringView name, Word word);
void records_push_unresolved(Records *records, uint64_t addr, StringView name);
void records_free(Records *records);

#endif
