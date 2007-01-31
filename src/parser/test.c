
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "html_parser.h"


void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf )
{
    printf("\t%s (%i) ", word, pos);

    switch (pu)
	{
	    case pu_word: printf("[word]"); break;
	    case pu_linkword: printf("[linkword]"); break;
	    case pu_link: printf("[link]"); break;
	    case pu_baselink: printf("[baselink]"); break;
	    case pu_meta_keywords: printf("[meta keywords]"); break;
	    case pu_meta_description: printf("[meta description]"); break;
	    case pu_meta_author: printf("[meta author]"); break;
	    default: printf("[...]");
	}

    switch (puf)
	{
	    case puf_none: break;
	    case puf_title: printf(" +title"); break;
	    case puf_h1: printf(" +h1"); break;
	    case puf_h2: printf(" +h2"); break;
	    case puf_h3: printf(" +h3"); break;
	    case puf_h4: printf(" +h4"); break;
	    case puf_h5: printf(" +h5"); break;
	    case puf_h6: printf(" +h6"); break;
	}

    printf("\n");
}



int main( int argc, char *argv[] )
{
    if (argc<2)
	{
	    printf( "Usage: %s <filename> {<filename>}\n\n", argv[0] );
	    exit(-1);
	}

    int		paramnr;

    for (paramnr=1; paramnr<argc; paramnr++)
	{
	    FILE	*file = fopen( argv[paramnr], "r" );

	    if (!file)
		{
		    fprintf( stderr, "Could not open %s.\n", argv[1] );
		    return -1;
		}

	    // Get filesize:
	    struct stat	fileinfo;
	    fstat( fileno( file ), &fileinfo );

	    printf("Reading %i bytes...\n", fileinfo.st_size);

	    int		size = fileinfo.st_size;
	    char	*buf = (char*)malloc(sizeof(char)*size);

	    int	i;
	    for (i=0; i<size;)
		{
		    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
		    printf("%i...\n", i);
		}

//	    for (;;)
		run_html_parser( "http://YAHOOgroups.com/svada/index.html", buf, size, fn );
		//twise
		run_html_parser( "http://YAHOOgroups.com/svada/index.html", buf, size, fn );
	    free(buf);

	    fclose(file);
	}

exit(0);
}
