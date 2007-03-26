
#ifndef _SUGGEST_H_
#define _SUGGEST_H_

#include "suffixtree.h"

struct suggest_data {
	struct suffixtree tree;
};

struct suggest_input {
	char *word;
	int frequency;
};


void suggest_most_used(struct suggest_data *);
int suggest_read_frequency(struct suggest_data *, char *);
struct suggest_input **suggest_find_prefix(struct suggest_data *, char *);
struct suggest_data *suggest_init(void);

#endif

