#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/DocumentIndex.h"

#include <stdlib.h>
#include <string.h>




int
main(int argc, char **argv) {
	int LotNr;
	char lotPath[255];
	struct DocumentIndexFormat docindex;
	unsigned int DocID;
	char text[40];
	unsigned int radress;

	if (argc < 3)
		errx(1, "Usage: ./DIread lotNr subname");

#if 0
	if (argc > 3)
		anchoraddnew(125500001, "eirik", 5, "www", NULL);
#endif

	LotNr = atoi(argv[1]);
	char *subname = argv[2];

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);


#if 1
	//loppergjenom alle
	while (DIGetNext(&docindex, LotNr, &DocID, subname)) {
		printf("DocID: %d\n", DocID);
		if (anchorRead(LotNr, subname, DocID, text, sizeof(text)))
			printf("\tAnchor text: '%s'\n", text);
	}
#endif

	
	return 0;
}
