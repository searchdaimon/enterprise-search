/*
 *	(C) Searchdaimon 2004-2009, Written by Magnus Galåen (mg@searchdaimon.com)
 *
 *	Example for "query_parser".
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../common/boithohome.h"
#include "../ds/dcontainer.h"
#include "../ds/dmap.h"

#include "query_parser.h"
#include "stemmer.h"


void test_expand( thesaurus *T, char *tekst );
void testit( char *tekst );

int main( int argc, char *argv[] )
{
    container	*BigT = load_all_thesauruses(bfile("data/thesaurus/"));

    iterator	it = map_find(BigT, "nbo");
    thesaurus	*T = NULL;
    if (it.valid) T = map_val(it).ptr;

    if (argc>1)
	{
//	    testit( argv[1] );
	    test_expand( T, argv[1] );
	}

    destroy_all_thesauruses( BigT );

    return 0;
}

void test_expand( thesaurus *T, char *tekst )
{
    query_array		qa;

    // Parse query:
    get_query( tekst, strlen(tekst), &qa);

    // Kjør stemming på query:
    thesaurus_expand_query(T, &qa);

    // Print query med innebygd print-funksjon:
    char	buf[1024];
    sprint_expanded_query(buf, 1023, &qa);
    printf("\nExpanded query: %s\n\n", buf);

    // --- Eksempel på iterering av expanded query:

    int		i, j, k;

    for (i=0; i<qa.n; i++)
	{
	    printf(" %c:", qa.query[i].operand);

	    // Brukerens query:
	    if (qa.query[i].n > 1 || qa.query[i].operand == QUERY_PHRASE) printf("\"");

	    for (j=0; j<qa.query[i].n; j++)
		{
		    if (j>0) printf(" ");
		    printf("%s", qa.query[i].s[j]);
		}

	    if (qa.query[i].n > 1 || qa.query[i].operand == QUERY_PHRASE) printf("\"");

	    // Expanded query (fra stemming og synonymer):
	    if (qa.query[i].alt != NULL)
		{
		    printf("(");
		    for (j=0; j<qa.query[i].alt_n; j++)
			{
			    if (j>0) printf("|");
			    if (qa.query[i].alt[j].n > 1) printf("\"");

			    for (k=0; k<qa.query[i].alt[j].n; k++)
				{
				    if (k>0) printf(" ");
				    printf("%s", qa.query[i].alt[j].s[k]);
				}

			    if (qa.query[i].alt[j].n > 1) printf("\"");
			}
		    printf(")");
		}
	}

    printf("\n");

    // ---

    destroy_query(&qa);
}




void testit( char *tekst )
{
//    char		*tekst = "-Jeg -søker etter- \"Magnus Galåen\", Lars Monrad-Krohn og -\"Lille-Jon";
//    char		*tekst = "+hvor- \" \"  -e\"r - d'u\"\"?";
//    char		*tekst = "boitho date:\"this week\"";
    query_array		*qa = malloc(sizeof(query_array));
    query_array		qa2;

//    tekst = "espen Øxnes æøå ÆØLÅ aæødlapAWÅÆAØÅ Ã¦Ã¸Ã¥ Ã¦æÃ¸aÃB¥";

    get_query( tekst, strlen(tekst), qa);
    copy_query( &qa2, qa );

    char	buf[1024];

    sprint_query(buf, 1023, qa);
    printf("%s\n", buf);

    sprint_query(buf, 1023, &qa2);
    printf("%s\n", buf);

    sprint_query_array(buf, 1023, qa);
    printf("%s\n", buf);

    destroy_query( qa );
    free(qa);

    destroy_query( &qa2 );

/*
    int			i,j;
    printf("\nquery %s\n",tekst);

    for (i=0; i<qa.n; i++)
	{
	    printf("(%c)", qa.query[i].operand );

		for (j=0; j<qa.query[i].n; j++)
		{
			printf(" %s", qa.query[i].s[j]);
		}

	    printf("\n");
	}

    printf("\n");
*/
/*
    query_array		qa_html;
    copy_htmlescaped_query( &qa_html, &qa );

    for (i=0; i<qa_html.n; i++)
	{
	    printf("(%c)", qa_html.query[i].operand );

		for (j=0; j<qa_html.query[i].n; j++)
		{
			printf(" [%s]", qa_html.query[i].s[j]);
		}

	    printf("\n");
	}

    destroy_query( &qa_html );
*/
}

