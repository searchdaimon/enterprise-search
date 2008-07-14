#include <stdlib.h>
#include <string.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/ir.h"
#include "../common/revindex.h"
#include "../common/url.h"
#include "../common/integerindex.h"
#include "../common/ir.h"

#define subname "www"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int lotNr;
	unsigned int DocID;
	char domain[650];
	FILE *DIFH;

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	lotNr = atoi(argv[1]);
	DocID = 0;

	unsigned short int DomainDI;

	struct iintegerFormat iinteger;

	if ((DIFH = lotOpenFileNoCasheByLotNr(lotNr,"DocumentIndex","r", 's', subname)) == NULL) {
		printf("don't have a DocumentIndex\n");
		exit(1);
	}

	fclose(DIFH);


	if (!iintegerOpenForLot(&iinteger,"domainid",lotNr, "w", subname)) {
		perror("iintegerOpenForLot");
		exit(1);
	}

	while (DIGetNext (&DocumentIndexPost,lotNr,&DocID,subname)) {


		if (DocumentIndexPost.Url[0] == '\0') {

		}
		else if (strncmp(DocumentIndexPost.Url,"http://",7) != 0) {
			//printf("no http: %s\n",DocumentIndexPost.Url);
		}
		else if (!find_domain_no_subname2(DocumentIndexPost.Url,domain,sizeof(domain))) {
		//else if (!find_domain(DocumentIndexPost.Url,domain,sizeof(domain))) {
			#ifdef DEBUG
			printf("!find_domain %s\n",DocumentIndexPost.Url);
			#endif
		}
		else {
			//printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);



			DomainDI = calcDomainID(domain);


			#ifdef DEBUG
			printf("url: \"%s\", DocID %u, Domain %s, DomainDI %ho\n",DocumentIndexPost.Url,DocID,domain,DomainDI);
			#endif

			iintegerSetValue(&iinteger,&DomainDI,sizeof(DomainDI),DocID,subname);



			

			
		}
	}


	//DIClose();

	iintegerClose(&iinteger);

	return 1;
}

