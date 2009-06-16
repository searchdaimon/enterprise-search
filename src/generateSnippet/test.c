
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "../query/query_parser.h"
#include "../query/stemmer.h"
#include "snippet.parser.h"

int main(int argc, char *argv[])
{
    if (argc<3)
	{
	    printf("Usage: %s <query> <preparsed_file> {<preparsed_file>}\n\n", argv[0]);
	    exit(0);
	}

    int		paramnr;
//    char	*sok = "Clustering \"Used to \" \"the combination\" \"patches whicH\"";
    char	*sok = argv[1];
//    char	*sok = "Magnus Galåen";
    query_array	qa;

    get_query(sok, strlen(sok), &qa);

//    thesaurus		*T = thesaurus_init("../../data/thesaurus.text", "../../data/thesaurus.id");
    thesaurus		*T = thesaurus_init("../../data/mini.thesaurus.text", "../../data/mini.thesaurus.id");

    // Kjør stemming på query:
    thesaurus_expand_query(T, &qa);

    // Print query med innebygd print-funksjon:
    char	buf[1024];
    sprint_expanded_query(buf, 1023, &qa);
//    sprint_query(buf, 1023, &qa);
    printf("\nQuery: %s\n\n", buf);

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

	    if (paramnr>2) printf("\n     ---------------\n\n");

	    int		modes[3] = {first_snippet, plain_snippet, db_snippet};
	    for (i=0; i<3; i++)
		{
		    char	*snippet;

		    int		success = generate_snippet( qa, buf, size, &snippet, "\033[1;32m[", "]\033[0m", modes[i], 320, 4, 80 );

		    if (i>0) printf("\n");
		    printf("%s\n", snippet);
		    free(snippet);

		    if (!success)
			printf("FAILURE WHEN PARSING.\n");
		}

	    free(buf);

	    fclose(file);
	}

    destroy_query(&qa);
    thesaurus_destroy(T);
}
