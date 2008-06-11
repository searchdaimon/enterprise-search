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

struct dFormat {
	struct DocumentIndexFormat docindex;
	unsigned int DocID;
};


int compare_d_SummaryPointer (const void *p1, const void *p2) {


        if (((struct dFormat *)p1)->docindex.SummaryPointer < ((struct dFormat *)p2)->docindex.SummaryPointer)
                return -1;
        else
                return ((struct dFormat *)p1)->docindex.SummaryPointer > ((struct dFormat *)p2)->docindex.SummaryPointer;

}


int
gcsummary(int LotNr, char *subname)
{
	char path[1024];
	char path_old[1024];
	struct DocumentIndexFormat docindex;
	unsigned int DocID, firstDocID;
	FILE *fh, *newfh;
	int keept = 0;
	int gced = 0;
	int i;

	struct dFormat *d;

	if ((d = malloc(sizeof(struct dFormat) * NrofDocIDsInLot)) == NULL) {
		perror("malloc dFormat");
		return 0;
	}
	
	GetFilPathForLot(path, LotNr, subname);
	strcpy(path_old, path);
	strcat(path, "summary.gcSummary");
	strcat(path_old, "summary");

	if ((fh = lotOpenFileNoCasheByLotNr(LotNr,"summary","r",'e',subname)) == NULL)
		err(1, "Unable to open summary file");

	if ((newfh = lotOpenFileNoCasheByLotNr(LotNr,"summary.gcSummary","w",'e',subname)) == NULL)
		err(1, "Unable to open summary wip file");

	fseeko64(fh, 0, SEEK_SET);
	if (fread(&firstDocID, sizeof(firstDocID), 1, fh) != 1) {
		warn("Unable to read first docid");
	}

	//inialiserer
	//for(i=0;i<NrofDocIDsInLot;i++) {
	//	d[i].DocID = 0;
	//}

	i = 0;
	while (DIGetNext(&docindex, LotNr, &DocID, subname)) {
		d[i].DocID = DocID;
		d[i].docindex = docindex;
		++i;
	}

	//sorterer på SummaryPointer for å få å kunne lese så sekvensielt som mulig.
	qsort(d, NrofDocIDsInLot, sizeof(struct dFormat), compare_d_SummaryPointer);

	//går igjenom alle
	for(i=0;i<NrofDocIDsInLot;i++) {
	
		char sumbuf[65536];
		unsigned int len, sDocID;


		if(d[i].docindex.SummarySize == 0) {
			//++gced;
			continue;
		}

		#ifdef DEBUG
			printf("SummaryPointer %i\n",(int)d[i].docindex.SummaryPointer);
		#endif

		len = d[i].docindex.SummarySize;
		if ((char *)d[i].docindex.SummaryPointer == NULL && d[i].DocID != firstDocID) {

			#ifdef DEBUG
				printf("skiping...\n");
			#endif
			++gced;
			continue;
		}
		if (fseeko64(fh, (off_t)d[i].docindex.SummaryPointer, SEEK_SET) == -1) {
			warn("Unable to seek to summary for %d", d[i].DocID);
			++gced;
			continue;
		}
		if (fread(&sDocID, sizeof(sDocID), 1, fh) != 1) {
			if (feof(fh)) {
				warnx("Hit eof while reading docid from summary");
				break;
			}
			else {
				warn("Unable to read docid from summary");
			}
			++gced;
			continue;
		}
<<<<<<< gcsummary.c
		if (sDocID != d[i].DocID && d[i].docindex.SummaryPointer != NULL) {
			#ifdef DEBUG
				warnx("Did not read the same DocID as was requested");
			#endif
=======
		if (sDocID != DocID && (char *)docindex.SummaryPointer != NULL) {
			warnx("Did not read the same DocID as was requested");
>>>>>>> 1.2
			++gced;
			continue;
		}
		if (fread(sumbuf, len, 1, fh) != 1) {
			warn("Unable to fread from summary");
			++gced;
			continue;
		}

		/* Write new summary */
		d[i].docindex.SummaryPointer = ftell(newfh);
		fwrite(&d[i].DocID, sizeof(d[i].DocID), 1, newfh);
		fwrite(sumbuf, d[i].docindex.SummarySize, 1, newfh);
		DIWrite(&d[i].docindex, d[i].DocID, subname, NULL);
		++keept;
	}

	#ifdef DI_FILE_CASHE
		closeDICache();
	#endif

	fclose(fh);
	fclose(newfh);
	unlink(path_old);

	if (rename(path, path_old) != 0)
		err(1, "rename(summary.gcSummary, summary)");

	printf("gcsummary: keept %i\ngced %i\n",keept,gced);

	free(d);

	return 0;
}

