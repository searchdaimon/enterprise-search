#include <unistd.h>

#include "../common/re.h"
#include "../common/define.h"
#include "../common/langToNr.h"


int main (int argc, char *argv[]) {
	struct reformat *redi;
	int i;
        int LotNr;
        unsigned int DocID;
	char *rank;


        if (argc < 3) {
               printf("This program reads a PopRank file.\n\n\tUsage: ./readPopRank 1 subname");
               exit(0);
        }



        LotNr = atoi(argv[1]);
        char *subname = argv[2];
        DocID = 0;


	if ((redi = reopen(LotNr, 4, "filtypes", subname, RE_READ_ONLY||RE_STARTS_AT_0)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	DocID = LotDocIDOfset(LotNr);

	for (i=0;i<NrofDocIDsInLot;i++) {

		rank = REN_Char(redi, i);

		if (rank[0] != '\0') {
			printf("DocID: %u, rank: %c%c%c%c\n", DocID, rank[0],rank[1],rank[2],rank[3]);
		}

		++DocID;


	}


	reclose(redi);
}
