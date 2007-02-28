
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


int suggest_read_frequency(struct suggest_data *, char *);

#endif

