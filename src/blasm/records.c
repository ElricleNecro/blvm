#include "records.h"

Word records_find_label(Records records, StringView name) {
	for(size_t idx=0; idx < records.labels_size; idx++)
		if( stringview_eq(records.labels[idx].name, name) )
			return records.labels[idx].word;

	return (Word){.u64 = UINT64_MAX};
}

bool records_push_label(Records *records, StringView name, Word word) {
	for(size_t idx=0; idx < records->labels_size; idx++)
		if( stringview_eq(records->labels[idx].name, name) )
			return false;

	records->labels = realloc(records->labels, (records->labels_size + 1) * sizeof(struct label_t));
	records->labels[records->labels_size++] = (Label){.name = stringview_memcopy(name), .word = word};

	return true;
}

void records_push_unresolved(Records *records, uint64_t addr, StringView name) {
	records->jmps = realloc(records->jmps, (records->jmps_size + 1) * sizeof(struct unresolved_t));
	records->jmps[records->jmps_size++] = (Unresolved){.addr = addr, .name = stringview_memcopy(name)};
}

void records_free(Records *records) {
	for(size_t idx=0; idx < records->jmps_size; idx++)
		free(records->jmps[idx].name.data);
	free(records->jmps), records->jmps = NULL, records->jmps_size = 0;
	for(size_t idx=0; idx < records->labels_size; idx++)
		free(records->labels[idx].name.data);
	free(records->labels), records->labels = NULL, records->labels_size = 0;
}
