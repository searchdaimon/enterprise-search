
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
	char *url, *attributes;
	unsigned long int raddress;
	char path[1024];
	char path2[1024];
	char path3[1024];
	FILE *DOCINDEXFH, *FNREPO;

	int keept = 0;
	int gced = 0;

	container *attrkeys = ropen();

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



	for (i=0;i<NrofDocIDsInLot;i++) {
		DIArray[i].DocID = 0;
		DIArray[i].gced = 0;

		if (!DIRead(&DIArray[i].docindex, LotDocIDOfset(LotNr) +i, subname)) {
			//fprintf(stderr, "Unable to locate a DI for %d\n", LotDocIDOfset(LotNr) +i);
			continue;
		}


	}


        if ( (FNREPO = lotOpenFileNoCasheByLotNr(LotNr,"reposetory","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a reposetory file\n");
		#endif

                return 0;
        }


	while (rGetNext_fh(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&raddress,0,0,subname,&acl_allow,&acl_deny, FNREPO ,&url, &attributes)) {

		DocIDPlace = (ReposetoryHeader.DocID - LotDocIDOfset(LotNr));
		DIArray[DocIDPlace].DocID = ReposetoryHeader.DocID;

		#ifdef DEBUG
		printf("dokument \"%s\", DocID %u.\n",
			DIArray[DocIDPlace].docindex.Url,
			ReposetoryHeader.DocID);
		#endif

		//printf("%p\n", docindex.RepositoryPointer);
		if (raddress != DIArray[DocIDPlace].docindex.RepositoryPointer) {
			DIArray[DocIDPlace].gced = 1;
			#ifdef DEBUG
			printf("Garbage collecting %d at %u. docindex has %u\n", ReposetoryHeader.DocID, raddress,DIArray[DocIDPlace].docindex.RepositoryPointer);
			#endif
			++gced;
		}
		else {
			unsigned long int offset;
			offset = rApendPost(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_deny, "repo.wip", url,
			    attributes, attrkeys);
			DIArray[DocIDPlace].docindex.RepositoryPointer = offset;
			#ifdef DEBUG
			printf("Writing DocID: %d\n", ReposetoryHeader.DocID);
			#endif
			++keept;

		}
	}
	fclose(FNREPO);

	//lokker filen repo.wip
	//lotCloseFiles();
	rclose(attrkeys);

	printf("keept %i\ngced %i\n",keept,gced);

	printf("writing to DI..\n");
	for (i=0;i<NrofDocIDsInLot;i++) {

		if (DIArray[i].DocID != 0) {
			DIWrite(&DIArray[i].docindex, DIArray[i].DocID, subname, "DocumentIndex.wip");
		}
	}
	printf("..done\n");

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

