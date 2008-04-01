#define _GNU_SOURCE
#include <stdio.h>
#include <err.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "../3pLibs/keyValueHash/hashtable.h"

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

void
editsn(char *word, char *best, int *max, int levels)
{
	int i, j;
	char nword[LINE_MAX];
	int *freq;

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		strncpy(nword, word, i);
		strcpy(nword+i, word+i+1);
		if ((freq = hashtable_search(words, nword)) == NULL && levels == 1)
			continue;
		if (levels > 1) {
			editsn(nword, best, max, levels-1);
		} else if (*freq > *max) {
			strcpy(best, nword);
			*max = *freq;
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
		if ((freq = hashtable_search(words, nword)) == NULL && levels == 1)
			continue;

		if (levels > 1) {
			editsn(nword, best, max, levels-1);
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
			if ((freq = hashtable_search(words, nword)) == NULL && levels == 1)
				continue;
			if (levels > 1) {
				editsn(nword, best, max, levels-1);
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
			if ((freq = hashtable_search(words, nword)) == NULL && levels == 1)
				continue;
			if (levels > 1) {
				editsn(nword, best, max, levels-1);
			} else if (best != NULL) {
				if (*freq > *max) {
					strcpy(best, nword);
					*max = *freq;
				}
			}
		}
	}
#endif
}

int
correct_word(char *word)
{
	return (hashtable_search(words, word) != NULL);
}

void
check_word(char *word)
{
	char best[LINE_MAX];
	int max = 0;

	if (words == NULL)
		return;

	if (correct_word(word))
		return;
	editsn(word, best, &max, 2);
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
	char *w;
	char *best;

	train("mydict");

	check_word("niv\xe5\xe5");

	untrain();

	return 0;
}
