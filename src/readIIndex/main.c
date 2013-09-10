#include <stdio.h>
#include <stdlib.h>


int main (int argc, char *argv[]) {

	int i,y;

	FILE *fileha;

        unsigned long DocID;
        unsigned long TermAntall;
        unsigned short hit;

	unsigned long term;
	unsigned long Antall;;
	unsigned char langnr;

        if (argc < 2) {
                printf("This program prints an inverted index. Pleas specify iindex file\n\n\tUsage: ./readIIndex /boithoData/lot/1/iindex/collection/Main/index/aa/1.txt");
                exit(0);
        }


        if ((fileha = fopen(argv[1],"rb")) == NULL) {
                perror(argv[1]);
        }


	while (!feof(fileha)) {
		//wordid hedder
        	if (fread(&term,sizeof(unsigned long),1,fileha) != 1) {
			perror("can't read term: ");
			continue;
		}
        	fread(&Antall,sizeof(unsigned long),1,fileha);

		printf("term: %u antall: %u\n",term,Antall);

		for (i=0;i<Antall;i++) {
			//side hedder
			if (fread(&DocID,sizeof(unsigned long),1,fileha) != 1) {
				printf("can't read DocID for nr %i\n",i);
				perror("");
				continue;
			}
			fread(&langnr,sizeof(char),1,fileha);
        		fread(&TermAntall,sizeof(unsigned long),1,fileha);

			printf("DocID: %u, langnr: %i, nr: %u. Hits: ",DocID,(int)langnr,TermAntall);

			for (y = 0;y < TermAntall; y++) {
                		        fread(&hit,sizeof(unsigned short),1,fileha);

					printf("%hu,",hit);

			}
			printf("\n");
		}	
		//printf("\n\n");
	}
	fclose(fileha);
}
