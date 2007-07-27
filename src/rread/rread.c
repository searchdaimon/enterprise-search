#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include <stdlib.h>
#include <string.h>





main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;

	char uncompresshtml[50000];
	int uncompresshtmlLength;
	int nerror;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	char *subname = argv[2];

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);


	//loppergjenom alle
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl_allow,&acl_deny)) {

		printf("DocId: %i url: %s res %hi htmls %hi time %lu\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time);
		uncompresshtmlLength = sizeof(uncompresshtml);
		if ( (nerror = uncompress((Bytef*)uncompresshtml,(uLong *)&uncompresshtmlLength,(Bytef*)htmlbuffer,ReposetoryHeader.htmlSize)) != 0) {
                	printf("uncompress error. Code: %i\n",nerror);
                
                        continue;
                }

		printf("################################\n%s##############################\n",uncompresshtml);

	}
	
	
}
