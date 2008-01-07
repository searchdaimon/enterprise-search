/**
 *	(C) Copyright Boitho 2007-2008. Written by Magnus Galåen
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "read_thesaurus.h"


int main( int argc, char *argv[] )
{
    if (argc!=3)
	{
	    printf( "Usage: %s <thesaurus> <query>\n\n", argv[0] );
	    return -1;
	}

    container	*T;

//    for (paramnr=1; paramnr<argc; paramnr++)
	{
	    FILE	*file = fopen( argv[1], "r" );

	    if (!file)
		{
		    fprintf( stderr, "Could not open %s.\n", argv[1] );
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

	    T = read_thesaurus(buf, size);

	    free(buf);
	    fclose(file);
	}

    container	*text = vector_container( string_container() );
    vector_pushback(text, argv[2]);

    int		i, j;
    container	*V = get_synonyms(T, text);

    printf("Synonyms:");
    if (V!=NULL)
	{
	    for (i=0; i<vector_size(V); i++)
		{
		    container	*W = vector_get(V,i).C;
		    for (j=0; j<vector_size(W); j++)
			printf(" %s", (char*)vector_get(W,j).ptr);

		    if (i<(vector_size(V)-1)) printf(",");
		}
	    printf("\n");
	}
    else printf(" <none>\n");

    destroy(text);

    destroy_synonyms(V);
    destroy(T);

    return 0;
}
