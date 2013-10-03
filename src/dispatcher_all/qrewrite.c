#include "../common/define.h"
#include <regex.h>
#include <err.h>

#include "qrewrite.h"

#include "../common/strlcpy.h"


#define ATTR_REGEX "attribute:\"[a-z0-9A-Z ]+=.+\""
#define COLL_REGEX "collection:[a-z0-9A-Z_\.]+"


void qrewrite_init(qrewrite *q, char *query) {
	char errmsg[256];
	int error;
	int rewrite = REWRITE_COLL;

	switch (rewrite) {
		case REWRITE_COLL:
			if ((error = regcomp(&q->re, ATTR_REGEX, REG_ICASE | REG_EXTENDED))) {
				regerror(error, &q->re, errmsg, sizeof errmsg);
				errx(1, "regcomp failed with error '%s'\n", errmsg);
			}
			break;
		default:
			errx(1, "%s:%d Unsupported rewrite: %d", __FILE__, __LINE__, rewrite);
	}

	q->query = query;
	q->query_len = strlen(query);
	if (q->query_len > MaxQueryLen)
		errx(1, "%s:%d query is larger than MaxQueryLen", __FILE__, __LINE__);
}

void qrewrite_destroy(qrewrite *q) {
	regfree(&q->re);
}

char *query_clean(qrewrite *q) {
	char errmsg[256];
	int error;

	int dst_pos = 0;
	int orig_pos = 0;
	char *dst = q->clean_query;
	char *query = q->query;

	// copy all but attributes
	while (1) {
		regmatch_t pmatch[1];
		if ((error = regexec(&q->re, &(query[orig_pos]), 1, pmatch, 0))) {
			if (error != REG_NOMATCH) {
				regerror(error, &q->re, errmsg, sizeof errmsg);
				errx(1, "regexec error '%s' on query %s", errmsg, q->query);
			}
			break;
		}

		int i = 0;
		for (; i < pmatch[0].rm_so; i++)
			dst[dst_pos++] = query[orig_pos + i];

		orig_pos += pmatch[0].rm_eo;
	}

	// copy remaining chars
	for (; orig_pos < (q->query_len); orig_pos++)
		dst[dst_pos++] = query[orig_pos];


	dst[dst_pos] = '\0';
	return dst;
}

int query_attr_set_filter(char *dst, size_t dst_len, qrewrite *q, char *key, char *val, int filter_only) {
	int ret;
	if (filter_only) {
		ret = snprintf(dst, dst_len, "attribute:\"%s=%s\"", key, val);
	}
	else {
		char *clean_query = query_clean(q);
		ret = snprintf(dst, dst_len, "%s attribute:\"%s=%s\"", clean_query, key, val);
	}
	//warnx("query: %s, clean: %s, dst: %s\n", q->query, clean_query, dst);
	return ret;
}

/*
int main(void) {
	char attrkey[] = "author";
	char attrval[] = "bunarr";
	char dst[MaxQueryLen];

	char query[] = "fast attribute:\"pages=8\" search attribute:\"creator=bob\" solution";
	printf("before: %s\n", query);
	query_attr qattr;
	query_attr_init(&qattr, query);
	query_attr_set_filter(dst, sizeof dst, &qattr, attrkey, attrval);
	query_attr_destroy(&qattr);
	printf("after : %s\n", dst);
}
*/
