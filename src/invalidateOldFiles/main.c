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
#define TIMEOUT		60*60*24*14 // Two weeks

int
main(int argc, char **argv)
{
	int LotNr;
	docid DocId;
	char *subname;
	struct DocumentIndexFormat docindex;
	time_t timeout;
	int dry;

        //tester for at vi har fåt hvilken lot vi skal bruke
        if (argc < 3)
                errx(1, "Usage: ./%s lotnr subname [dry]", argv[0]);

	if (argc > 3)
		dry = 1;
	else
		dry = 0;

	LotNr = atoi(argv[1]);
	subname = argv[2];

	timeout = time(NULL) - TIMEOUT;
	if (dry)
		printf("Doing a dry run, won't write updated document indexes.\n");

	while (DIGetNext(&docindex, LotNr, &DocId, subname)) {
		if (docindex.htmlSize == 0)
			continue;
		/* Invalidate? */
		if (docindex.lastSeen < timeout) {
			printf("Invalidating %u...\n", DocId);
			docindex.RepositoryPointer = 0;
			docindex.htmlSize = 0;
			if (!dry) {
				DIWrite(&docindex, DocId, subname, NULL);
			}
		}
	}

	return 0;
}
