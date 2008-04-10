#include "../common/define.h"
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

	while(!feof(UPDATEFILE)) {
		fread(&anchorfileData,sizeof(struct anchorfileFormat),1,UPDATEFILE);


		printf("%u : %s\n",anchorfileData.DocID,anchorfileData.text);
	
		
	}
	fclose(UPDATEFILE);
	
}



