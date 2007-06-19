/*
 * Exchange crawler
 *
 * June, 2007
 */

struct ex_buffer {
	size_t size;
	unsigned int modified;
	char *buf;
};

char *ex_getEmail(const char *, const char *, const char *, struct ex_buffer *);
char *ex_getContent(const char *, const char *, const char *);

