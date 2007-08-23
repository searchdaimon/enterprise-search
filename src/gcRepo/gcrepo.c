
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/DocumentIndex.h"



//int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]) {







int
main(int argc, char **argv)
{
	struct ReposetoryHeaderFormat ReposetoryHeader;
	int LotNr;
	char *subname;
	char lotPath[255];
	char htmlbuffer[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;
	unsigned long int raddress;
	char path[1024];
	char path2[1024];
	char path3[1024];
	
	if (argc < 3)
		errx(1, "Usage: ./gcrepo lotnr subname");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);


	//loppergjenom alle
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&raddress,0,0,subname,&acl_allow,&acl_deny)) {
		struct DocumentIndexFormat docindex;

		if (!DIRead(&docindex, ReposetoryHeader.DocID, subname)) {
			fprintf(stderr, "Unable to locate a DI for %d\n", ReposetoryHeader.DocID);
			continue;
		}

		//printf("%p\n", docindex.RepositoryPointer);
		if (raddress == docindex.RepositoryPointer) {
			unsigned long int offset;
			offset = rApendPost(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_deny, "repo.wip");
			docindex.RepositoryPointer = offset;
			DIWrite(&docindex, ReposetoryHeader.DocID, subname, "DocumentIndex.wip");
			printf("Writing DocID: %d\n", ReposetoryHeader.DocID);
		} else {
			printf("Garbage collecting %d at %lx\n", ReposetoryHeader.DocID, raddress);
		}
	}

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

	return 0;
}

