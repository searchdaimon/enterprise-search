#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/poprank.h"

#include <stdlib.h>
#include <string.h>





main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];
	int rank;

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl;

	if (argc < 5) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www servername minrank\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	char *subname = argv[2];
	char *servername = argv[3];
	int minrank = atoi(argv[4]);

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);

	popopenMemArray(servername,"www");

	//loppergjenom alle
// int rGetNext (unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[],
// int htmlbufferSize, char imagebuffer[], unsigned int *radress,
// unsigned int FilterTime, unsigned int FileOffset, char subname[]);

	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl)) {

		rank = popRankForDocIDMemArray(ReposetoryHeader.DocID);

		if (rank >= minrank) {
			//printf("DocId: %i url: %s res %hi htmls %hi time %lu, rank %i\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time,rank);
			printf("%s\n",ReposetoryHeader.url);
		}
		//printf("################################\n%s##############################\n",htmlbuffer);

	}
	
	
}
