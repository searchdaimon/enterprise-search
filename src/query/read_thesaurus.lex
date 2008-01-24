%{
/*
 *	(C) Boitho 2007-2008, Written by Magnus Galåen
 *
 *	TODO:
 *
 *		* rydde og optimisere i koden
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"
#include "../ds/dmap.h"
#include "../ds/dset.h"
#include "../common/utf8-strings.h"
#include "read_thesaurus.h"



struct _rt_yy_extra
{
    container		*swlist;
    int			splitsize;
    char		*prefix;
    container		*optional;
    char		in_or;
    container		*synonyms;
    container		*current_group;
};

#define YY_EXTRA_TYPE	struct _rt_yy_extra*

static inline void add_firstword( char *text, yyscan_t yyscanner );
static inline void add_altword( char *text, yyscan_t yyscanner );

%}

letterstart	[a-zÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõöøùúûüışÿ]
letter		[0-9a-z'\#\+\-_ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõöøùúûüışÿ]
%option	noyywrap reentrant
%x OPTIONAL OR
%%
^#[^\n]*$		{}
\;			{
			    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );
			    int		i;

			    for (i=0; i<vector_size(rte->swlist); i++)
				{
				    container	*V = vector_get(rte->swlist, i).C;

				    if (vector_size(V) > 0)
					{
					    container	*N = vector_container( ptr_container() );
					    iterator	it = map_insert_value(rte->synonyms, container_value(V), container_value(N));

					    if (!it.valid)
						{
						    destroy(N);
						    it = map_find_value(rte->synonyms, container_value(V));
						}
					    else
						{
						    vector_set(rte->swlist, i, container_value(NULL));
						}

					    vector_pushback(rte->current_group, it.node);
					}
				}

			    clear(rte->swlist);
			    vector_pushback(rte->swlist);
			}
\#			{
			    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );
			    int		i, j;

			    for (i=0; i<vector_size(rte->current_group); i++)
				{
				    iterator	mit;
				    container	*B;

				    mit.node = vector_get(rte->current_group, i).ptr;
				    mit.valid = 1;
				    B = map_val(mit).C;

				    for (j=0; j<vector_size(rte->current_group); j++)
					{
					    if (i==j) continue;
					    vector_pushback(B, vector_get(rte->current_group, j).ptr);
					}
				}

			    clear(rte->current_group);
			}
{letter}*\(		{
			    int		len = strlen((char*)yytext);
			    char	*dup = malloc(len);

			    strncpy(dup, (char*)yytext, len);
			    dup[len-1] = '\0';

			    yyget_extra(yyscanner)->prefix = dup;
			    BEGIN OPTIONAL;
			}
{letter}+		{
			    add_firstword(yytext, yyscanner);
			}
<OPTIONAL>{letter}+	{
			    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );

			    vector_pushback(rte->optional, yytext);
			}
<OPTIONAL>\){letter}*	{
			    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );

			    if (vector_size(rte->optional)==1)	// hus(et)
				{
				    char		*shortstr, *longstr, *infix, *postfix;
				    int			len;

				    infix = vector_get(rte->optional, 0).ptr;
				    postfix = (char*)yytext;
				    postfix++;

				    len = strlen(rte->prefix) + strlen(postfix) + 1;
				    if (len>1)
					{
					    shortstr = malloc(len);

					    strcpy(shortstr, rte->prefix);
					    strcat(shortstr, postfix);
					}

				    longstr = malloc(len + strlen(infix));
				    strcpy(longstr, rte->prefix);
				    strcat(longstr, infix);
				    strcat(longstr, postfix);

				    if (len>1)
					{
					    if (rte->in_or)
						{
						    add_altword(shortstr, yyscanner);
						    rte->in_or = 0;
						}
					    else
						{
						    add_firstword(shortstr, yyscanner);
						}

					    add_altword(longstr, yyscanner);
					    free(shortstr);
					}
				    else
					{
					    unsigned char	*word = copy_latin1_to_utf8((unsigned char*)infix);
					    int		i, j;

					    for (i=0; i<rte->splitsize; i++)
						{
						    vector_pushback(rte->swlist);
						    container		*V = vector_get(rte->swlist, i).C;
						    container		*W = vector_get(rte->swlist, vector_size(rte->swlist)-1).C;

						    for (j=0; j<vector_size(V); j++)
							{
							    vector_pushback(W, (char*)vector_get(V,j).ptr);
							}

						    vector_pushback(W, word);
						}

					    free(word);
					}

				    free(longstr);
				}
			    else	// nedenom (og hjem)
				{
				    int		i, j;

				    for (i=0; i<rte->splitsize; i++)
					{
					    vector_pushback(rte->swlist);
					    container		*V = vector_get(rte->swlist, i).C;
					    container		*W = vector_get(rte->swlist, vector_size(rte->swlist)-1).C;

					    for (j=0; j<vector_size(V); j++)
						{
						    vector_pushback(W, (char*)vector_get(V,j).ptr);
						}

					    for (j=0; j<vector_size(rte->optional); j++)
						{
						    unsigned char	*word = copy_latin1_to_utf8((unsigned char*)vector_get(rte->optional, j).ptr);
						    vector_pushback(W, word);
						    free(word);
						}
					}
				}

			    free(rte->prefix);
			    clear(rte->optional);

			    BEGIN INITIAL;
			}
\/			{ BEGIN OR; }
<OR>\-{letter}+		{ BEGIN INITIAL; /* Ignore for now */ }
<OR>{letter}*\(		{
			    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );
			    int		len = strlen((char*)yytext);
			    char	*dup = malloc(len);

			    strncpy(dup, (char*)yytext, len);
			    dup[len-1] = '\0';

			    rte->prefix = dup;
			    rte->in_or = 1;
			    BEGIN OPTIONAL;
			}
<OR>{letter}+		{
			    add_altword(yytext, yyscanner);
			    BEGIN INITIAL;
			}
<*>.|\n			{}
%%


static inline void add_firstword( char *text, yyscan_t yyscanner )
{
    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );
    unsigned char	*word = copy_latin1_to_utf8((unsigned char*)text);
    int		i;

    rte->splitsize = vector_size(rte->swlist);

    for (i=0; i<vector_size(rte->swlist); i++)
	{
	    container	*V = vector_get(rte->swlist, i).C;

	    vector_pushback(V, word);
	}

    free(word);
}

static inline void add_altword( char *text, yyscan_t yyscanner )
{
    struct _rt_yy_extra	*rte = yyget_extra( yyscanner );
    unsigned char	*word = copy_latin1_to_utf8((unsigned char*)text);
    int		i, j;

    for (i=0; i<rte->splitsize; i++)
	{
	    vector_pushback(rte->swlist);
	    container		*V = vector_get(rte->swlist, i).C;
	    container		*W = vector_get(rte->swlist, vector_size(rte->swlist)-1).C;

	    for (j=0; j<vector_size(V)-1; j++)
		{
		    vector_pushback(W, (char*)vector_get(V,j).ptr);
		}

	    vector_pushback(W, word);
	}

    free(word);
}



container* read_thesaurus( char text[], int text_size )
{
    struct _rt_yy_extra		*rte = malloc(sizeof(struct _rt_yy_extra));

    rte->swlist = vector_container( vector_container( string_container() ) );
    vector_pushback(rte->swlist);

    rte->optional = vector_container( string_container() );
    rte->synonyms = map_container( vector_container( string_container() ), vector_container( ptr_container() ) );
    rte->current_group = vector_container( ptr_container() );

    rte->in_or = 0;

    yyscan_t	scanner;

    yylex_init( &scanner );
    yyset_extra( rte, scanner );
    YY_BUFFER_STATE	bs = yy_scan_bytes( text, text_size, scanner );

    yylex( scanner );

    yy_delete_buffer( bs, scanner );
    yylex_destroy( scanner );

/*
    int		i, j;

    iterator	mit = map_begin(rte->synonyms);
    for (; mit.valid; mit=map_next(mit))
	{
	    container	*A = map_key(mit).C;
	    container	*B = map_val(mit).C;

	    printf("[");
	    for (i=0; i<vector_size(A); i++)
		{
		    if (i>0) printf(" ");
		    printf("%s", (char*)vector_get(A,i).ptr);
		}
	    printf("]:");
	    for (j=0; j<vector_size(B); j++)
		{
		    iterator	it;
		    container	*Z;

		    it.node = vector_get(B, j).ptr;
		    it.valid = 1;
		    Z = map_key(it).C;

		    for (i=0; i<vector_size(Z); i++)
			printf(" %s", (char*)vector_get(Z,i).ptr);

		    if (j<(vector_size(B)-1)) printf(",");
		}
	    printf("\n");
	}
*/
    int		i;

    for (i=0; i<vector_size(rte->current_group); i++)
	free(vector_get(rte->current_group, i).ptr);
    destroy(rte->current_group);

    destroy(rte->swlist);
    destroy(rte->optional);

    container	*ret_container = rte->synonyms;

    free(rte);

    return ret_container;
}


container* get_synonyms( container *C, container *text )
{
    iterator	mit;
    container	*H, *N;
    int		i;

    mit = map_find_value(C, container_value(text));

    N = vector_container( vector_container( string_container() ) );
    if (!mit.valid) return N;

    H = map_val(mit).C;

    for (i=0; i<vector_size(H); i++)
	{
	    iterator	it;

	    it.node = vector_get(H, i).ptr;
	    it.valid = 1;

	    vector_pushback_value(N, container_value(map_key(it).C));
	}

    return N;
}


void destroy_synonyms( container *C )
{
    if (C==NULL) return;

    int		i;

    for (i=0; i<vector_size(C); i++)
	{
	    vector_set(C, i, container_value(NULL));
	}

    destroy(C);
}


void expand_query( container *C, query_array *qa )
{
    int			x,i;

    for (x=0; x<qa->n; x++)
	{
	    if (qa->query[x].operand == QUERY_WORD)
		{
		    container		*S = set_container( string_container() );
		    container		*Q = vector_container( string_container() );

		    convert_to_lowercase((unsigned char*)qa->query[x].s[0]);
		    vector_pushback(Q, qa->query[x].s[0]);
		    set_insert(S, qa->query[x].s[0]);

		    container		*stems = get_synonyms(C, Q);

		    for (i=0; i<vector_size(stems); i++)
			{
			    container	*R = vector_get(stems,i).C;

			    // Only do singleworded stems and synonyms for now:
			    if (vector_size(R) == 1)
				{
				    set_insert(S, (char*)vector_get(R,0).ptr);
			        }
			}

		    set_remove(S, qa->query[x].s[0]);

		    if (set_size(S) > 0)
			{
			    qa->query[x].alt_n = set_size(S);
			    qa->query[x].alt = malloc(sizeof(string_alternative) * qa->query[x].alt_n);

			    iterator	sit = set_begin(S);
			    for (i=0; sit.valid; sit=set_next(sit),i++)
				{
				    qa->query[x].alt[i].n = 1;
				    qa->query[x].alt[i].s = malloc(sizeof(char*) * qa->query[x].alt[i].n);
				    qa->query[x].alt[i].s[0] = strdup((char*)set_key(sit).ptr);
				}
			}

		    destroy_synonyms(stems);
		    destroy(S);
		    destroy(Q);
		}
	}
}

