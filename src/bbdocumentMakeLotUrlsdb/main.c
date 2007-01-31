#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include "../bbdocument/bbdocument.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>




main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl;
	int count;
	char fileName[512];

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	char *subname = argv[2];


	GetFilPathForLotFile(fileName,"urls.db",1,subname);

	printf("Working on \"%s\"\n",fileName);

	//skal bare slette hvis dette er førte lot
	if (LotNr == 1) {
		if (unlink(fileName) != 0) {
			perror(fileName);
		}
	}
	//loppergjenom alle
// int rGetNext (unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[],
// int htmlbufferSize, char imagebuffer[], unsigned long int *radress,
// unsigned int FilterTime, unsigned int FileOffset, char subname[]);
	count =0;
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl)) {

		#ifdef DEBUG
		printf("DocId: %i url: %s res %hi htmls %hi time %lu\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time);
		#endif

		uriindex_add (ReposetoryHeader.url, ReposetoryHeader.DocID, ReposetoryHeader.time, subname);

		//printf("################################\n%s##############################\n",htmlbuffer);
		++count;
	}

	printf("count %i\n",count);
	
	
}
