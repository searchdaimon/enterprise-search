
#ifndef _SUGGEST_H_
#define _SUGGEST_H_

#include "suffixtree.h"

struct suggest_data {
	struct suffixtree tree;
};

struct suggest_input {
	char *word;
	int frequency;
#ifdef WITH_ACL
	char **aclallow;
	char **acldeny;
#endif
};


void suggest_most_used(struct suggest_data *);
int suggest_read_frequency(struct suggest_data *, char *);
#ifdef WITH_ACL
struct suggest_input **suggest_find_prefix(struct suggest_data *, char *, char *, char ***, int *);
#else
struct suggest_input **suggest_find_prefix(struct suggest_data *, char *);
#endif
struct suggest_data *suggest_init(void);
void suggest_destroy(struct suggest_data *);

#endif

