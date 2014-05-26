#include "../common/reposetory.h"

#include <stdlib.h>





main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];

	if (argc < 2) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);

	GetFilePathForIindex(&lotPath,LotNr);
	printf("Opning lot at: %s\n",lotPath);


	//loppergjenom alle
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,imagebuffer,&radress,0,0)) {

		printf("DocId: %i url: %s\n",ReposetoryHeader.DocID,ReposetoryHeader.url);


		//skriver datane til indeks

		//sletter litt slik at vi ser om det blir noen feil i lesingen
		ReposetoryHeader.DocID = -1;

	}
	
	
}
