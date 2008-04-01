#define _GNU_SOURCE
#include <stdio.h>
#include <err.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../dictionarywordsLot/set.h"

const char alphabet[] = "abcdefghijklmnopqrstuvwxyz\xe5\xe6\xe8";

static unsigned int
wordhashfromkey(void *ky)
{
	char *p = ky;
	unsigned int hash = 5381;
	int c;

	while ((c = *p++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

static int
wordequalkeys(void *k1, void *k2)
{
	char *c1, *c2;

	c1 = k1;
	c2 = k2;

	return (strcmp(c1, c2) == 0);
}


struct spelling {
	struct hashtable *words;
};

struct hashtable *words;

void
train(const char *dict)
{
	FILE *fp;
	char *word;
	char *p;
	char *line;
	size_t len;
	unsigned int *frequency;


	if ((fp = fopen(dict, "r")) == NULL) {
		warn("fopen(dict)");
		return;
	}

	words = create_hashtable(5000, wordhashfromkey, wordequalkeys);

	line = NULL;
	while (getline(&line, &len, fp) > 0) {
		int i;

		p = line;
		p[strlen(p)-1] = '\0';
		while (!isspace(*p))
			p++;
		word = strndup(line, p - line);
		/* Only accept a-z for now */
		for (i = 0; word[i] != '\0'; i++) {
			if (!isalpha(word[i]) && word[i] != '\xe5' && word[i] != '\xe6' && word[i] != '\xe8') {
				free(word);
				goto word_end;
			}
		}

		p++; /* Get the frequency */

		frequency = hashtable_search(words, word);

		if (frequency == NULL) {
			frequency = malloc(sizeof(*frequency));
			*frequency = strtol(p, NULL, 10);

			if (!hashtable_insert(words, word, frequency)) {
				warn("hashtable_insert()");
				free(frequency);
				free(word);
			}
		} else {
			*frequency = *frequency + strtol(p, NULL, 10);
		}
 word_end:
		free(line);
		line = NULL;
	}

	//printf("Collected %d words\n", hashtable_count(words));

	fclose(fp);
}

void
untrain(void)
{
	struct hashtable *h;

	h = words;
	words = NULL;
	hashtable_destroy(h, 1);
}

set *
edits1(char *word, set *nwords, int remove_nonexistent, int makeset, char *best, int *max)
{
	int i, j;
	char nword[LINE_MAX];
	char *addword;
	int *freq;

#if 0
	return set([word[0:i]+word[i+1:] for i in range(n)] +                     # deletion
			[word[0:i]+word[i+1]+word[i]+word[i+2:] for i in range(n-1)] + # transposition
			[word[0:i]+c+word[i+1:] for i in range(n) for c in alphabet] + # alteration
			[word[0:i]+c+word[i:] for i in range(n+1) for c in alphabet])  # insertion


#endif

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		strncpy(nword, word, i);
		strcpy(nword+i, word+i+1);
		if ((freq = hashtable_search(words, nword)) == NULL && remove_nonexistent)
			continue;
		if (makeset) {
			addword = strdup(nword);
			set_add(nwords, addword);
		} else if (best != NULL) {
			if (*freq > *max) {
				strcpy(best, nword);
				*max = *freq;
			}
		}

	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		strncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		strcpy(nword+i+2, word+i+2);
		if ((freq = hashtable_search(words, nword)) == NULL && remove_nonexistent)
			continue;

		if (makeset) {
			addword = strdup(nword);
			set_add(nwords, addword);
		} else if (best != NULL) {
			if (*freq > *max) {
				strcpy(best, nword);
				*max = *freq;
			}
		}
	}

#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			strncpy(nword, word, i);
			nword[i] = alphabet[j];
			strcpy(nword+i+1, word+i+1);
			if ((freq = hashtable_search(words, nword)) == NULL && remove_nonexistent)
				continue;
			if (makeset) {
				addword = strdup(nword);
				set_add(nwords, addword);
			} else if (best != NULL) {
				if (*freq > *max) {
					strcpy(best, nword);
					*max = *freq;
				}

			}
		}
	}
#endif

#if 1
	/* insertions */
	for (j = 0; alphabet[j] != '\0'; j++) {
		int len = strlen(word);
		for (i = 0; i <= len; i++) {
			strncpy(nword, word, i);
			nword[i] = alphabet[j];
			strcpy(nword+i+1, word+i);
			if ((freq = hashtable_search(words, nword)) == NULL && remove_nonexistent)
				continue;
			if (makeset) {
				addword = strdup(nword);
				set_add(nwords, addword);
			} else if (best != NULL) {
				if (*freq > *max) {
					strcpy(best, nword);
					*max = *freq;
				}
			}
		}
	}
#endif

	return nwords;
}

char *
edits2(char *word)
{
	set e1set;
	set_iterator iter;
	char *w;
	char bestword[LINE_MAX];
	int max = 0;

	set_init(&e1set);

	bestword[0] = '\0';

	edits1(word, &e1set, 0, 1, NULL, NULL);
	SET_FOREACH(iter, &e1set, w) {
		edits1(w, NULL, 1, 0, bestword, &max);
	}

	set_free_all(&e1set);

	if (max == 0)
		return NULL;

	return strdup(bestword);
}


char *
maximize_from_set(set *cwords)
{
	set_iterator iter;
	char *w, *mword;
	int *freq;
	int max;


	SET_FOREACH(iter, cwords, w) {
		if ((freq = hashtable_search(words, w)) == NULL)
			continue;
		if (*freq > max) {
			max = *freq;
			mword = w;
		}
	}

	return strdup(mword);
}

int
correct_word(char *word)
{
	return (hashtable_search(words, word) != NULL);
}

void
check_word(char *word)
{
	char *best;

	if (words == NULL)
		return;

	if (correct_word(word))
		return;
	best = edits2(word);
	if (best != NULL) {
		printf("Found: %s\n", best);
		free(best);
	} else {
		puts("No better spelling found.");
	}
}

int
main(int argc, char **argv)
{
	set *nwords;
	set_iterator iter;
	char *w;
	char *best;

	nwords = malloc(sizeof(*nwords));
	set_init(nwords);

	train("mydict");

	check_word("niv\xe5\xe5");

	untrain();

	return 0;
}
