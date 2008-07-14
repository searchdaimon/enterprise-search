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
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../common/ht.h"
#include "../ds/dcontainer.h"
#include "../ds/dlist.h"

#include "spelling.h"
#include "dmetaphone.h"
#include "levenshtein.h"

wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyz" L"0123456789" L"\xf8\xe5\xe6";
wchar_t alphabet_capital[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define MAX_EDIT_DISTANCE 2

struct wordelem {
	wchar_t *word, *soundslike;
	unsigned int frequency;
};

typedef struct hashtable scache_t;


int
train(spelling_t *s, const char *dict)
{
	FILE *fp;
	char *word;
	char *p;
	char *line;
	size_t len;

	if ((fp = fopen(dict, "r")) == NULL) {
		warn("fopen(dict)");
		return 0;
	}

	s->words = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);
	s->soundslike = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);

	line = NULL;
	while (getline(&line, &len, fp) > 0) {
		wchar_t *wcword;
		struct wordelem *we, *wel;

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

		wel = hashtable_search(s->words, wcword);
		if (wel == NULL) {
			container *list;
			we = malloc(sizeof(*we));
			we->frequency = strtol(p, NULL, 10);
			we->word = wcword;
			we->soundslike = dmetaphone(wcword);
			//printf("Sounds like: %s\n", we->soundslike);

#if 0
			for (i = 0; we->soundslike[i] != 0; i++)
				we->soundslike[i] = _tolower(we->soundslike[i]);
#endif

			if (!hashtable_insert(s->words, wcword, we)) {
				warn("hashtable_insert()");
			}

			/* And add the sounds like */
			list = hashtable_search(s->soundslike, we->soundslike);
			if (list == NULL) {
				list = list_container(ptr_container());
				if (!hashtable_insert(s->soundslike, we->soundslike, list))
					warn("hashtable_insert(soundslike)");
			}
			list_pushback(list, we);
		} else {
			wel->frequency += strtol(p, NULL, 10);
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

void editsn(spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int levels);

static inline int
normalizefreq(unsigned int freq, int distance)
{
	double val;

	val = pow(distance, 2.4);
	val += log(freq);

	return (int)val;
}


void check_soundslike(spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *like, wchar_t **bestw, int *mindist, int *maxfreq, int stage);

static inline void
//handle_word(spelling_t *s, wchar_t *word, wchar_t *best, int *max, int levels)
handle_word(spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int phase)
{
	int *freq;
	wchar_t *like;

	like = dmetaphone(word);
	check_soundslike(s, c, wword, like, best, mindist, maxfreq, phase);
	free(like);

#if 0
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
#if 0
	if (levels > 1) {
		editsn(s, word, best, max, levels-1);
		return;
	}
#endif
#endif
}

void
editsn(spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int stage)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		handle_word(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		handle_word(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet[j];
			wcscpy(nword+i+1, word+i+1);
			handle_word(s, c, wword, nword, best, mindist, maxfreq, stage);
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
			handle_word(s, c, wword, nword, best, mindist, maxfreq, stage);
		}
	}
#endif
}


static inline void
handle_soundslike(scache_t *c, int dist, int frequency, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int phase)
{
	if (dist < *mindist) {
		//printf("Hmpf %ls\n", we->word);
		*mindist = dist;
		*best = word;
		*maxfreq = frequency;
	} else if (dist == *mindist && *maxfreq < frequency) {
		//printf("Hmpf2 %ls\n", we->word);
//		if (dist >= 3) {
//			if (*best)
//				return;
//		}
		*maxfreq = frequency;
		*best = word;
	}

	hashtable_insert(c, wcsdup(word), (void*)1);

#if 0
	if (levels > 1) {
		editsn(s, word, best, max, levels-1);
		return;
	}
#endif
}


void
editsn_soundslike(spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int stage)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	/* alterations */
	for (j = 0; alphabet_capital[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet_capital[j];
			wcscpy(nword+i+1, word+i+1);
			check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
		}
	}
#endif

#if 1
	/* insertions */
	for (j = 0; alphabet_capital[j] != '\0'; j++) {
		int len = wcslen(word);
		for (i = 0; i <= len; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet_capital[j];
			wcscpy(nword+i+1, word+i);
			check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
		}
	}
#endif
}

int
correct_word(spelling_t *s, wchar_t *word)
{
	return (hashtable_search(s->words, word) != NULL);
}

void
check_soundslike(spelling_t *s, scache_t *c,  wchar_t *wword, wchar_t *like, wchar_t **bestw, int *mindist, int *maxfreq, int phase)
{
	container *list;
	//struct hashtable_itr *itr;
	iterator itr;


	list = hashtable_search(s->soundslike, like);
	if (list == NULL) {
		return;
	}

	//printf("Words matching that(%ls):\n", like);
	for (itr = list_begin(list); itr.valid; itr = list_next(itr)) {
		struct wordelem *we;
		int dist;

		we = list_val(itr).ptr;
		dist = levenshteindistance(wword, we->word);
		handle_soundslike(c, dist, we->frequency, we->word, bestw, mindist, maxfreq, phase);

		//handle_soundslike(s, wword, word, best, mindist, maxfreq, levels);

		//printf("\t%ls (%d): %d\n", we->word, we->frequency, dist);
	}
	hashtable_insert(c, wcsdup(like), (void*)1);
}

char *
check_word(spelling_t *s, char *word, int *found)
{
	wchar_t *bestw;
	int mindist = INT_MAX;
	int maxfreq = 0;
	wchar_t *wword;
	wchar_t *like;
	scache_t *cache;

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

	cache = create_hashtable(7, ht_wstringhash, ht_wstringcmp);
	if (cache == NULL) {
		free(wword);
		return NULL;
	}

#if 0
	if (correct_word(s, wword)) {
		printf("Good as it gets\n");
		//free(wword);
		//return NULL;
	}
#endif

	// Phase 1, sounds like words
	like = dmetaphone(wword);
	bestw = NULL;
	//printf("We sound like: %ls\n", like);
	check_soundslike(s, cache, wword, like, &bestw, &mindist, &maxfreq, 0);
	free(like);

	// Phase 2, edit distance on foo
	//editsn_soundslike(s, wword, like, &bestw, &mindist, &maxfreq, 1);
	editsn(s, cache, wword, wword, &bestw, &mindist, &maxfreq, 1);

	hashtable_destroy(cache, 0);

	if (bestw != NULL) {
		//printf("Phase 1: Best match: %ls, freq: %d\n", bestw, maxfreq);
		return bestw;
	}

	return NULL;
}

#if 0

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

#endif

#ifdef TESTMAIN
int
main(int argc, char **argv)
{
	int found;
	int i;
	spelling_t s;
	//time_t start, end;

	setlocale(LC_ALL, "en_US.UTF-8");

	train(&s, "/home/eirik/wordlist-100freq.2");
	//train(&s, "/home/eirik/Boitho/boitho/websearch/var/dictionarywords");

#if 0

	struct hashtable_itr *itr;
	itr = hashtable_iterator(s.soundslike);

	do {
		wchar_t *key = hashtable_iterator_key(itr);

		printf("Sounds like: %ls\n", key);
	} while (hashtable_iterator_advance(itr));

#endif

	for (i = 1; i < argc; i++) {
		wchar_t *p;

		p = check_word(&s, argv[i], &found);
		if (p != NULL) {
			printf("%ls\n", p);
			//printf("Corrected '%s' to '%s'\n", argv[i], p);
		} else {
			printf("-----------\n");
			//printf("No better word found\n");
		}
	}

	untrain(&s);

	return 0;
}
#endif
