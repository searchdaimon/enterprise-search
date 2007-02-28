
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <errno.h>

#include "suffixtree.h"
#include "suggest.h"

static int
suffixtree_allocate_node(struct suffixtree *node)
{
	node->next = NULL;
	node->si = NULL;
	node->children = NULL;
	node->isroot = 0;
	node->suffix = NULL;

	return 0;
}

int
suffixtree_init(struct suffixtree *st)
{
	suffixtree_allocate_node(st);
	st->si = NULL;
	st->suffix = NULL;
	st->isroot = 1;

	return 0;
}


static int
find_common_substr(const char *str1, const char *str2)
{
	int len;

	for (len = 0; str1[len] == str2[len] && str1[len] != '\0'; len++)
		;

	return len;
}

void _suffixtree_insert(struct suffixtree *, struct suggest_input *, unsigned int);

static struct suffixtree *
_suffixtree_insert_new_node(void)
{
	struct suffixtree *sf;

	if ((sf = malloc(sizeof(*sf))) == NULL)
		return NULL;
	suffixtree_allocate_node(sf);

	return sf;
}

static void
_suffixtree_delete_node(struct suffixtree *sf)
{
	struct suffixtree *child;

	forchildren(child, sf)
		_suffixtree_delete_node(child);

	if (sf->suffix)
		free(sf->suffix);

	free(sf);
}

static struct suffixtree *
_suffixtree_insert_find_child(struct suffixtree *root, struct suggest_input *data, unsigned int suffixlen)
{
	struct suffixtree *sf;
	unsigned int i;

	forchildren(sf, root) {
		i = find_common_substr(data->word + suffixlen, sf->suffix);
		if (i == strlen(sf->suffix)) {
			return sf;
		}
		else if (i != 0) {
			/* Split up node */
			struct suffixtree *sf2;

			if ((sf2 = _suffixtree_insert_new_node()) == NULL)
				return NULL;
			if ((sf2->suffix = strdup(sf->suffix + i)) == NULL) {
				_suffixtree_delete_node(sf2);
				return NULL;
			}
			sf2->children = sf->children;
			sf->children = sf2;
			free(sf->suffix);
			if ((sf->suffix = strndup(data->word + suffixlen, i)) == NULL) {
				_suffixtree_delete_node(sf);
				return NULL;
			}
			if (sf->si) {
				sf2->si = sf->si;
				sf->si = NULL;
			}

			return sf;
		}
	}
	
	sf = _suffixtree_insert_new_node();
	sf->next = root->children;
	root->children = sf;

	return sf;
}

void
_suffixtree_insert(struct suffixtree *root, struct suggest_input *data, unsigned int suffixlen)
{
	struct suffixtree *sf;

	if (root->suffix == NULL && !root->isroot) {
		if ((root->suffix = strdup(data->word + suffixlen)) == NULL)
			return;
		root->si = data;
	}
	else if (suffixlen == strlen(data->word) && root->si == NULL) {
		root->si = data;
	}
	else {
		sf = _suffixtree_insert_find_child(root, data, suffixlen);
		_suffixtree_insert(sf, data, suffixlen + (sf->suffix ? strlen(sf->suffix) : 0));
	}
}

void
suffixtree_insert(struct suffixtree *root, struct suggest_input *data)
{
	_suffixtree_insert(root, data, 0);
}

#if MAX_BEST <= 0
#error "MAX_BEST must be at least 1"
#endif

int
suffixtree_most_used(struct suffixtree *root)
{
	struct suffixtree *sf;
	int pos = 0, i = 0;
	int children = 0;
	int *bookkeeping;
	int usedthis = 0;

	if (root->children == NULL) {
		root->best[pos++] = root->si;
		root->best[pos] = NULL;
		return 0;
	}

	forchildren(sf, root) {
		suffixtree_most_used(sf);
		children++;
	}	

	if ((bookkeeping = malloc(sizeof(int) * children)) == NULL)
		return -1;
	bzero(bookkeeping, sizeof(int) * children);

	for (i = 0; i < MAX_BEST; i++) {
		int min, j, k;
		struct suffixtree *sfmin;

		sfmin = root->children;	
		min = 0;
		for (k = 0; k < children; k++) {
			if (k != 0) {
				sfmin = sfmin->next;
				min = k;
			}
			if (sfmin->best[bookkeeping[min]])
				break;
		}
		if (sfmin->best[bookkeeping[min]] == NULL)
			break;
		j = 0;
		forchildren(sf, root) {
			if (sf->best[bookkeeping[j]] == NULL) {
				j++;
				continue;
			}

			if (sfmin->best[bookkeeping[min]]->frequency <
			    sf->best[bookkeeping[j]]->frequency) {
				min = j;
				sfmin = sf;
			}
			j++;
		}
		if (usedthis == 0 &&
		    (sfmin->best[bookkeeping[min]] == NULL ||
		     (root->si &&
		      root->si->frequency >= sfmin->best[bookkeeping[min]]->frequency))) {
			usedthis = 1;
			root->best[i] = root->si;
			continue;
		}
		if (sfmin->best[bookkeeping[min]] == NULL)
			break;
		root->best[i] = sfmin->best[bookkeeping[min]];
		bookkeeping[min]++;
	}
	root->best[i] = NULL;

	return 0;
}

struct suggest_input *_suffixtree_find_word(struct suffixtree *, char *, unsigned int);

static struct suggest_input *
_suffixtree_find_word_children(struct suffixtree *root, char *word, unsigned int len)
{
	struct suffixtree *sf;

	forchildren(sf, root) {
		if ((unsigned int)find_common_substr(word+len, sf->suffix) == strlen(sf->suffix)) {
			return _suffixtree_find_word(sf, word, len + strlen(sf->suffix));
		}
	}

	return NULL;
}

int
suffixtree_scan(struct suffixtree *root)
{
	struct suffixtree *sf;
	int i = 0;

	// Children
	forchildren(sf, root) {
		i += suffixtree_scan(sf);
	}

	if (root->si) {
		printf(" %s\n", root->si->word);
	}

	return i + (root->si ? 1 : 0);
}

struct suggest_input *
_suffixtree_find_word(struct suffixtree *root, char *word, unsigned int len)
{
	if (len == strlen(word)) {
		return root->si;
	}
	else {
		return _suffixtree_find_word_children(root, word, len);
	}
}

struct suggest_input *
suffixtree_find_word(struct suffixtree *root, char *word)
{
	return _suffixtree_find_word(root, word, 0);
}

struct suggest_input **_suffixtree_find_suffix(struct suffixtree *, char *, unsigned int);

static struct suggest_input **
_suffixtree_find_suffix_children(struct suffixtree *root, char *word, unsigned int len)
{
	struct suffixtree *sf;

	forchildren(sf, root) {
		if ((unsigned int)find_common_substr(word+len, sf->suffix) == strlen(sf->suffix)) {
			printf("suffix: %s\n", sf->suffix);
			return _suffixtree_find_suffix(sf, word, len + strlen(sf->suffix));
		}
	}

	return NULL;
}

struct suggest_input **
_suffixtree_find_suffix(struct suffixtree *root, char *word, unsigned int len)
{
	if (len >= strlen(word)) {
		return root->best;
	}
	else {
		return _suffixtree_find_suffix_children(root, word, len);
	}
}

struct suggest_input **
suffixtree_find_suffix(struct suffixtree *root, char *word)
{
	return _suffixtree_find_suffix(root, word, 0);
}

