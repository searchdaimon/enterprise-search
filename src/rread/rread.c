#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"

#include <stdlib.h>
#include <string.h>





main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char htmlbuffer[524288];
	char htmlbuffer_uncom[524288];
	char imagebuffer[524288];
	char *acl_allow;
	char *acl_deny;

	int StatisticsUncompressError = 0;;
	int StatisticsUncompressOk = 0;

	char uncompresshtml[50000];
	int uncompresshtmlLength;
	int nerror;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	int optPrintHtml = 0;
	int optStatistics = 0;
        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"hs"))!=-1) {
                switch (c) {
                        case 'h':
                                optPrintHtml = 1;
                                printf("will print html\n");
                                break;
                        case 's':
                                optStatistics  = 1;
                                printf("will statistics\n");
                                break;
                        default:
                                exit(1);
                }

        }
        --optind;


	LotNr = atoi(argv[1 + optind]);
	char *subname = argv[2 + optind];

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);


	//loppergjenom alle
	while (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,sizeof(htmlbuffer),imagebuffer,&radress,0,0,subname,&acl_allow,&acl_deny)) {

		printf("DocId: %i url: %s res %hi htmls %hi time %lu\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time);
		uncompresshtmlLength = sizeof(uncompresshtml);
		if ( (nerror = uncompress((Bytef*)uncompresshtml,(uLong *)&uncompresshtmlLength,(Bytef*)htmlbuffer,ReposetoryHeader.htmlSize)) != 0) {
                	printf("uncompress error. Code: %i\n",nerror);
                
                        continue;
                }

		if (optPrintHtml || optStatistics) {
			if (runpack(htmlbuffer_uncom,sizeof(htmlbuffer_uncom),htmlbuffer,ReposetoryHeader.htmlSize)) {
				if (optPrintHtml) {
					printf("################################\n%s##############################\n",htmlbuffer_uncom);
				}
				++StatisticsUncompressOk;
			}
			else {
				if (optPrintHtml) {
					printf("rread: can't uncompress\n");
				}
				++StatisticsUncompressError;
			}
		}

	}
	
	if (optStatistics) {
		printf("StatisticsUncompressOk %i\nStatisticsUncompressError %i\n",StatisticsUncompressOk,StatisticsUncompressError);
	}
}
