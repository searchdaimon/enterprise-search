/*
 *	(C) Searchdaimon 2004-2008, Written by Magnus Galåen (mg@searchdaimon.com)
 *
 *	Example for "query_parser".
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "query_parser.h"
#include "stemmer.h"


void test_expand( char *tekst );
void testit( char *tekst );

int main( int argc, char *argv[] )
{
    if (argc>1)
	{
	    testit( argv[1] );
	    test_expand( argv[1] );
	}

    return 0;
}

void test_expand( char *tekst )
{
    printf("Loading thesaurus..."); fflush(stdout);

    thesaurus		*T = thesaurus_init("../../data/thesaurus.text", "../../data/thesaurus.id");

    printf("done\n");

    query_array		qa;

    get_query( tekst, strlen(tekst), &qa);

    thesaurus_expand_query(T, &qa);

    char	buf[1024];
    sprint_expanded_query(buf, 1023, &qa);

    printf("Expanded query: %s\n", buf);

    destroy_query(&qa);

    thesaurus_destroy(T);
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

