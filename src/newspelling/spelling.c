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

#include "spelling.h"

wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyz" L"0123456789" L"\xf8\xe5\xe6";

#define MAX_EDIT_DISTANCE 2

int
train(spelling_t *s, const char *dict)
{
	FILE *fp;
	char *word;
	char *p;
	char *line;
	size_t len;
	unsigned int *frequency;

	if ((fp = fopen(dict, "r")) == NULL) {
		warn("fopen(dict)");
		return 0;
	}

	s->words = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);

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

		frequency = hashtable_search(s->words, wcword);

		if (frequency == NULL) {
			frequency = malloc(sizeof(*frequency));
			*frequency = strtol(p, NULL, 10);

			if (!hashtable_insert(s->words, wcword, frequency)) {
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

	//printf("Collected %d words\n", hashtable_count(s->words));

	fclose(fp);
	s->inited = 1;

	return 1;
}

void
untrain(spelling_t *s)
{
	struct hashtable *h;

	s->inited = 0;
	h = s->words;
	s->words = NULL;
	hashtable_destroy(h, 1);
}

void editsn(spelling_t *, wchar_t *, wchar_t *, int *, int);

static inline int
normalizefreq(unsigned int freq, int distance)
{
	double val;

	val = pow(distance, 2.4);
	val += log(freq);

	return (int)val;
}

static inline void
handle_word(spelling_t *s, wchar_t *word, wchar_t *best, int *max, int levels)
{
	int *freq;

	if ((freq = hashtable_search(s->words, word)) == NULL && levels == 1)
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
		editsn(s, word, best, max, levels-1);
		return;
	}
}

void
editsn(spelling_t *s, wchar_t *word, wchar_t *best, int *max, int levels)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		handle_word(s, nword, best, max, levels);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		handle_word(s, nword, best, max, levels);
	}
#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet[j];
			wcscpy(nword+i+1, word+i+1);
			handle_word(s, nword, best, max, levels);
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
			handle_word(s, nword, best, max, levels);
		}
	}
#endif
}

int
correct_word(spelling_t *s, wchar_t *word)
{
	return (hashtable_search(s->words, word) != NULL);
}

char *
check_word(spelling_t *s, char *word, int *found)
{
	wchar_t best[LINE_MAX];
	char u8best[LINE_MAX];
	int max = 0;
	wchar_t *wword;

	*found = 0;
	if (!s->inited)
		return NULL;

	wword = malloc((strlen(word)+1)*sizeof(wchar_t));
	if (wword == NULL)
		return NULL;
	mbstowcs(wword, word, strlen(word)+1);

	if (s->words == NULL) {
		free(wword);
		return NULL;
	}

	if (correct_word(s, wword)) {
		free(wword);
		return NULL;
	}
	editsn(s, wword, best, &max, MAX_EDIT_DISTANCE);
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
	spelling_t s;
	//time_t start, end;

	setlocale(LC_ALL, "en_US.UTF-8");

	train(&s, "/home/eirik/Boitho/boitho/websearch/var/dictionarywords");

	for (i = 1; i < argc; i++) {
		char *p;

		p = check_word(&s, argv[i], &found);
		if (p != NULL) {
			printf("Corrected '%s' to '%s'\n", argv[i], p);
			free(p);
		} else {
			printf("No better word found\n");
		}
	}

	untrain(&s);

	return 0;
}
#endif
