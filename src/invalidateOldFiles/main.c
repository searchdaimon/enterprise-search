#include <stdio.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"

/* Should probably get this from the crawler, decided depending on the recrawl time */
#define TIMEOUT		60*60*12*1

int
main(int argc, char **argv)
{
	int LotNr;
	docid DocId;
	char *subname;
	struct DocumentIndexFormat docindex;
	time_t timeout;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3)
                errx(1, "Usage: ./%s lotnr subname\n\n", argv[0]);

	LotNr = atoi(argv[1]);
	subname = argv[2];

	timeout = time(NULL) - TIMEOUT;
	while (DIGetNext(&docindex, LotNr, &DocId, subname)) {
		/* Skip if we don't have a resource set */
		if (docindex.ResourceSize == 0)
			continue;
		/* Invalidate? */
		if (docindex.lastSeen < timeout) {
			printf("Invalidating %u...\n", DocId);
			docindex.ResourcePointer = 0;
			docindex.ResourceSize = 0;
			DIWrite(&docindex, DocId, subname, NULL);
		}
	}

	return 0;
}
