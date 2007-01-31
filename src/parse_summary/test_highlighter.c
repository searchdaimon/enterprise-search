
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../query/query_parser.h"
#include "summary.h"
#include "highlight.h"







int main( int argc, char *argv[] )
{
    if (argc<2)
	{
//	    printf( "Usage: %s <filename> <filename>\n\n", argv[0] );
	    printf( "Usage: %s <filename> [<filename> [<filename> [...]]]\n\n", argv[0] );
	    exit(-1);
	}

    int	arg_id;

    for (arg_id=1; arg_id<argc; arg_id++)
	{
	    FILE	*file = fopen( argv[arg_id], "r" );

	    if (!file)
		{
		    fprintf( stderr, "Could not open %s.\n", argv[arg_id] );
		    return -1;
		}

	    // Get filesize:
	    struct stat	fileinfo;
	    fstat( fileno( file ), &fileinfo );

	    int		size = fileinfo.st_size;
	    char	*buf = (char*)malloc(sizeof(char)*size);

	    int	i;
	    for (i=0; i<size;)
		{
		    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
		}

	    char	*title, *body, *metakeyw, *metadesc;
	    generate_summary( buf, size, &title, &body, &metakeyw, &metadesc );

            free(buf);
/*
	    printf("Title: { %s }\n", title);
	    printf("Body: { %s }\n", body);
	    printf("Meta Keywords: { %s }\n", metakeyw);
	    printf("Meta Description: { %s }\n", metadesc);
*/
	    // Invoke highlighter:

	    // Lage query:
	    query_array		qa;
//	    char		*tekst = "\"hotel and casino\" \"Las Vegas\"";
//	    char		*tekst = "\"et treff\"";
//	    char		*tekst = "\"stian rustad\"";
//	    char		*tekst = "bunnpris";
//	    char		*tekst = "stian rustad";
	    char		*tekst = "lars monrad-krohn";
	    get_query( tekst, strlen(tekst), &qa );

		// Generer highlighting:
	    char	*summary;
	    generate_highlighting( qa, body, strlen(body)+1, &summary );

	    // Ødelegge query:
	    destroy_query( &qa );

	    printf("\n%s\n", summary);

	    free(summary);
	    // end (Invoke highlighter)

	    free(title);
	    free(body);
	    free(metakeyw);
	    free(metadesc);

	    fclose(file);
	}
}

