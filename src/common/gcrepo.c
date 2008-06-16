
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "define.h"
#include "reposetory.h"
#include "lot.h"
#include "DocumentIndex.h"

#define MaxAgeDiflastSeen 86400
//#define MaxAgeDiflastSeen 100

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
	char *url;
	unsigned long int raddress;
	char path[1024];
	char path2[1024];
	char path3[1024];
	FILE *GCEDFH;
	#ifdef BLACK_BOKS
		time_t newest_document;
	#endif
	FILE *DOCINDEXFH;

	int keept = 0;
	int gced = 0;

	struct DIArrayFormat *DIArray;
	int DocIDPlace;

        if ( (DOCINDEXFH = lotOpenFileNoCasheByLotNr(LotNr,"DocumentIndex","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a DocumentIndex file\n");
		#endif

                return 0;
        }
	fclose(DOCINDEXFH);

	if ((DIArray = malloc(sizeof(struct DIArrayFormat) * NrofDocIDsInLot)) == NULL) {
		perror("gcrepo: malloc DIArray");
		exit(1);
	}

	#ifdef BLACK_BOKS
		newest_document = 0;	
		printf("newest_document: %s",ctime(&newest_document));
	#endif


	for (i=0;i<NrofDocIDsInLot;i++) {
		DIArray[i].DocID = 0;
		DIArray[i].gced = 0;

		if (!DIRead(&DIArray[i].docindex, LotDocIDOfset(LotNr) +i, subname)) {
			//fprintf(stderr, "Unable to locate a DI for %d\n", LotDocIDOfset(LotNr) +i);
			continue;
		}

		#ifdef BLACK_BOKS
		if ((DIArray[i].docindex.lastSeen != 0) && (newest_document < DIArray[i].docindex.lastSeen)) {
                                newest_document = DIArray[i].docindex.lastSeen;
                }
		#endif

	}

	#ifdef BLACK_BOKS
	printf("newest_document: %s",ctime(&newest_document));
	#endif

	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&raddress,0,0,subname,&acl_allow,&acl_deny, &url)) {

		DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset(LotNr));
		DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;

		#ifdef DEBUG
		#ifdef BLACK_BOKS
		printf("dokument \"%s\", DocID %u. lastSeen: %s",
			DIArray[DocIDPlace].docindex.Url,
			ReposetoryHeader.DocID,
			ctime(&DIArray[DocIDPlace].docindex.lastSeen));
		#endif
		#endif

		//printf("%p\n", docindex.RepositoryPointer);
		if (raddress != DIArray[DocIDPlace].docindex.RepositoryPointer) {
			DIArray[DocIDPlace].gced = 1;
			#ifdef DEBUG
			printf("Garbage collecting %d at %u. docindex has %u\n", ReposetoryHeader.DocID, raddress,DIArray[DocIDPlace].docindex.RepositoryPointer);
			#endif
			++gced;
		}
		#ifdef BLACK_BOKS
		else if ((DIArray[DocIDPlace].docindex.lastSeen != 0) && (newest_document > (DIArray[DocIDPlace].docindex.lastSeen + MaxAgeDiflastSeen))) {
			DIArray[DocIDPlace].gced = 1;

			DIArray[DocIDPlace].docindex.htmlSize = 0;

			printf("dokument \"%s\" is deleted. lastSeen: %s",DIArray[DocIDPlace].docindex.Url,ctime(&DIArray[DocIDPlace].docindex.lastSeen));
			++gced;
		
		} 
		#endif
		else {
			unsigned long int offset;
			offset = rApendPost(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_deny, "repo.wip", url);
			DIArray[DocIDPlace].docindex.RepositoryPointer = offset;
			#ifdef DEBUG
			printf("Writing DocID: %d\n", ReposetoryHeader.DocID);
			#endif
			++keept;

		}
	}

	//lokker filen repo.wip
	lotCloseFiles();

	printf("keept %i\ngced %i\n",keept,gced);

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
	free(DIArray);

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

	#ifdef DI_FILE_CASHE
		closeDICache();
	#endif


	return 0;
}

