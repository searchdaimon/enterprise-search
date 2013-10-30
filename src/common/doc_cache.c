#include <stdio.h>

#include "doc_cache.h"
#include "crc32.h" // cdc32boitho


unsigned int sign_cache_params(docid doc_id, char *subname, time_t time) {
	char data[maxSubnameLength + 128];

	snprintf(data, sizeof data, "%u %s %lld %s", 
		doc_id, subname, (long long)time, DOC_CACHE_SALT);

	return crc32boitho(data);
}

