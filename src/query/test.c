/*
 *	(C) Boitho 2004-2007, Written by Magnus Galåen
 *
 *	Example for "query_parser".
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "query_parser.h"

void testit();

int main( int argc, char *argv[] )
{
    if (argc>1)
	testit( argv[1] );

    return 0;
}

void testit( char *tekst )
{
//    char		*tekst = "-Jeg -søker etter- \"Magnus Galåen\", Lars Monrad-Krohn og -\"Lille-Jon";
//    char		*tekst = "+hvor- \" \"  -e\"r - d'u\"\"?";
//    char		*tekst = "boitho date:\"this week\"";
    query_array		qa;

//    tekst = "espen Øxnes æøå ÆØLÅ aæødlapAWÅÆAØÅ Ã¦Ã¸Ã¥ Ã¦æÃ¸aÃB¥";

    get_query( tekst, strlen(tekst), &qa);


    char	buf[1024];

    sprint_query(buf, 1023, &qa);

    printf("%s\n", buf);

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
    destroy_query( &qa );
}
