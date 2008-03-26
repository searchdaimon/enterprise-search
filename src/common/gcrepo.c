
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "define.h"
#include "reposetory.h"
#include "lot.h"
#include "DocumentIndex.h"

struct DIArrayFormat {
        struct DocumentIndexFormat docindex;
        unsigned int DocID;
	char gced;
};

int
gcrepo(int LotNr, char *subname)
{
	int i;
	struct ReposetoryHeaderFormat ReposetoryHeader;

	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;
	unsigned long int raddress;
	char path[1024];
	char path2[1024];
	char path3[1024];
	FILE *GCEDFH;

	int keept = 0;
	int gced = 0;

	struct DIArrayFormat *DIArray;
	int DocIDPlace;

	DIArray = malloc(sizeof(struct DIArrayFormat) * NrofDocIDsInLot);
	

	for (i=0;i<NrofDocIDsInLot;i++) {
		DIArray[i].DocID = 0;
		DIArray[i].gced = 0;
	}

	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&raddress,0,0,subname,&acl_allow,&acl_deny)) {

		DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset(LotNr));
		DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;

		if (!DIRead(&DIArray[DocIDPlace].docindex, ReposetoryHeader.DocID, subname)) {
			fprintf(stderr, "Unable to locate a DI for %d\n", ReposetoryHeader.DocID);
			continue;
		}

		//printf("%p\n", docindex.RepositoryPointer);
		if (raddress == DIArray[DocIDPlace].docindex.RepositoryPointer) {
			unsigned long int offset;
			offset = rApendPost(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_deny, "repo.wip");
			DIArray[DocIDPlace].docindex.RepositoryPointer = offset;
			#ifdef DEBUG
			printf("Writing DocID: %d\n", ReposetoryHeader.DocID);
			#endif
			++keept;
		} else {
			DIArray[DocIDPlace].gced = 1;
			#ifdef DEBUG
			printf("Garbage collecting %d at %lx\n", ReposetoryHeader.DocID, raddress);
			#endif
			++gced;
		}
	}

	printf("writing to DI..\n");
	for (i=0;i<NrofDocIDsInLot;i++) {
		if (DIArray[i].DocID != 0) {
			DIWrite(&DIArray[i].docindex, DIArray[i].DocID, subname, "DocumentIndex.wip");
		}
	}
	printf("..done\n");

	//lagrer hvilkene filer vi har slettet
	GCEDFH =  lotOpenFileNoCasheByLotNr(LotNr,"gced","a", 'e',subname);

	for (i=0;i<NrofDocIDsInLot;i++) {
		if ((DIArray[i].DocID != 0) && (DIArray[i].gced)) {
			if (fwrite(&DIArray[i].DocID,sizeof(DIArray[i].DocID),1,GCEDFH) != 1) {
				perror("can't write gc file");
			}
		}
	}

	fclose(GCEDFH);

	/* And we have a race... */
	GetFilPathForLot(path, LotNr, subname);
	strcpy(path2, path);
	strcpy(path3, path);
	strcat(path, "repo.wip");
	strcat(path2, "reposetory");
	rename(path, path2);
	strcpy(path, path3);
	strcat(path, "DocumentIndex.wip");
	strcat(path3, "DocumentIndex");
	rename(path, path3);

	printf("keept %i\ngced %i\n",keept,gced);

	return 0;
}

