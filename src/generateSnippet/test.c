
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "../query/query_parser.h"
#include "snippet.parser.h"

int main(int argc, char *argv[])
{
    if (argc<3)
	{
	    printf("Usage: %s <query> <preparsed_file> {<preparsed_file>}\n\n", argv[0]);
	}

    int		paramnr;
//    char	*sok = "Clustering \"Used to \" \"the combination\" \"patches whicH\"";
//    char	*sok = argv[1];
    char	*sok = "Magnus Galåen";
    query_array	qa;

    get_query(sok, strlen(sok), &qa);

    for (paramnr = 2; paramnr<argc; paramnr++)
	{
	    FILE	*file = fopen(argv[paramnr], "ro");

	    if (!file)
		{
		    printf("Could not open %s\n", argv[paramnr]);
		    return -1;
		}

	    struct stat	fileinfo;
	    fstat( fileno(file), &fileinfo );

	    int		size = fileinfo.st_size;
	    char	*buf = malloc(size);
	    int		i;

	    for (i=0; i<size;)
		{
		    i+= fread( &(buf[i]), sizeof(char), size-i, file );
		}

	    char	*snippet;

	    generate_snippet( qa, buf, size, &snippet, "\033[1;32m", "\033[0m", 320 );

	    printf("%s\n", snippet);
	    free(snippet);
	    free(buf);

	    fclose(file);
	}

    destroy_query(&qa);
}
