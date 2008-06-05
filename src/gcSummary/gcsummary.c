#define __USE_LARGEFILE64 1
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>

#include "../common/gcsummary.h"


int
main(int argc, char **argv)
{
	int LotNr;
	char *subname;
	
	if (argc < 3)
		errx(1, "Usage: ./gcsummary lotnr subname");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	gcsummary(LotNr, subname);

	return 0;
}

