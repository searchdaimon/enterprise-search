#ifndef _SUFFIXTREE_H_
#define _SUFFIXTREE_H_

struct suggest_input;

#define forchildren(sf, root) for (sf = root->children; sf != NULL; sf = sf->next)

#define MAX_BEST 10

struct suffixtree {
	struct suggest_input *si;
	struct suffixtree *children, *next;
	char *suffix;
	int isroot;

	struct suggest_input *best[MAX_BEST+1]; /* Terminating NULL */
};

int suffixtree_init(struct suffixtree *);
void suffixtree_insert(struct suffixtree *, struct suggest_input *);
struct suggest_input *suffixtree_find_word(struct suffixtree *, char *);
int suffixtree_most_used(struct suffixtree *);
struct suggest_input **suffixtree_find_prefix(struct suffixtree *, char *, char *);


#endif
