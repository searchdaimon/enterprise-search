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
#include "../common/bstr.h"
#include "../ds/dcontainer.h"
#include "../ds/dlist.h"
#include "../ds/dset.h"
#include "../ds/dmap.h"
#include "../ds/dvector.h"
#include "../logger/logger.h"

#include "spelling.h"
#include "dmetaphone.h"
#include "levenshtein.h"

wchar_t alphabet[] = L"abcdefghijklmnopqrstuvwxyz" L"0123456789" L"\xf8\xe5\xe6";
wchar_t alphabet_capital[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

#define MAX_EDIT_DISTANCE 2

struct wordelem {
	char	*word, *soundslike;
	unsigned int frequency;
	int	*acl_allowed, *acl_denied, *collections;
	unsigned short	acl_allowed_len, acl_denied_len, collections_len;
};


typedef struct hashtable scache_t;

unsigned int spelling_min_freq = 0;



wchar_t* strtowcsdup(char *str)
{
    wchar_t     *w = malloc((strlen(str)+1)*sizeof(wchar_t));

    if (mbstowcs(w, str, strlen(str)+1) == (size_t) -1)
        {
            // Has probably encountered utf8-characters
            int         slen = strlen(str);
            int         len, wpos;

            for (len=0, wpos=0; len<slen;)
                {
                    int         n;

                    if ((n=valid_utf8_byte((utf8_byte*)&str[len], slen-len)) != 0)
                        {
                            int U = convert_utf8_U((utf8_byte*)&str[len]);

                            // UCS2: skip values above U+FFFF
                            if (U < 65536) w[wpos++] = (wchar_t)U;

                            len+= n;
                        }
                    else len++; // skip
                }

            w[wpos] = L'\0';
            wchar_t     *w2 = wcsdup(w);
            free(w);
            return w2;
        }

    return w;
}


char* wcstostrdup(wchar_t *w)
{
    size_t	i, len=0;
    char	*tempstr = malloc((wcslen(w)+1)*sizeof(char)*4);
	
    if (tempstr == NULL) {
        perror("wcstostrdup malloc:");
        return NULL;
    }

    for (i=0; i<wcslen(w); i++)
	{
	    len+= convert_U_utf8((utf8_byte*)&tempstr[len], (int)w[i]);
	}

    tempstr[len] = '\0';

    char	*str = strdup(tempstr);
    if (str == NULL) {
        perror("wcstostrdup strdup:");
        return NULL;
    }

    free(tempstr);

    return str;
}

int* create_intarray_from_list(struct hashtable *H, container *V, char *L, unsigned short *out_len)
{
    char	**Data;
    int		Count = 0;
    int		len = 0;
    int		*ret = NULL;

    if (split(L, ",", &Data) != 0)
        {
            for (Count=0; Data[Count] != NULL && Count<65535; Count++) len++;
	    ret = malloc(len * sizeof(int));
            if (ret == NULL) {
                perror("create_intarray_from_list malloc ret: ");
            }

            for (Count=0; Data[Count] != NULL && Count<65535; Count++)
    		{
		    int* value = hashtable_search(H, Data[Count]);

		    if (value == NULL)
			{
			    int	j, *k;
			    vector_pushback(V, Data[Count]);
			    j = vector_size(V)-1;
			    ret[Count] = j;

			    k = malloc(sizeof(int));
			    *k = j;
			    hashtable_insert(H, vector_get(V,j).ptr, k);
			}
		    else
			{
			    ret[Count] = *value;
			}

		    free(Data[Count]);
		}

            free(Data);
        }

    *out_len = len;
    return ret;
}

int* merge_and_free_intarrays(int perform_union, int *dest, unsigned short *dest_len, int *src, unsigned short *src_len)
{
    int		i, j, len, count;
    int		*ret = NULL;

    len = 0;

    // Calculate len:
    if (perform_union)
	{
	    len = *src_len;

	    for (i=0; i<*dest_len && len<65535; i++)
		{
		    int	dup = 0;

		    for (j=0; j<*src_len && !dup; j++)
			if (dest[i] == src[j])
			    dup = 1;

		    if (!dup) len++;
		}
	}
    else // perform intersection
	{
	    for (i=0; i<*dest_len && len<65535; i++)
		{
		    int	dup = 0;

		    for (j=0; j<*src_len && !dup; j++)
			if (dest[i] == src[j])
			    dup = 1;

		    if (dup) len++;
		}
	}

    ret = malloc(len * sizeof(int));
    count = 0;

    if (perform_union)
	{
	    len = *src_len;
	    for (j=0; j<*src_len; j++) ret[count++] = src[j];

	    for (i=0; i<*dest_len && count<65535; i++)
		{
		    int	dup = 0;

		    for (j=0; j<*src_len && !dup; j++)
			if (dest[i] == src[j])
			    dup = 1;

		    if (!dup) ret[count++] = dest[i];
		}
	}
    else // perform intersection
	{
	    for (i=0; i<*dest_len && count<65535; i++)
		{
		    int	dup = 0;

		    for (j=0; j<*src_len && !dup; j++)
			if (dest[i] == src[j])
			    dup = 1;

		    if (dup) ret[count++] = dest[i];
		}
	}


    free(dest);
    free(src);

    *dest_len = len;
    return ret;
}


void _hashtable_destroy_values(struct hashtable *h)
{
    unsigned int i;
    struct entry *e, *f;
    struct entry **table = h->table;

    for (i = 0; i < h->tablelength; i++)
        {
            e = table[i];
            while (NULL != e)
            { f = e; e = e->next; free(f->v); free(f); }
        }

    free(h->table);
    free(h);
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
	struct hashtable *aclH, *subnameH;

	int _we=0, _word=0, _soundslike=0, _ints=0, _vector=0, _vn=0, _hash=0;

	if ((fp = fopen(dict, "r")) == NULL) {
		warn("train: Can't open dictionary. fopen(dict)");
		return NULL;
	}

	if ((s = malloc(sizeof(spelling_t))) == NULL) {
        	perror("malloc spelling_t");
		return NULL;
        }

	printf("train(s=%p, dict=%s)\n",s,dict);
	printf("Globals: spelling_min_freq %d\n", spelling_min_freq);

	s->words = create_hashtable(5000, ht_stringhash, ht_stringcmp);
	if (s->words == NULL) {
		perror("create_hashtable s->words");
		return NULL;
	}

	s->soundslike = create_hashtable(5000, ht_stringhash, ht_stringcmp);
	if (s->soundslike == NULL) {
		perror("create_hashtable s->soundslike");
		return NULL;
	}

	soundslikelookup = create_hashtable(5000, ht_stringhash, ht_stringcmp);
	if (soundslikelookup == NULL) {
		perror("create_hashtable soundslikelookup");
		return NULL;
	}

	aclH = create_hashtable(50, ht_stringhash, ht_stringcmp);
	if (aclH == NULL) {
		perror("create_hashtable aclH");
		return NULL;
	}

	subnameH = create_hashtable(50, ht_stringhash, ht_stringcmp);
	if (subnameH == NULL) {
		perror("create_hashtable subnameH");
		return NULL;
	}

	s->_aclV = vector_container( string_container() );
	s->_subnameV = vector_container( string_container() );
	s->_vp_tmpl = vector_empty_container( ptr_container() );


	line = NULL;
	int last_printed = -1, last_printed_dup = -1;
	while (getline(&line, &len, fp) > 0) {
		//wchar_t *wcword;
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

		/* Get the frequency */
		if (strtol(token[1], NULL, 10) < spelling_min_freq) {
			goto word_end;
		}

		word = strdup(token[0]);
		_word+= (strlen(word)+1)*sizeof(char);

		//for (i = 0; wcword[i] != '\0'; i++)
		//	wcword[i] = tolower(wcword[i]);
		utf8_strtolower((utf8_byte*)word);
		//utf8_strtolower((utf8_byte*)token[2]); // acl (groups)
		utf8_strtolower((utf8_byte*)token[3]);
		utf8_strtolower((utf8_byte*)token[4]);

		wel = hashtable_search(s->words, word);
		if (wel == NULL) {
			container *list;
			we = malloc(sizeof(*we));
			if (we == NULL) {
                            perror("Malloc we:");
                            return NULL;
                        }
			_we+= sizeof(*we);
			we->frequency = strtol(token[1], NULL, 10);
			we->word = word;

			{
			wchar_t *wcword = strtowcsdup(word);
			wchar_t *wmetaph = dmetaphone(wcword);
			we->soundslike = wcstostrdup(wmetaph);
			free(wmetaph);
			free(wcword);
			}

			_soundslike+= (strlen(we->soundslike)+1)*sizeof(char);

			we->acl_allowed = create_intarray_from_list(aclH, s->_aclV, token[2], &we->acl_allowed_len);
			we->acl_denied = create_intarray_from_list(aclH, s->_aclV, token[3], &we->acl_denied_len);
			we->collections = create_intarray_from_list(subnameH, s->_subnameV, token[4], &we->collections_len);
			_ints+= (we->acl_allowed_len + we->acl_denied_len + we->collections_len)*sizeof(int);

			if (!hashtable_insert(s->words, word, we)) {
				warn("hashtable_insert()");
				return NULL;
			}
			_hash++;

			/* And add the sounds like */
			list = hashtable_search(s->soundslike, we->soundslike);
			if (list == NULL) {
				char *sl = strdup(we->soundslike);
				list = vector_new_data();
				_vector+= 16;
				if (!hashtable_insert(s->soundslike, we->soundslike, list)) {
					warn("hashtable_insert(soundslike)");
					return NULL;
				}
				if (!hashtable_insert(soundslikelookup, sl, we->soundslike)) {
					warn("hashtable_insert(soundslikelookup)");
					return NULL;
				}
				_hash+= 2;
			} else {
				char *sl = hashtable_search(soundslikelookup, we->soundslike);

				free(we->soundslike);
				we->soundslike = sl;
			}
			vector_attach_data(s->_vp_tmpl, list);
			vector_pushback(s->_vp_tmpl, we);	// list(tmpl:s->_vp_tmpl) <- we
			_vn++;
			num_words++;
		} else {
			_word-= (strlen(word)+1)*sizeof(char);
			free(word);
			num_dup_words++;
			wel->frequency += strtol(token[1], NULL, 10);

			// Merge acls and collections:
			unsigned short len;
			wel->acl_allowed = merge_and_free_intarrays(1, wel->acl_allowed, &wel->acl_allowed_len, create_intarray_from_list(aclH, s->_aclV, token[2], &len), &len);
			wel->acl_denied = merge_and_free_intarrays(0, wel->acl_denied, &wel->acl_denied_len, create_intarray_from_list(aclH, s->_aclV, token[3], &len), &len);
			wel->collections = merge_and_free_intarrays(1, wel->collections, &wel->collections_len, create_intarray_from_list(subnameH, s->_subnameV, token[4], &len), &len);
		}
 word_end:

 		if (((num_words % 10000) == 0 && num_words != last_printed)
		    || ((num_dup_words % 10000) == 0 && num_dup_words != last_printed_dup)) {
			printf("Words: %d (dups: %d)\t\t(we:%i word:%i soundslike:%i ints:%i hash:%i vector:%i+%zu aclV:%i subV:%i)\n",
			    num_words, num_dup_words,
			    _we, _word, _soundslike, _ints, _hash, _vector, _vn*sizeof(void*),
			    vector_size(s->_aclV), vector_size(s->_subnameV));

			last_printed = num_words;
			last_printed_dup = num_dup_words;
		}
		free(line);
		line = NULL;
	}
	free(line);

	bblog(INFO,"Collected %d words\n", hashtable_count(s->words));

	fclose(fp);
	s->inited = 1;

	hashtable_destroy(soundslikelookup, 0);
	_hashtable_destroy_values(subnameH);
	_hashtable_destroy_values(aclH);

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
		vector_new_data_free(list);
	} while (hashtable_iterator_advance(itr));
	free(itr);

	hashtable_destroy(s->soundslike, 0);
	s->soundslike = NULL;

	// firgjør s->words. Vil kalle free() på alle elementene
	itr = hashtable_iterator(s->words);
	do {
		struct wordelem *we = hashtable_iterator_value(itr);

		free(we->acl_allowed);
		free(we->acl_denied);
		free(we->collections);

	} while (hashtable_iterator_advance(itr));
	free(itr);

	hashtable_destroy(s->words, 1);
	s->words = NULL;

	destroy(s->_aclV);
	destroy(s->_subnameV);
	destroy(s->_vp_tmpl);

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
	free(s);
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
handle_soundslike(int dist, int frequency, wchar_t *word, wchar_t **best, int *mindist, int *maxfreq)
{
	if (dist < *mindist) {
		*mindist = dist;
		if (*best != NULL) free(*best);
		*best = wcsdup(word);
		*maxfreq = frequency;
	} else if (dist == *mindist && *maxfreq < frequency) {
		*maxfreq = frequency;
		if (*best != NULL) free(*best);
		*best = wcsdup(word);
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

int allowed_we(const spelling_t *s, struct wordelem *we, container *groups, container *subnames)
{
    bblog(DEBUGINFO, "\n[spelling] ");
    int i;
    bblog(DEBUGINFO, "%s | subnames(%i): ", we->word, we->collections_len);

    if (set_size(subnames)==0)
	{
		bblog(ERROR, "Dident have access to any collection, so denying spelling");
		return 0;
	}

    int subname_allowed=0;

    for (i=0; i < we->collections_len; i++)
	{
	    iterator	it = set_find(subnames, vector_get(s->_subnameV, we->collections[i]).str);
	    bblog(DEBUGINFO, "%s ", vector_get(s->_subnameV, we->collections[i]).str);
	    if (it.valid)
		{
		    subname_allowed = 1;
		    bblog(DEBUGINFO, "(in collection)");
		    break;
		}
	}

    if (!subname_allowed)
	{
	    bblog(DEBUGINFO, "(not in collection)\n");
	    return 0;
	}


    // We have no group info, proboly because we have an anonynmus searc. Skipping (and allowing).
    if (groups==NULL) {
	return 1;
    }

    bblog(DEBUGINFO, "\ndenied(%i): ", we->acl_denied_len);

    for (i=0; i < we->acl_denied_len; i++)
	{
	    iterator	it = set_find(groups, vector_get(s->_aclV, we->acl_denied[i]).str);
	    bblog(DEBUGINFO, "%s ", vector_get(s->_aclV, we->acl_denied[i]).str);

	    if (it.valid)
		{
		    bblog(DEBUGINFO, "(denied)\n");
		    return 0;
		}
	}

    bblog(DEBUGINFO, "(not denied)\nallowed(%i): ", we->acl_allowed_len);

    for (i=0; i < we->acl_allowed_len; i++)
	{
	    iterator	it = set_find(groups, vector_get(s->_aclV, we->acl_allowed[i]).str);
	    bblog(DEBUGINFO, "%s ", vector_get(s->_aclV, we->acl_allowed[i]).str);

	    if (it.valid)
		{
		    bblog(DEBUGINFO, "(allowed)\n");
		    return 1;
		}
	}

    bblog(DEBUGINFO, "(not in acl_allowed)\n");
    return 0;
}


int
correct_word(const spelling_t *s, char *word, container *groups, container *subnames)
{
	if (s->words == NULL) return 1;

	utf8_strtolower((utf8_byte*)word);

	struct wordelem *we = hashtable_search(s->words, word);

	if (we == NULL || !allowed_we(s, we, groups, subnames))
		return 0;
	return 1;
}

void
check_soundslike(const spelling_t *s, scache_t *c,  wchar_t *wword, wchar_t *like, wchar_t **bestw, int *mindist, int *maxfreq, int phase, container *groups, container *subnames)
{
	container *list;
	//struct hashtable_itr *itr;
	//iterator itr;
	int	i;

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

	char *clike = wcstostrdup(like);
	list = hashtable_search(s->soundslike, clike);
	free(clike);
	if (list == NULL) {
		return;
	}

	vector_attach_data(s->_vp_tmpl, list);	// s->_vp_tmpl <-> list
	//printf("Words matching that(%ls):\n", like);
	//for (itr = list_begin(list); itr.valid; itr = list_next(itr)) {
	for (i=0; i<vector_size(s->_vp_tmpl); i++) {
		struct wordelem *we;
		int dist;

		we = vector_get(s->_vp_tmpl,i).ptr;
		if (!allowed_we(s, we, groups, subnames)) continue;

		wchar_t *we_wword = strtowcsdup(we->word);

		dist = levenshteindistance(wword, we_wword);
		handle_soundslike(dist, we->frequency, we_wword, bestw, mindist, maxfreq);

		free(we_wword);

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
	wchar_t	*wword;
	wchar_t *like;
	scache_t *cache;
	char u8word[LINE_MAX];

	*found = 0;

	bblog(DEBUGINFO,"inited p: %p, words p %p",s->inited,s->words);

	if (!s->inited)
		return NULL;

	if (s->words == NULL) {
		return NULL;
	}

	cache = create_hashtable(7, ht_wstringhash, ht_wstringcmp);
	if (cache == NULL) {
		return NULL;
	}

	utf8_strtolower((utf8_byte*)word);
	wword = strtowcsdup(word);

	// Phase 1, sounds like words
	like = dmetaphone(wword);
	bestw = NULL;
	bblog(DEBUGINFO, "We sound like: %ls\n", like);
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
		free(bestw);

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
	spelling_t *s;
	//time_t start, end;

	setlocale(LC_ALL, "en_US.UTF-8");

	s = train("/home/boitho/boithoTools/var/dictionarywords");
	if (s == NULL) {
		printf("Error: train() returned NULL.");
		return 0;
	}

#if 0

	struct hashtable_itr *itr;
	itr = hashtable_iterator(s->soundslike);

	do {
		wchar_t *key = hashtable_iterator_key(itr);

		printf("Sounds like: %ls\n", key);
		printf("Sounds like2: %s\n", key);
	} while (hashtable_iterator_advance(itr));

#endif

#if 0
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
#endif

	untrain(s);

	return 0;
}
#endif
