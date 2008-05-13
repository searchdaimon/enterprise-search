#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "../common/gcrepo.h"
#include "../common/define.h"

int
main(int argc, char **argv)
{

	int LotNr;
	char *subname;

	
	if (argc < 2)
		errx(1, "Usage: ./gcrepo subname [ lotnr ]");

	subname = argv[1];


	if (argc == 3) {
		LotNr = atoi(argv[2]);
		gcrepo(LotNr,subname);
	}
	else {
		for(LotNr=1;LotNr<maxLots;LotNr++) {
			gcrepo(LotNr,subname);
		}
	}

	return 0;
}

