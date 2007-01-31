//#include "../common/define.h"
#include "../common/lot.h"
#include "../common/reposetory.h"

#include <stdio.h>



int main (int argc, char *argv[]) {

	FILE *UPDATEFILE;
	struct anchorfileFormat anchorfileData;

	//tester for at vi har fåt hvilken fil vi skal bruke
	if (argc < 2) {
		printf("Usage: ./addanchors anchorfile\n\n\tanchorfile, fil med tekster på linker\n\n");
		exit(1);
	}

	if ((UPDATEFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read anchorfile ");
                perror(argv[1]);
                exit(1);
        }

	lotlistLoad();
	void lotlistMarkLocals(char server[]);

	while(!feof(UPDATEFILE)) {
		fread(&anchorfileData,sizeof(struct anchorfileFormat),1,UPDATEFILE);


		//printf("%i : %s\n",anchorfileData.DocID,anchorfileData.text);
	
		//sjekker om dette er en lokal lot		
		//temp: utestet:
		if (lotlistIsLocal(rLotForDOCid(anchorfileData.DocID))) {
			anchoradd(anchorfileData.DocID,anchorfileData.text,sizeof(anchorfileData.text));
		}
		else {
			printf("lot is not locale");
		}
	}
	fclose(UPDATEFILE);
	
}



