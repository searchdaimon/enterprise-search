#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "../common/gcrepo.h"

int
main(int argc, char **argv)
{

	int LotNr;
	char *subname;

	
	if (argc < 3)
		errx(1, "Usage: ./gcrepo lotnr subname");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	gcrepo(LotNr,subname);


	return 0;
}

