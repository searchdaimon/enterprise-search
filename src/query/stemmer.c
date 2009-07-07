
/**
 *	stemmer.c
 *
 *	(C) Copyright SearchDaimon AS 2008-2009, Magnus Galåen (mg@searchdaimon.com)
 *
 *	Slår opp stems og bøyninger av ord.
 *
 *	TODO: utnytte endinger (ord som begynner med '-').
 *	Støtter ikke bøyninger av f.eks "world cup" (flerordsbøyninger),
 *	men støtter motsatt vei "tv2" blir "tv 2". Bør forandre query-struct'en
 *	for å få med disse.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "../ds/dcontainer.h"
#include "../ds/dmap.h"
#include "../ds/dset.h"
#include "../common/utf8-strings.h"
#include "../common/boithohome.h"
#include "stemmer.h"



container* load_all_thesauruses(char *path)
{
    DIR		*dir = opendir(path);

    if (!dir)
	{
	    printf("[thesaurus] Could not find directory '%s'. Stemming will be disabled.\n", path);
	    return NULL;
	}
    else printf("[thesaurus] Reading thesauruses from '%s'.\n", path);

    container	*Stext = set_container( string_container() ),
		*Sid = set_container( string_container() );

    struct dirent	*entry;
    while ((entry = readdir(dir)) != NULL)
	{
	    char	prefix[4], postfix[6];
	    strncpy(prefix, entry->d_name, 4);
	    if (strlen(entry->d_name) > 3) strncpy(postfix, &((entry->d_name)[3]), 6);
	    else postfix[0] = '\0';
	    prefix[3] = '\0';
	    postfix[5] = '\0';

	    //printf("%s [%s:%s]\n", entry->d_name, prefix, postfix);
	    // [a-z][a-z][a-z].[text|id]
	    if (!strcmp(postfix, ".text")) set_insert(Stext, prefix);
	    if (!strcmp(postfix, ".id")) set_insert(Sid, prefix);
	}

    closedir(dir);

    container *Thesauruses = ds_intersection(set_begin2(Stext), set_begin2(Sid));

    destroy(Stext);
    destroy(Sid);

    container	*BigT = map_container( string_container(), ptr_container() );

    iterator	it = set_begin(Thesauruses);
    for (; it.valid; it=set_next(it))
	{
	    char	*n_text, *n_id, *lang = set_key(it).str;
	    asprintf(&n_text, "%s%s.text", path, lang);
	    asprintf(&n_id, "%s%s.id", path, lang);

	    printf("[thesaurus] Loading \"%s\"...\n", lang);
	    map_insert(BigT, lang, thesaurus_init(n_text, n_id));

	    free(n_text);
	    free(n_id);
	}

    destroy(Thesauruses);

    println(BigT);

    return BigT;
}


void destroy_all_thesauruses( container *BigT )
{
    iterator	it = map_begin(BigT);
    for (; it.valid; it=map_next(it))
	{
	    thesaurus_destroy(map_val(it).ptr);
	}

    destroy(BigT);
}




// tword-flags:
const char tword_stem=1, tword_dup=2;


thesaurus* thesaurus_init( char *fname_text, char *fname_id )
{

    printf("thesaurus_init(%s, %s)\n",fname_text,fname_id);

    FILE	*f_text = fopen(fname_text, "r"),
		*f_id = fopen(fname_id, "r");

    if (f_text==NULL || f_id==NULL)
	{
	    printf("Error: thesaurus: Could not open files for reading.\n");
	    return NULL;
	}

    

    thesaurus	*T = malloc(sizeof(thesaurus));
    fscanf(f_id, "%i\n", &T->Id_size);
    fscanf(f_text, "%i\n", &T->W_size);

    T->W = malloc(sizeof(tword)*T->W_size);
    T->Id = malloc(sizeof(tword)*T->Id_size);

    int		i;

    for (i=0; !feof(f_id) && i<T->Id_size; i++)
	{
	    int		id, flags;
	    char	word[1024];
	    int		pos=0;

	    fscanf(f_id, "%i;", &id);
	    fscanf(f_id, "%i;", &flags);
	    for (; pos<1024 && (pos==0 || word[pos-1]!='\n'); pos++)
		{
		    word[pos] = fgetc(f_id);
		    if (word[pos] == '-') word[pos] = ' ';
		}
	    word[pos-1] = '\0';
	    convert_to_lowercase((unsigned char*)word);

	    T->Id[i].id = id;
	    T->Id[i].text = strdup(word);
	    T->Id[i].flags = flags;
	}

    for (i=0; !feof(f_text) && i<T->W_size; i++)
	{
	    int		id, flags;
	    char	word[1024];
	    int		pos=0;

//	    do { word[pos++] = fgetc(f_text); } while (pos<1024 && word[pos-1]!=';');
	    for (; pos<1024 && (pos==0 || word[pos-1]!=';'); pos++)
		{
		    word[pos] = fgetc(f_text);
		    if (word[pos] == '-') word[pos] = ' ';
		}
	    word[pos-1] = '\0';
	    convert_to_lowercase((unsigned char*)word);
	    fscanf(f_text, "%i;", &flags);
	    fscanf(f_text, "%i\n", &id);

	    T->W[i].id = id;
	    T->W[i].text = strdup(word);
	    T->W[i].flags = flags;
	}

    fclose(f_id);
    fclose(f_text);

    return T;
}


void thesaurus_destroy( thesaurus* T )
{
    if (T==NULL) return;

    int		i;

    for (i=0; i<T->Id_size; i++)
        free(T->Id[i].text);

    for (i=0; i<T->W_size; i++)
        free(T->W[i].text);

    free(T->W);
    free(T->Id);
    free(T);
}


int tword_compare_id(const void *_a, const void *_b)
{
    int			*a = (int*)_a;
    tword		*b = (tword*)_b;

    return (*a) - b->id;
}


int tword_compare_text(const void *_a, const void *_b)
{
    char		*a = (char*)_a;
    tword		*b = (tword*)_b;

    return strcmp(a, b->text);
}


container* thesaurus_get_words_from_id(thesaurus *T, int id)
{
    if (T==NULL) return NULL;
    tword	*match = (tword*)bsearch((const void*)&id, T->Id, T->Id_size, sizeof(tword), tword_compare_id);

    if (match!=NULL)
	{
	    container	*S = set_container( string_container() );
	    tword	*prev = match;

	    //fprintf(stderr, "  %s: %s\n", (match->flags&1?"stem":"word"), match->text);
	    set_insert(S, match->text);

	    while (prev!=&(T->W[0]) && (--prev)->id == id)
		{
		    //fprintf(stderr, " -%s: %s\n", (prev->flags&1?"stem":"word"), prev->text);
		    set_insert(S, prev->text);
		}

	    prev = match;

	    while (prev!=&(T->W[T->W_size-1]) && (++prev)->id == id)
		{
		    //fprintf(stderr, " +%s: %s\n", (prev->flags&1?"stem":"word"), prev->text);
		    set_insert(S, prev->text);
		}

	    return S;
	}

    return NULL;
}


container* thesaurus_get_synonyms(thesaurus *T, char *_word)
{
    if (T==NULL) return NULL;

    char	*word = strdup(_word);
    convert_to_lowercase((unsigned char*)word);

    tword	*match = (tword*)bsearch((const void*)word, T->W, T->W_size, sizeof(tword), tword_compare_text);

    if (match==NULL)
	{
	    #ifdef DEBUG
	    fprintf(stderr, "stemmer: No match for word \"%s\".\n", word);
	    #endif
	    free(word);
	    return NULL;
	}
    else
	{
	    container	*V = vector_container( string_container() );
	    container	*S = NULL; // = set_container( string_container() );
	    tword	*prev = match;

	    //fprintf(stderr, " id: %i flags: %i word: %s\n", match->id, match->flags, match->text);
	    S = thesaurus_get_words_from_id(T, match->id);

	    while (prev!=&(T->W[0]) && !strcmp((--prev)->text, word))
		{
		    //fprintf(stderr, "-id: %i flags: %i word: %s\n", prev->id, prev->flags, prev->text);
		    container	*s = thesaurus_get_words_from_id(T, prev->id);
		    if (s==NULL) continue;
		    iterator	it = set_begin(s);
		    for (; it.valid; it=set_next(it))
			set_insert(S, set_key(it).ptr);
		    destroy(s);
		}

	    prev = match;

	    while (prev!=&(T->W[T->W_size-1]) && !strcmp((++prev)->text, word))
		{
		    //fprintf(stderr, "+id: %i flags: %i word: %s\n", prev->id, prev->flags, prev->text);
		    container	*s = thesaurus_get_words_from_id(T, prev->id);
		    if (s==NULL) continue;
		    iterator	it = set_begin(s);
		    for (; it.valid; it=set_next(it))
			set_insert(S, set_key(it).ptr);
		    destroy(s);
		}

	    iterator	it = set_begin(S);
	    for (; it.valid; it=set_next(it))
		vector_pushback(V, set_key(it).ptr);
	    destroy(S);

	    free(word);
	    return V;
	}
}


void thesaurus_expand_query( thesaurus *T, query_array *qa )
{
    if (T==NULL) return;
    #ifdef DEBUG
	    fprintf(stderr, "stemmer: thesaurus_expand_query()\n");
    #endif

    int		i, j, k;

    for (i=0; i<qa->n; i++)
	{
	    if (qa->query[i].operand == QUERY_WORD)
		{
		    container	*V = thesaurus_get_synonyms(T, qa->query[i].s[0]);

		    if (V!=NULL)
			{
			    int		n = vector_size(V);

			    qa->query[i].alt_n = n;
			    qa->query[i].alt = malloc(sizeof(string_alternative)*n);

			    for (j=0; j<n; j++)
				{
				    int		m = 0;
				    char	*str = strdup((char*)vector_get(V,j).ptr);
				    char	delim = 1;

				    for (k=0; str[k]!='\0'; k++)
					{
					    if (str[k]!=' ' && delim)
						{
						    m++;
						    delim = 0;
						}
					    else if (str[k]==' ' && !delim)
						{
						    delim = 1;
						}
					}

				    qa->query[i].alt[j].n = m;
				    qa->query[i].alt[j].s = malloc(sizeof(char*)*m);

				    char	*ptrptr, *token;
				    token = strtok_r(str, " ", &ptrptr);

				    for (k=0; token!=NULL; k++)
					{
					    qa->query[i].alt[j].s[k] = strdup(token);
					    token = strtok_r(NULL, " ", &ptrptr);
					}

				    free(str);
				}

			    destroy(V);
			}
		}
	}

    #ifdef DEBUG
	    fprintf(stderr, "stemmer: ~thesaurus_expand_query()\n");
    #endif
}

