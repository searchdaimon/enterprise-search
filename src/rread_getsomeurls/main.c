#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define FilePath "/tmp/frames_udfile"

main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char subname[maxSubnameLength];

	struct udfileFormat udfilepost;

	FILE *FH;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	strncpy(subname,argv[2],sizeof(subname) -1);

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);

	if ((FH = fopen(FilePath,"ab")) == NULL) {
		perror("tmpe file");
		exit(1);
	}	

	//loppergjenom alle
	int htmlbufferSize = sizeof(htmlbuffer);
	int count = 0;
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,htmlbufferSize,imagebuffer,&radress,0,0,subname)) {

		if ((ReposetoryHeader.response == 200) && (ReposetoryHeader.imageSize == 0) && (ReposetoryHeader.htmlSize != 0)) {
			//printf("DocId: %i url: %s res %hi htmls %hi time %lu\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time);
	
			//printf("################################\n%s##############################\n",htmlbuffer);

			strncpy(udfilepost.url,ReposetoryHeader.url,sizeof(udfilepost.url));
			udfilepost.DocID = ReposetoryHeader.DocID;

			fwrite(&udfilepost,sizeof(udfilepost),1,FH);
		}
		++count;
	}
	printf("Did analyse %i pages\n",count);

	fclose(FH);	
}

