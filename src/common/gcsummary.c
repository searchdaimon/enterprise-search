#define __USE_LARGEFILE64 1
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>

#include "define.h"
#include "reposetory.h"
#include "lot.h"
#include "DocumentIndex.h"


int
gcsummary(int LotNr, char *subname)
{
	char path[1024];
	char path2[1024];
	struct DocumentIndexFormat docindex;
	unsigned int DocID, firstDocID;
	FILE *fh, *newfh;
	int keept = 0;
	int gced = 0;
	
	GetFilPathForLot(path, LotNr, subname);
	strcpy(path2, path);
	strcat(path, "summary");
	strcat(path2, "summary.old");
	if (rename(path, path2) != 0)
		err(1, "rename(summary, summary.old)");

	if ((fh = lotOpenFileNoCasheByLotNr(LotNr,"summary.old","r",'e',subname)) == NULL)
		err(1, "Unable to open summary file");

	if ((newfh = lotOpenFileNoCasheByLotNr(LotNr,"summary","w",'e',subname)) == NULL)
		err(1, "Unable to open summary wip file");

	fseeko64(fh, 0, SEEK_SET);
	if (fread(&firstDocID, sizeof(firstDocID), 1, fh) != 1) {
		warn("Unable to read first docid");
	}


	while (DIGetNext(&docindex, LotNr, &DocID, subname)) {
		char sumbuf[65536];
		unsigned int len, sDocID;

		len = docindex.SummarySize;
		if (docindex.SummaryPointer == NULL && DocID != firstDocID) {
			#ifdef DEBUG
				printf("skiping...\n");
			#endif
			++gced;
			continue;
		}
		if (fseeko64(fh, (off_t)docindex.SummaryPointer, SEEK_SET) == -1) {
			warn("Unable to seek to summary for %d", DocID);
			++gced;
			continue;
		}
		if (fread(&sDocID, sizeof(sDocID), 1, fh) != 1) {
			if (feof(fh))
				warnx("Hit eof while reading docid from summary");
			else
				warn("Unable to read docid from summary");

			++gced;
			continue;
		}
		if (sDocID != DocID && docindex.SummaryPointer != NULL) {
			warnx("Did not read the same DocID as was requested");
			++gced;
			continue;
		}
		if (fread(sumbuf, len, 1, fh) != 1) {
			warn("Unable to read summary");
			++gced;
			continue;
		}

		/* Write new summary */
		docindex.SummaryPointer = ftell(newfh);
		fwrite(&DocID, sizeof(DocID), 1, newfh);
		fwrite(sumbuf, docindex.SummarySize, 1, newfh);
		DIWrite(&docindex, DocID, subname, NULL);
		++keept;
	}

	#ifdef DI_FILE_CASHE
		closeDICache();
	#endif

	fclose(fh);
	fclose(newfh);
	unlink(path2);

	printf("gcsummary: keept %i\ngced %i\n",keept,gced);

	return 0;
}

