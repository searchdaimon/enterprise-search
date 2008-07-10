
#include "../common/re.h"
#include "../common/define.h"


int main (int argc, char *argv[]) {
	struct reformat *redi;
	int i;
        int LotNr;
        unsigned int DocID;

        if (argc < 3) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

        LotNr = atoi(argv[1]);
        char *subname = argv[2];
        DocID = 0;


	if ((redi = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_READ_ONLY)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	for (i=0;i<NrofDocIDsInLot;i++) {
		if (REN_DocumentIndex(redi, i)->Url[0] == '\0') {
			continue;
		}
		printf("url: \"%s\"\n",REN_DocumentIndex(redi, i)->Url);
	}


	reclose(redi);
}
