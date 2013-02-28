#include <unistd.h>

#include "../common/re.h"
#include "../common/define.h"
#include "../common/langToNr.h"
#include "../common/stdlib.h"
#include "../common/url.h"


int main (int argc, char *argv[]) {
	struct reformat *redi;
	int i;
        int LotNr;
        unsigned int DocID;

	char *optLang 	= NULL;
	char *optTtl 	= NULL;
	unsigned short optVerbos = 0;
	unsigned short optResponse = 0;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"l:r:t:v"))!=-1) {
                switch (c) {
			case 'v':
				optVerbos = 1;
				break;
                        case 'r':
				optResponse = atoi(optarg);
				break;
                        case 't':
				optTtl = optarg;
				break;
                        case 'l':
                                optLang = bitoa(getLangNr(optarg));
                                break;
                        default:
                                exit(1);
                }

        }
        --optind;


        if (argc -optind < 3) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }



        LotNr = atoi(argv[1 +optind]);
        char *subname = argv[2 +optind];
        DocID = 0;

	printf("args: Lang %i, Response %hu\n",optLang,optResponse);

	if ((redi = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_READ_ONLY)) == NULL) {
		perror("reopen");
		exit(-1);
	}

	for (i=0;i<NrofDocIDsInLot;i++) {
		if (REN_DocumentIndex(redi, i)->Url[0] == '\0') {
			continue;
		}

		if ((optResponse != 0) && (optResponse != REN_DocumentIndex(redi, i)->response)) {
			continue;
		}
		else if ((optLang != NULL) && (strcmp(optLang,REN_DocumentIndex(redi, i)->Sprok) != 0)) {
			continue;
		}
		else if ((optTtl != NULL) && (!url_isttl(REN_DocumentIndex(redi, i)->Url ,optTtl))) {
			continue;
		}
		else {

			if (optVerbos == 1) {
				printf("url: \"%s\", CrawleDato %u, lang \"%s\", response %hu\n",REN_DocumentIndex(redi, i)->Url, REN_DocumentIndex(redi, i)->CrawleDato, REN_DocumentIndex(redi, i)->Sprok,REN_DocumentIndex(redi, i)->response);
			}
			else {
				printf("%s\n",REN_DocumentIndex(redi, i)->Url );
			}
		}
	}


	reclose(redi);
}
