/*
 *	(C) Boitho 2004-2008, Written by Magnus Galåen
 *
 *	Example for "query_parser".
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../common/bfileutil.h"
#include "query_parser.h"
#include "read_thesaurus.h"

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
    // Les inn stemwords:
    char	*buf;
    int		size;

    size = readfile_into_buf("./../../data/stemwords.NBO", &buf);
//    printf("opening file: "); fflush(stdout);
//    size = readfile_into_buf("thesaurus.NBO", &buf);
//    printf("done\nparsing: "); fflush(stdout);
    container	*D = read_thesaurus(buf, size);	// Selve kommandoen for å parse ordboka. Dette trengs bare gjøres én gang.
//    printf("done\n");
    free(buf);

    /***/

    query_array		qa;

    get_query( tekst, strlen(tekst), &qa );
    expand_query( D, &qa );	// Ekspander query med stems.

    char		s[1024];
    sprint_expanded_query(s, 1023, &qa);
    printf("%s\n", s);

    /***/
    /***/

    int		i, j, k;
    printf("\nquery %s\n",tekst);

    for (i=0; i<qa.n; i++)
	{
	    printf("(%c)", qa.query[i].operand );

	    for (j=0; j<qa.query[i].n; j++)
		{
			printf(" %s", qa.query[i].s[j]);
		}

	    if (qa.query[i].alt != NULL)
		{
		    printf(" (");

		    for (j=0; j<qa.query[i].alt_n; j++)
			{
			    if (j>0) printf("|");

			    for (k=0; k<qa.query[i].alt[j].n; k++)
				printf("%s", qa.query[i].alt[j].s[k]);
			}

		    printf(")");
		}

	    printf("\n");
	}

    printf("\n");

    destroy_query(&qa);
    destroy(D);	// Deallokér ordboka.
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

