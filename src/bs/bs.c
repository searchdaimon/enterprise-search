#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bs.h"

void *bs_init(struct bs *s, int max) {

	s->count = 0;
	s->max = max;

	s->t = malloc(sizeof(void *) * s->max);
}

int bs_add(struct bs *s, void *data) {

	if ((s->count +1)> s->max) {
		printf("can't add to stack. Hav max elements from before.\n");
		return 0;
	}

	s->t[s->count] = data;

	++s->count;

	return 1;
}

int bs_get(struct bs *s, void **data) {

	if (s->count == 0) {
		printf("can't get from stack. Hav 0 elements.\n");
		return 0;
	}
	

	--s->count;

	*data = s->t[s->count];

	return 1;
}

