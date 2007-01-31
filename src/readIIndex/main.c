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
                printf("Dette programet printer ut en iindex. Gi den en fil inn\n\n");
                exit(0);
        }


        if ((fileha = fopen(argv[1],"rb")) == NULL) {
                perror(argv[1]);
        }


	while (!feof(fileha)) {
		//wordid hedder
        	fread(&term,sizeof(unsigned long),1,fileha);
        	fread(&Antall,sizeof(unsigned long),1,fileha);

		printf("term: %u antall: %u\n",term,Antall);

		for (i=0;i<Antall;i++) {
			//side hedder
			fread(&DocID,sizeof(unsigned long),1,fileha);
			fread(&langnr,sizeof(char),1,fileha);
        		fread(&TermAntall,sizeof(unsigned long),1,fileha);

			printf("DocID: %u, langnr: %i, nr: %u\n",DocID,(int)langnr,TermAntall);

			for (y = 0;y < TermAntall; y++) {
                		        fread(&hit,sizeof(unsigned short),1,fileha);

					//printf("%i,",hit);

			}
			//printf("\n");
		}	
		//printf("\n\n");
	}
	fclose(fileha);
}
