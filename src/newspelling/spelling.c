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
#include "../common/utf8-strings.h"
#include "../ds/dcontainer.h"
#include "../ds/dlist.h"
#include "../ds/dset.h"

#include "spelling.h"
#include "dmetaphone.h"
#include "levenshtein.h"

wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyz" L"0123456789" L"\xf8\xe5\xe6";
wchar_t alphabet_capital[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define MAX_EDIT_DISTANCE 2

struct wordelem {
	wchar_t *word, *soundslike;
	unsigned int frequency;
        char	*acl_allowed, *acl_denied, *collections;
};


typedef struct hashtable scache_t;

unsigned int spelling_min_freq = 0;


char* merge_lists(char *list1, char *list2, int logical_and)
{
    char	**Data;
    int		Count;
    container	*S = set_container( string_container() );
    container	*T = set_container( string_container() );

    if (split(list1, ",", &Data) != 0)
        for (Count=0; Data[Count] != NULL; Count++)
	    {
		set_insert(S, Data[Count]);
		free(Data[Count]);
	    }
    free(Data);

    if (split(list2, ",", &Data) != 0)
        for (Count=0; Data[Count] != NULL; Count++)
	    {
	        if (Data[Count][0] == '\0') continue;
		set_insert(T, Data[Count]);
		free(Data[Count]);
	    }
    free(Data);

    container *R;
    if (logical_and) R = ds_intersection( set_begin2(S), set_begin2(T) );
    else R = ds_union( set_begin2(S), set_begin2(T) );

    char	*result = asprint(R, ",");
    destroy(R);
    destroy(S);
    destroy(T);

    return result;
}


spelling_t *
train(const char *dict)
{
	FILE *fp;
	char *word;
	char *p;
	char *line;
	size_t len;
	spelling_t *s;
	struct hashtable *soundslikelookup;
	int num_words = 0, num_dup_words = 0;

	if ((fp = fopen(dict, "r")) == NULL) {
		warn("fopen(dict)");
		return NULL;
	}

	if ((s = malloc(sizeof(spelling_t))) == NULL) {
        	perror("malloc spelling_t");
		return NULL;
        }

	printf("train(s=%p, dict=%s)\n",s,dict);
	printf("Globals: spelling_min_freq %d\n", spelling_min_freq);

	s->words = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);
	if (s->words == NULL) {
		perror("create_hashtable s->words");
		return NULL;
	}

	s->soundslike = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);
	if (s->soundslike == NULL) {
		perror("create_hashtable s->soundslike");
		return NULL;
	}

	soundslikelookup = create_hashtable(5000, ht_wstringhash, ht_wstringcmp);
	if (soundslikelookup == NULL) {
		perror("create_hashtable soundslikelookup");
		return NULL;
	}

	

	line = NULL;
	while (getline(&line, &len, fp) > 0) {
		wchar_t *wcword;
		struct wordelem *we, *wel;
		int i;
		char	*token[5];

		p = line;
		for (i=0; i<5; i++) token[i] = NULL;
		for (i=0; i<5 && *p!='\0'; i++)
		    {
			token[i] = p;
			while (!isspace(*p) && *p!='\0') p++;
			*p = '\0';
			p++;
		    }

		if (i<5)
		    {
			//invalid line data
			goto word_end;
		    }

		word = strdup(token[0]);

		/* Get the frequency */
		if (strtol(token[1], NULL, 10) < spelling_min_freq) {
			free(word);
			goto word_end;
		}

		/* Convert from utf-8 to wchar_t */
		wcword = malloc((strlen(word)+1)*sizeof(wchar_t)); /* Check length */
		if (wcword == NULL) {
			free(word);
			goto word_end;
		}
		if (mbstowcs(wcword, word, strlen(word)+1) == -1) {
			free(word);
			free(wcword);
			#ifdef DEBUG
				warn("mbstowcs");
			#endif
			goto word_end;
		}
		free(word);

		for (i = 0; wcword[i] != '\0'; i++)
			wcword[i] = tolower(wcword[i]);
		utf8_strtolower((utf8_byte*)token[2]);
		utf8_strtolower((utf8_byte*)token[3]);
		utf8_strtolower((utf8_byte*)token[4]);

		wel = hashtable_search(s->words, wcword);
		if (wel == NULL) {
			container *list;
			we = malloc(sizeof(*we));
			we->frequency = strtol(token[1], NULL, 10);
			we->word = wcword;
			we->soundslike = dmetaphone(wcword);
			we->acl_allowed = strdup(token[2]);
			we->acl_denied = strdup(token[3]);
			we->collections = strdup(token[4]);

			if (!hashtable_insert(s->words, wcword, we)) {
				warn("hashtable_insert()");
			}

			/* And add the sounds like */
			list = hashtable_search(s->soundslike, we->soundslike);
			if (list == NULL) {
				wchar_t *sl = wcsdup(we->soundslike);
				list = list_container(ptr_container());
				if (!hashtable_insert(s->soundslike, we->soundslike, list))
					warn("hashtable_insert(soundslike)");
				if (!hashtable_insert(soundslikelookup, sl, we->soundslike))
					warn("hashtable_insert(soundslikelookup)");
			} else {
				wchar_t *sl = hashtable_search(soundslikelookup, we->soundslike);

				free(we->soundslike);
				we->soundslike = sl;
			}
			list_pushback(list, we);
			num_words++;
		} else {
			num_dup_words++;
			wel->frequency += strtol(token[1], NULL, 10);

			// Merge acls and collections:
			char	*m_allowed = merge_lists(wel->acl_allowed, token[2], 0);
			char 	*m_denied = merge_lists(wel->acl_denied, token[3], 1);
			char	*m_coll = merge_lists(wel->collections, token[4], 0);
			free(wel->acl_allowed);
			free(wel->acl_denied);
			free(wel->collections);
			wel->acl_allowed = m_allowed;
			wel->acl_denied = m_denied;
			wel->collections = m_coll;
		}
 word_end:

 		if ((num_words % 10000) == 0) {
			printf("Words: %d (dups: %d)\n", num_words, num_dup_words);
		}
		free(line);
		line = NULL;
	}
	free(line);

	//printf("Collected %d words\n", hashtable_count(s->words));

	fclose(fp);
	s->inited = 1;

	hashtable_destroy(soundslikelookup, 0);

	printf("~train\n");

	return s;
}

void
untrain(spelling_t *s)
{
	// @@TODO: Her mangler det mye
	struct hashtable_itr *itr;

	printf("untrain()\n");

	s->inited = 0;

	// frigjør s->soundslike, vil kalle egen free funksjon ved å iterere over alle.
	itr = hashtable_iterator(s->soundslike);
	do {
		container *list = hashtable_iterator_value(itr);

		destroy(list);
	} while (hashtable_iterator_advance(itr));
	free(itr);
	hashtable_destroy(s->soundslike, 0);
	s->soundslike = NULL;

	// firgjør s->words. Vil kalle free() på alle elementene
	hashtable_destroy(s->words, 1);
	s->words = NULL;

	printf("~untrain\n");


/*
	struct hashtable *h;
	struct hashtable_itr *itr;

	printf("untrain()\n");

	s->inited = 0;
	h = s->soundslike;
	s->soundslike = NULL;
	itr = hashtable_iterator(h);
	do {
		container *list = hashtable_iterator_value(itr);

		destroy(list);
	} while (hashtable_iterator_advance(itr));
	free(itr);
	hashtable_destroy(h, 0);
	h = s->words;
	s->words = NULL;
	hashtable_destroy(h, 1);

	printf("~untrain\n");
*/
}

void editsn(const spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int levels, container *groups, container *subnames);

static inline int
normalizefreq(unsigned int freq, int distance)
{
	double val;

	val = pow(distance, 2.4);
	val += log(freq);

	return (int)val;
}


void check_soundslike(const spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *like, wchar_t **bestw, int *mindist, int *maxfreq, int stage, container *groups, container *subnames);

static inline void
handle_word(const spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int phase, container *groups, container *subnames)
{
	wchar_t *like;

	like = dmetaphone(word);
	check_soundslike(s, c, wword, like, best, mindist, maxfreq, phase, groups, subnames);
	free(like);
}

void
editsn(const spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int stage, container *groups, container *subnames)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	/* deletions */
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		handle_word(s, c, wword, nword, best, mindist, maxfreq, stage, groups, subnames);
	}
#endif

#if 1
	/* transposition */
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		handle_word(s, c, wword, nword, best, mindist, maxfreq, stage, groups, subnames);
	}
#endif

#if 1
	/* alterations */
	for (j = 0; alphabet[j] != '\0'; j++) {
		for (i = 0; word[i] != '\0'; i++) {
			wcsncpy(nword, word, i);
			nword[i] = alphabet[j];
			wcscpy(nword+i+1, word+i+1);
			handle_word(s, c, wword, nword, best, mindist, maxfreq, stage, groups, subnames);
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
			handle_word(s, c, wword, nword, best, mindist, maxfreq, stage, groups, subnames);
		}
	}
#endif
}

static inline void
handle_soundslike(scache_t *c, int dist, int frequency, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int phase)
{
	if (dist < *mindist) {
		*mindist = dist;
		*best = word;
		*maxfreq = frequency;
	} else if (dist == *mindist && *maxfreq < frequency) {
		*maxfreq = frequency;
		*best = word;
	}
}

/*
void
editsn_soundslike(const spelling_t *s, scache_t *c, wchar_t *wword, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq, int stage)
{
	int i, j;
	wchar_t nword[LINE_MAX];

#if 1
	// deletions
	for (i = 0; word[i] != '\0'; i++) {
		wcsncpy(nword, word, i);
		wcscpy(nword+i, word+i+1);
		check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	// transposition
	for (i = 0; word[i+1] != '\0'; i++) {
		wcsncpy(nword, word, i);
		nword[i] = word[i+1];
		nword[i+1] = word[i];
		wcscpy(nword+i+2, word+i+2);
		check_soundslike(s, c, wword, nword, best, mindist, maxfreq, stage);
	}
#endif

#if 1
	// alterations
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
	// insertions
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
*/

int allowed_we(struct wordelem *we, container *groups, container *subnames)
{
    char **Data;
    int Count;

    printf("[spelling] ");
    int i;
    for (i=0; we->word[i]!=L'\0'; i++) printf("%c", (char)we->word[i]);
    printf(" ");

    if (set_size(subnames)>0 && split(we->collections, ",", &Data) != 0)
	{
	    int subname_allowed=0;

	    for (Count=0; Data[Count] != NULL; Count++)
		{
		    iterator	it = set_find(subnames, Data[Count]);
		    if (it.valid)
			{
			    subname_allowed = 1;
			    break;
			}

		    free(Data[Count]);
    		}

	    for (; Data[Count] != NULL; Count++) free(Data[Count]);
	    free(Data);

	    if (!subname_allowed)
		{
		    printf("(not in collection)\n");
		    return 0;
		}
	}


    if (split(we->acl_denied, ",", &Data) != 0)
	{
	    int has_been_denied=0;

	    for (Count=0; Data[Count] != NULL; Count++)
		{
		    iterator	it = set_find(groups, Data[Count]);
		    if (it.valid)
			{
			    printf("(denied)\n");
			    has_been_denied = 1;
			    break;
			}

		    free(Data[Count]);
    		}

	    for (; Data[Count] != NULL; Count++) free(Data[Count]);
	    free(Data);

	    if (has_been_denied) return 0;
	}


    /**
     *  Ax: public-collections vil per i dag ikke bli 'allowed' pga acl
     */
    if (split(we->acl_allowed, ",", &Data) != 0)
	{
	    int has_been_allowed = 0;

	    for (Count=0; Data[Count] != NULL; Count++)
		{
		    iterator	it = set_find(groups, Data[Count]);
		    if (it.valid)
			{
			    printf("(allowed)\n");
			    has_been_allowed = 1;
			    break;
			}

		    free(Data[Count]);
    		}

	    for (; Data[Count] != NULL; Count++) free(Data[Count]);
	    free(Data);

	    if (has_been_allowed) return 1;
	}

    printf("(not in acl_allowed)\n");
    return 0;
}


int
correct_word(const spelling_t *s, char *word, container *groups, container *subnames)
{
	wchar_t *wword;
	int i;

	wword = malloc((strlen(word)+1)*sizeof(wchar_t));
	if (wword == NULL)
		return 1;
	mbstowcs(wword, word, strlen(word)+1);

	for (i = 0; wword[i] != '\0'; i++)
		wword[i] = tolower(wword[i]);

	if (s->words == NULL) {
		free(wword);
		return 1;
	}

	struct wordelem *we = hashtable_search(s->words, wword);

	free(wword);

	if (we == NULL || !allowed_we(we, groups, subnames))
		return 0;
	return 1;
}

void
check_soundslike(const spelling_t *s, scache_t *c,  wchar_t *wword, wchar_t *like, wchar_t **bestw, int *mindist, int *maxfreq, int phase, container *groups, container *subnames)
{
	container *list;
	//struct hashtable_itr *itr;
	iterator itr;

	/* Already seen this */
	if (hashtable_search(c, like))
		return;

#if 0
	printf("hashtable count: %d\n", hashtable_count(c));
	hitr = hashtable_iterator(c);
	if (hashtable_count(c) > 0) {
		do {
			wchar_t *a = hashtable_iterator_key(hitr);
			printf("Word: %ls\n", a);
		} while (hashtable_iterator_advance(hitr));
	}
#endif

	list = hashtable_search(s->soundslike, like);
	if (list == NULL) {
		return;
	}

	//printf("Words matching that(%ls):\n", like);
	for (itr = list_begin(list); itr.valid; itr = list_next(itr)) {
		struct wordelem *we;
		int dist;

		we = list_val(itr).ptr;
		if (!allowed_we(we, groups, subnames)) continue;

		dist = levenshteindistance(wword, we->word);
		handle_soundslike(c, dist, we->frequency, we->word, bestw, mindist, maxfreq, phase);

		//printf("\t%ls (%d): %d\n", we->word, we->frequency, dist);
	}
	hashtable_insert(c, wcsdup(like), (void*)1);
}

char *
check_word(const spelling_t *s, char *word, int *found, container *groups, container *subnames)
{
	wchar_t *bestw;
	int mindist = INT_MAX;
	int maxfreq = 0;
	wchar_t *wword;
	wchar_t *like;
	scache_t *cache;
	char u8word[LINE_MAX];
	int i;

	*found = 0;
	if (!s->inited)
		return NULL;

	wword = malloc((strlen(word)+1)*sizeof(wchar_t));
	if (wword == NULL)
		return NULL;
	mbstowcs(wword, word, strlen(word)+1);

	for (i = 0; wword[i] != '\0'; i++)
		wword[i] = tolower(wword[i]);

	if (s->words == NULL) {
		free(wword);
		return NULL;
	}

	cache = create_hashtable(7, ht_wstringhash, ht_wstringcmp);
	if (cache == NULL) {
		free(wword);
		return NULL;
	}

	// Phase 1, sounds like words
	like = dmetaphone(wword);
	bestw = NULL;
	//printf("We sound like: %ls\n", like);
	check_soundslike(s, cache, wword, like, &bestw, &mindist, &maxfreq, 0, groups, subnames);
	free(like);

	// Phase 2, edit distance on foo
	//editsn_soundslike(s, wword, like, &bestw, &mindist, &maxfreq, 1);
	editsn(s, cache, wword, wword, &bestw, &mindist, &maxfreq, 1, groups, subnames);

	hashtable_destroy(cache, 0);

	free(wword);
	if (bestw != NULL) {
		*found = 1;
		wcstombs(u8word, bestw, LINE_MAX);

		if (strcasecmp(u8word, word) == 0)
			return NULL;
		return strdup(u8word);
	} else {
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

	train(&s, "/home/eirik/wordlist-100freq");
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
		char *p;

		p = check_word(&s, argv[i], &found);
		if (p != NULL) {
			printf("%s\n", p);
			//printf("Corrected '%s' to '%s'\n", argv[i], p);
			free(p);
		} else {
			printf("-----------\n");
			//printf("No better word found\n");
		}
	}

	untrain(&s);

	return 0;
}
#endif
