#ifndef _HEADER_QATTR_
#define _HEADER_QATTR_

#include <regex.h>
#include "../common/define.h"

#define REWRITE_COLL 1
#define REWRITE_ATTR 2

struct qrewrite {
	regex_t re;
	char *query;
	int query_len;
	char clean_query[MaxQueryLen];
};
typedef struct qrewrite qrewrite;

void qrewrite_init(qrewrite *q, char *query);
void qrewrite_destroy(qrewrite *q);
char *query_clean(qrewrite *q);
int query_attr_set_filter(char *dst, size_t dst_len, qrewrite *q, char *key, char *val, int filter_only);


#endif
