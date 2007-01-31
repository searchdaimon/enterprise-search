#include <stdlib.h>
#include <stdio.h>

main(int argc, char *argv[])
{
	FILE *FH;
	unsigned char rank;
	unsigned int rankArray[255];
	int i;

	for(i=0;i<=255;i++) {
		rankArray[i] = 0;
	};


        if ((FH = fopen(argv[1],"r")) == NULL) {
                perror(argv[1]);
                exit(1);
        }

	
	while (!feof(FH)) {
		fread(&rank,sizeof(unsigned char),1,FH);

		++rankArray[rank];
		
		//printf("%u\n",rank);
	}

	fclose(FH);

	for(i=0;i<=255;i++) {
                printf("%i %i\n",i,rankArray[i]);
        };
}
