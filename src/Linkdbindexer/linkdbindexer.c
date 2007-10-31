#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "../common/define.h"
#include "../BrankCalculate5/defines.h"

extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence) __THROW;
extern __off64_t ftello64 (FILE *__stream) __THROW;


FILE *f_index;

void
write_index(docid DocID, off_t offset)
{
	//printf("docid: %u with offset: %Li, %u\n", DocID, offset, DocID * sizeof(offset));
	if (fseeko64(f_index, DocID * sizeof(offset), SEEK_SET) == -1) {
		warn("fseek(f_index): %u: %u", DocID, DocID * sizeof(offset));
		return;
	}
	if (fwrite(&offset, sizeof(offset), 1, f_index) != 1)
		warn("fwrite(offset)");
}

int
main(int argc, char **argv)
{
	char *linkdb, *indexfile;
	FILE *f_linkdb;
	struct ExpandedLinkdbFormat ldb[1];
	int n_read;
	docid last;
	off_t offset;
	docid done;
	int fd;

	printf("off_t size: %d\n", sizeof(off_t));
	printf("long int size: %d\n", sizeof(long int));

	if (argc < 3)
		errx(1, "Usage: %s linkdb indexfile", argv[0]);

	linkdb = argv[1];
	indexfile = argv[2];

	if ((f_linkdb = fopen(linkdb, "r")) == NULL)
		err(1, "Unable to open linkdb file: %s", linkdb);
	if ((f_index = fopen(indexfile, "w")) == NULL)
		err(1, "Unable to open index file: %s", indexfile);

	last = 0;
	done = 0;
	while (n_read > 0 || (n_read = fread(ldb, sizeof(struct ExpandedLinkdbFormat), 1, f_linkdb)) > 0) {
		struct ExpandedLinkdbFormat *db = &ldb[--n_read];

		if (db->DocID_to == last) {
			continue;
		}
		last = db->DocID_to;
		offset = ftello64(f_linkdb) - sizeof(struct ExpandedLinkdbFormat);
		write_index(db->DocID_to, offset);
		done++;
		if ((done % 1000000) == 0)
			printf("Done with %u docids\n", done);
	}

	fclose(f_index);
	fclose(f_linkdb);

	return 0;
}
