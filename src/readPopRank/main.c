#include <unistd.h>

#include "../common/re.h"
#include "../common/define.h"
#include "../common/langToNr.h"


int main (int argc, char *argv[]) {
	struct reformat *redi;
	int i;
        int LotNr;
        unsigned int DocID;
	unsigned int rank;


        if (argc < 3) {
               printf("This program reads a PopRank file.\n\n\tUsage: ./readPopRank 1 subname");
               exit(0);
        }



        LotNr = atoi(argv[1]);
        char *subname = argv[2];
        DocID = 0;


	if ((redi = reopen(LotNr, sizeof(unsigned char), "PopRank", subname, RE_READ_ONLY)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	DocID = LotDocIDOfset(LotNr);

	for (i=0;i<NrofDocIDsInLot;i++) {

		rank = *REN_Uchar(redi, i);

		if (rank != 0) {
			printf("DocID: %u, rank: %d\n", DocID, rank);
		}

		++DocID;


	}


	reclose(redi);
}
