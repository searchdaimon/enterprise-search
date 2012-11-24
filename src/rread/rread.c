#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include <stdlib.h>
#include <string.h>

void dolot(int LotNr, char *subname, int optPrintHtml, int optPrintAcl, int optStatistics, char *optReponame);

main (int argc, char *argv[]) {

	int LotNr, i;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	int optPrintHtml = 0;
	int optPrintAcl = 0;
	int optStatistics = 0;
	char *optReponame = NULL;
        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"hsr:a"))!=-1) {
                switch (c) {
                        case 'h':
                                optPrintHtml = 1;
                                break;
                        case 'a':
                                optPrintAcl = 1;
                                break;
                        case 's':
                                optStatistics  = 1;
                                break;
                        case 'r':
				optReponame = optarg;
				break;
                        default:
                                exit(1);
                }

        }
        --optind;


	LotNr = atoi(argv[1 + optind]);
	char *subname = argv[2 + optind];

	if (LotNr == 0) {
		for (i=0;i<maxLots;i++) {
			dolot(i, 	subname, optPrintHtml, optPrintAcl, optStatistics, optReponame);
		}
	}
	else {
			dolot(LotNr, 	subname, optPrintHtml, optPrintAcl, optStatistics, optReponame);
	}
}

void dolot(int LotNr, char *subname, int optPrintHtml, int optPrintAcl, int optStatistics, char *optReponame) {



	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char htmlbuffer[524288];
	char htmlbuffer_uncom[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;
	char *url, *attributes;

	int StatisticsUncompressError = 0;;
	int StatisticsUncompressOk = 0;

	char uncompresshtml[500000];
	int uncompresshtmlLength;
	int nerror;

	if (optReponame == NULL) {
		optReponame = strdup("reposetory");
	}


	GetFilPathForLot(lotPath,LotNr,subname);


	//loppergjenom alle
	while (rGetNext_reponame(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl_allow,&acl_deny,optReponame, &url, &attributes)) {


		printf("DocId: %i, url: %s, res: %hi, htmlsize: %hi, time: %lu, radress: %lu",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize2,ReposetoryHeader.time,radress);
		uncompresshtmlLength = sizeof(uncompresshtml);
		if ( (nerror = uncompress((Bytef*)uncompresshtml,(uLong *)&uncompresshtmlLength,(Bytef*)htmlbuffer,ReposetoryHeader.htmlSize2)) != 0) {
                	printf("uncompress error. Code: %i\n",nerror);
                
                        continue;
                }


		if (optPrintHtml || optStatistics || optPrintAcl) {
			if (runpack(htmlbuffer_uncom,sizeof(htmlbuffer_uncom),htmlbuffer,ReposetoryHeader.htmlSize2)) {

				if (optPrintAcl && !optPrintHtml) {
					printf(", acl_allow: \"%s\"",acl_allow);
					printf(", acl_deny: \"%s\"",acl_deny);
				}

				if (optPrintHtml) {
					printf("################################\n%s##############################\n",htmlbuffer_uncom);
					printf("acl_allow: \"%s\"\n",acl_allow);
					printf("acl_deny: \"%s\"\n",acl_deny);

				}


				++StatisticsUncompressOk;
			}
			else {
				if (optPrintHtml) {
					printf("rread: can't uncompress\n");
				}
				++StatisticsUncompressError;
			}

			if (optPrintHtml) {
				printf("Attributes: %s\n", attributes);
			}
		}

		printf("\n");

	}
	
	if (optStatistics) {
		printf("StatisticsUncompressOk %i\nStatisticsUncompressError %i\n",StatisticsUncompressOk,StatisticsUncompressError);
	}
}
