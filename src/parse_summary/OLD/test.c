
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "summary.h"

//char	text[] = "<html><title>Test</title><body>Dette er en test for å finne ut om dette fungerer eller ei.</body></html>";

int main( int argc, char *argv[] )
{
    if (argc<2)
	{
//	    printf( "Usage: %s <filename> <filename>\n\n", argv[0] );
	    printf( "Usage: %s <filename>\n\n", argv[0] );
	    exit(-1);
	}

    FILE	*file = fopen( argv[1], "r" );

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

/*
    for (i=0; i<fileinfo.st_size; i++)
	printf("%c", buf[i]);
    printf("\n");
*/

    char	*title, *body, *metakeyw, *metadesc;
    generate_summary( buf, size, &title, &body, &metakeyw, &metadesc );

//    struct summary	*S = generate_summary( text, sizeof(text) );

    printf("Title: { %s }\n", title);
    printf("Body: { %s }\n", body);
    printf("Meta Keywords: { %s }\n", metakeyw);
    printf("Meta Description: { %s }\n", metadesc);


    free(buf);
    free(title);
    free(body);
    free(metakeyw);
    free(metadesc);

    fclose(file);

    /**********/
/*
    file = fopen( argv[2], "r" );

    if (!file)
	{
	    fprintf( stderr, "Could not open %s.\n", argv[2] );
	    return -1;
	}

    // Get filesize:
    fstat( fileno( file ), &fileinfo );

    printf("Reading %i bytes...\n", fileinfo.st_size);

    size = fileinfo.st_size;
    buf = (char*)malloc(sizeof(char)*size);

    for (i=0; i<size;)
	{
	    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
	    printf("%i...\n", i);
	}

//    for (i=0; i<fileinfo.st_size; i++)
//	printf("%c", buf[i]);
//    printf("\n");

    generate_summary( buf, size, &title, &body );

//    struct summary	*S = generate_summary( text, sizeof(text) );

    printf("Title: { %s }\n", title);
    printf("Body: { %s }\n", body);
*/
}
