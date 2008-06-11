#define _GNU_SOURCE
#include <stdio.h>
#include <err.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../common/ht.h"

wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyz" L"0123456789" L"\xf8\xe5\xe6";

#define MAX_EDIT_DISTANCE 2


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

	words = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);

	line = NULL;
	while (getline(&line, &len, fp) > 0) {
		wchar_t *wcword;

		p = line;
		p[strlen(p)-1] = '\0';
		while (!isspace(*p))
			p++;
		word = strndup(line, p - line);

		p++; /* Get the frequency */

		/* Convert from utf-8 to wchar_t */
		wcword = malloc((strlen(word)+1)*sizeof(wchar_t)); /* Check length */
		if (wcword == NULL) {
			free(word);
			goto word_end;
		}
		if (mbstowcs(wcword, word, strlen(word)+1) == -1) {
			free(word);
			free(wcword);
			warn("mbstowcs");
			goto word_end;
		}
		free(word);

		frequency = hashtable_search(words, wcword);

		if (frequency == NULL) {
			frequency = malloc(sizeof(*frequency));
			*frequency = strtol(p, NULL, 10);

			if (!hashtable_insert(words, wcword, frequency)) {
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

void editsn(wchar_t *, wchar_t *, int *, int);

static inline int
normalizefreq(unsigned int freq, int distance)
{
	double val;

	val = pow(distance, 2.4);
	val += log(freq);

	return (int)val;
}

static inline void
handle_word(wchar_t *word, wchar_t *best, int *max, int levels)
{
	int *freq;

	if ((freq = hashtable_search(words, word)) == NULL && levels == 1)
		return;
#if 1
	if (freq && (normalizefreq(*freq, levels) > *max)) {
		wcscpy(best, word);
		*max = normalizefreq(*freq, levels);
	}
#else
	if (freq && *freq > *max) {
		wcscpy(best, word);
		*max = *freq;
	}
#endif
	if (levels > 1) {
		editsn(word, best, max, levels-1);
		return;
	}
}

void
editsn(wchar_t *word, wchar_t *best, int *max, int levels)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		handle_word(nword, best, max, levels);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		handle_word(nword, best, max, levels);
	}
#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet[j];
			wcscpy(nword+i+1, word+i+1);
			handle_word(nword, best, max, levels);
		}
	}
#endif

#if 1
	/* insertions */
	for (j = 0; alphabet[j] != '\0'; j++) {
		int len = wcslen(word);
		for (i = 0; i <= len; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet[j];
			wcscpy(nword+i+1, word+i);
			handle_word(nword, best, max, levels);
		}
	}
#endif
}

int
correct_word(wchar_t *word)
{
	return (hashtable_search(words, word) != NULL);
}

char *
check_word(char *word, int *found)
{
	wchar_t best[LINE_MAX];
	char u8best[LINE_MAX];
	int max = 0;
	wchar_t *wword;

	wword = malloc((strlen(word)+1)*sizeof(wchar_t));
	if (wword == NULL)
		return NULL;
	mbstowcs(wword, word, strlen(word)+1);

	if (words == NULL) {
		free(wword);
		return NULL;
	}

	*found = 0;
	if (correct_word(wword)) {
		free(wword);
		return NULL;
	}
	editsn(wword, best, &max, MAX_EDIT_DISTANCE);
	if (max > 0) {
		*found = 1;
		free(wword);
		wcstombs(u8best, best, LINE_MAX);
		return strdup(u8best);
	} else {
		free(wword);
		return NULL;
	}
}

#ifdef TESTMAIN
int
main(int argc, char **argv)
{
	int found;
	int i;
	//time_t start, end;

	setlocale(LC_ALL, "en_US.UTF-8");

	train("mydict.utf8");

	for (i = 1; i < argc; i++) {
		free(check_word(argv[i], &found));
	}

	untrain();

	return 0;
}
#endif
