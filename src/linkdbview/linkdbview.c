#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "../common/define.h"
#include "../BrankCalculate5/defines.h"

extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence) __THROW;
extern __off64_t ftello64 (FILE *__stream) __THROW;


FILE *f_index;
FILE *f_linkdb;

off_t
read_index(docid DocID)
{
	off_t offset;

	if (fseeko64(f_index, DocID * sizeof(offset), SEEK_SET) == -1) {
		warn("fseek(f_index): %u: %u", DocID, DocID * sizeof(offset));
		return 0;
	}
	if (fread(&offset, sizeof(offset), 1, f_index) != 1) {
		warn("fread(offset)");
		return 0;
	}

	return offset;
}


inline ranktype brankGet(unsigned int);

void
doStatus(docid DocID)
{
	struct ExpandedLinkdbFormat ldb[1];
	int n_read;
	off_t offset;

	offset = read_index(DocID);
	if (offset == 0) {
		warnx("Did not find any offset for %u", DocID);
		return;
	}

	fseeko64(f_linkdb, offset, SEEK_SET);
	printf("Links from %u:\n", DocID);
	printf("%-15s %-12s %-20s %-20s %-20s\n", "From docid:", "From rank:", "From ip:", "To ip:", "Num out links from:");
	printf("------------------------------------------------------------------------------------------\n");
	while ((n_read = fread(ldb, sizeof(struct ExpandedLinkdbFormat), 1, f_linkdb)) > 0) {
		struct ExpandedLinkdbFormat *db = &ldb[0];
		struct in_addr in1, in2;
		char buf1[1024], buf2[1024];
		float rank;

		if (db->DocID_to != DocID) {
			break;
		}

		in1.s_addr = db->IPAddress_from;
		strcpy(buf1, inet_ntoa(in1));
		in2.s_addr = db->IPAddress_to;
		strcpy(buf2, inet_ntoa(in2));
		rank = brankGet(db->DocID_from);

		printf("%-15u %-12f %-20s %-20s %-20d\n", db->DocID_from, rank, buf1, buf2, db->nrOfOutLinks_from);
	}


}

void brankInit (int);

int
main(int argc, char **argv)
{
	char *linkdb, *indexfile;
	int i;
	docid endlot, enddocid;

	assert(sizeof(off_t) == 8);

	if (argc < 5)
		errx(1, "Usage: %s linkdb indexfile endlot docid ...", argv[0]);

	linkdb = argv[1];
	indexfile = argv[2];
	endlot = atoi(argv[3]);
	enddocid = endlot * (NrofDocIDsInLot * endlot);

	brankInit(endlot);

	if ((f_linkdb = fopen(linkdb, "r")) == NULL)
		err(1, "Unable to open linkdb file: %s", linkdb);
	if ((f_index = fopen(indexfile, "r")) == NULL)
		err(1, "Unable to open index file: %s", indexfile);

	for (i = 4; i < argc; i++)
		doStatus(atoi(argv[i]));
	fclose(f_index);
	fclose(f_linkdb);

	return 0;
}
