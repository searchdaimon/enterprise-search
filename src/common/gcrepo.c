#include <string.h>
#include <stdio.h>
#include <err.h>

#include "define.h"
#include "re.h"
#include "reposetory.h"
#include "lot.h"
#include "DocumentIndex.h"


#ifdef BLACK_BOX


#define MaxAgeDiflastSeen 86400

int gcrepo(int LotNr, char *subname) {

	struct ReposetoryHeaderFormat ReposetoryHeader;

	Bytef htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;
	char *url, *attributes;
	unsigned int raddress;
	char path[1024];
	char path2[1024];
	char path3[1024];
	FILE *FNREPO;
	struct reformat *re;

	int keept = 0;
	int gced = 0;

	container *attrkeys = ropen();


	if((re = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_HAVE_4_BYTES_VERSION_PREFIX|RE_COPYONCLOSE)) == NULL) {
		perror("reopen DocumentIndex");
		return 0;
	}


        if ( (FNREPO = lotOpenFileNoCasheByLotNr(LotNr,"reposetory","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a reposetory file\n");
		#endif
                return 0;
        }


	while (rGetNext_fh(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&raddress,0,&acl_allow,&acl_deny, FNREPO ,&url, &attributes)) {


		#ifdef DEBUG
			printf("dokument \"%s\", DocID %u.\n",
			RE_DocumentIndex(re,ReposetoryHeader.DocID)->Url,
			ReposetoryHeader.DocID);
		#endif


		if (raddress != RE_DocumentIndex(re,ReposetoryHeader.DocID)->RepositoryPointer) {
			#ifdef DEBUG
			printf("Garbage collecting %d at %u. docindex has %u\n", ReposetoryHeader.DocID, raddress,RE_DocumentIndex(re,ReposetoryHeader.DocID)->RepositoryPointer);
			#endif
			++gced;
		}
		else {
			unsigned int offset;
			offset = rApendPost(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_deny, "repo.wip", url, attributes, attrkeys);
			RE_DocumentIndex(re,ReposetoryHeader.DocID)->RepositoryPointer = offset;
			#ifdef DEBUG
				printf("Writing DocID: %d\n", ReposetoryHeader.DocID);
			#endif
			++keept;

		}
	}
	fclose(FNREPO);

	//lukker filen repo.wip
	rclose(attrkeys);

	printf("keept %i\ngced %i\n",keept,gced);

	reclose(re);


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

#endif
