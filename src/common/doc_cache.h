#ifndef _H_DOC_CACHE
#define _H_DOC_CACHE

#include "define.h" // typedef docid, maxSubnameLength

#define DOC_CACHE_SALT "3@Ky?@7{FeW}A8l?l3a)tcMfD{8[F?ss"
unsigned int sign_cache_params(docid doc_id, char *subname, time_t time);

#endif
