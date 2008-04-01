#define _GNU_SOURCE
#include <stdio.h>
#include <err.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

void editsn(char *, char *, int *, int);

static inline void
handle_word(char *word, char *best, int *max, int levels)
{
	int *freq;

	if ((freq = hashtable_search(words, word)) == NULL && levels == 1)
		return;
	if (levels > 1) {
		editsn(word, best, max, levels-1);
	} else if (*freq > *max) {
		strcpy(best, word);
		*max = *freq;
	}
}

void
editsn(char *word, char *best, int *max, int levels)
{
	int i, j;
	char nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		strncpy(nword, word, i);
		strcpy(nword+i, word+i+1);
		handle_word(nword, best, max, levels);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		strncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		strcpy(nword+i+2, word+i+2);
		handle_word(nword, best, max, levels);
	}

#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			strncpy(nword, word, i);
			nword[i] = alphabet[j];
			strcpy(nword+i+1, word+i+1);
			handle_word(nword, best, max, levels);
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
			handle_word(nword, best, max, levels);
		}
	}
#endif
}

int
correct_word(char *word)
{
	return (hashtable_search(words, word) != NULL);
}

char *
check_word(char *word, int *found)
{
	char best[LINE_MAX];
	int max = 0;

	if (words == NULL)
		return NULL;

	*found = 0;
	if (correct_word(word))
		return NULL;
	editsn(word, best, &max, 2);
	if (max > 0) {
		*found = 1;
		printf("Found: %s\n", best);
		return strdup(best);
	} else {
		puts("No better spelling found.");
		return NULL;
	}
}

int
main(int argc, char **argv)
{
	int found;
	int i;
	time_t start, end;

	train("mydict");

	start = time(NULL);
	for (i = 0; i < 100; i++) {
		free(check_word("eppennn", &found));
	}
	end = time(NULL);

	printf("Checking took: %ld\n", end - start);

	untrain();

	return 0;
}
