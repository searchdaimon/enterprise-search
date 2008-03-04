#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "dp.h"

int main (int argc, char *argv[]) {


        if (argc < 2) {
                printf("Dette programet leser en fil\n\n\tBruk: ./dptest file\n");
                exit(0);
        }


	FILE *FH;
	int count = 0;
	char buf[1024];
	int n;

	dp_priority_locl_start();

	if ((FH = fopen(argv[1],"r")) == NULL) {
		perror(argv[1]);
		exit(1);
	}

	while ((n = fread(buf,sizeof(char),sizeof(buf),FH)) > 0) {
		count += n;
	}



	fclose(FH);

	dp_priority_locl_end();

	printf("did read %i bytes\n",count);

	return 0;
}

