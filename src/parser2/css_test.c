
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "css_parser.h"

int main(int argc, char *argv[])
{
    char	*text1 = "color: black; background: red url(\"teddy.png\"); text-align: left";
    char	*text2 = "color:blue";
    char	*text3 = strdup("h2 { font-size: large; }");
    css_selector_block	*selector_block;

/*
    assert(argc>=2);
    FILE	*file = fopen( argv[1], "r" );

    // Get filesize:
    struct stat	fileinfo;
    fstat( fileno( file ), &fileinfo );

    int		size = fileinfo.st_size;
    char	*buf = malloc(size+1);

    int	i;
    for (i=0; i<size;)
	{
	    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, file );
	}

    css_parser_init();
    selector_block = css_parser_run(buf, size);
//    css_parser_run(text3, strlen(text3));
    destroy_selectors(selector_block);
    css_parser_exit();

    free(buf);
    fclose(file);
*/
    css_parser_init();
    selector_block = css_parser_run(text3, strlen(text3));
    destroy_selectors(selector_block);
    css_parser_exit();
    free(text3);
}
