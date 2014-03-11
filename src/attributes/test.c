
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "show_attributes.h"


int main()
{
    FILE	*conf = fopen("../../config/show_attributes.conf", "r");
    struct stat	fileinfo;
    fstat( fileno( conf ), &fileinfo );

    int		size = fileinfo.st_size;
    char	*buf = (char*)malloc(sizeof(char)*size);

    int	i;
    for (i=0; i<size;)
	{
	    i+= fread( (void*)&(buf[i]), sizeof(char), size-i, conf );
	}

    char	*warnings;
    int		failed;
    attr_conf	*ac = show_attributes_init(buf, &warnings, &failed, 1);

    printf("Warnings:\n\n%s", warnings);

    free(warnings);
    show_attributes_destroy(ac);
}
