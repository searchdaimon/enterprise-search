#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "../common/iindex.h"
#include "../searchFilters/searchFilters.h"

/*****************************************************************
Finner gyldige sider, ved å se på DocumentIndex, slik at vi bare 
har de gyldige sidene med i indexksen. 

Dette brukes for å hindre at anker tekst indeksen blir full av 
sider som ikke kan vises.
*****************************************************************/
char *getValidDocIDs(int LotNr, char subname[]) {

	struct DocumentIndexFormat DocumentIndexPost;
        unsigned int DocID;
	int i;
	char *ret;
	int good;

	if ((ret = malloc(sizeof(char) * NrofDocIDsInLot)) == NULL) {
		perror("malloc ret");
		exit(-1);
	}

	i = 0;
	good = 0;
        while (DIGetNext (&DocumentIndexPost,LotNr,&DocID,subname)) {
		#ifdef DEBUG
                	printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);
		#endif

		//printf("%i == %i\n",i,(DocID - LotDocIDOfset(LotNr)));

		ret[i] = 0;

		if (DocumentIndexPost.Url[0] == '\0') {
			#ifdef DEBUG
				printf("url is emty\n");
			#endif
		}
		#ifndef BLACK_BOKS
		else if (filterResponse(DocumentIndexPost.response)) {
			#ifdef DEBUG
			printf("bad respons code %i\n",(int)DocumentIndexPost.response);
			#endif
		}
		#endif
		else {
			ret[i] = 1;	
			++good;
		}

		++i;
        }

	printf("have %i good urls.\n",good);

	return ret;
}

int main (int argc, char *argv[]) {


	int lotNr;
	int lotPart;
//	char iipath[256];
	unsigned lastIndexTime;
	struct IndekserOptFormat IndekserOpt;
	IndekserOpt.optMustBeNewerThen = 0;
	IndekserOpt.optAllowDuplicates = 0;
	IndekserOpt.optValidDocIDs = NULL;
	IndekserOpt.sequenceMode =0;
	IndekserOpt.garbareCollection = 0;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"ndag"))!=-1) {
                switch (c) {
                        case 'n':
                                IndekserOpt.optMustBeNewerThen = 1;
                                break;
                        case 'g':
                                IndekserOpt.garbareCollection = 1;
                                break;
                        case 'a':
				//lager en peker til ikke null for å markere at vi skal bruke den
                                IndekserOpt.optValidDocIDs = "x";
                                break;

                        case 'd':
                                IndekserOpt.optAllowDuplicates = 1;
                                break;
                        case 'v':
                                break;
                        default:
                                          exit(1);
                }
        }
        --optind;

	printf("lot %s, %i, optind %i\n",argv[1],argc,optind);

	if (((argc -optind) != 4) && ((argc -optind)!= 5)) {

		printf("usage: ./LotInvertetIndexMaker type lotnr subname [ lotPart ]\n\n");

		return 0;
	}



	char *type = argv[1 +optind];
	lotNr = atoi(argv[2 +optind]);
	char *subname = argv[3 +optind];

	if (IndekserOpt.optValidDocIDs != NULL) {
		IndekserOpt.optValidDocIDs = getValidDocIDs(lotNr,subname);
	}

	if ((argc -optind)== 4) {

                //finner siste indekseringstid
                lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);


                if(lastIndexTime == 0) {
                        printf("lastIndexTime is 0\n");
                        exit(1);
                }

               //sjekker om vi har nokk plass
                if (!lotHasSufficientSpace(lotNr,4096,subname)) {
                        printf("insufficient disk space\n");
                        exit(1);
                }


        	printf("Indexing all buvkets for lot %i\n",lotNr);

		for (lotPart=0;lotPart<64;lotPart++) {
			//printf("indexint part %i for lot %i\n",lotPart,lotNr);

			Indekser(lotNr,type,lotPart,subname,&IndekserOpt);	

		}

		//siden vi nå har lagt til alle andringer fra rev index kan vi nå slettet gced filen også
		Indekser_deleteGcedFile(lotNr, subname);


	}
	else if ((argc - optind) == 5) {
		lotPart = atoi(argv[4 +optind]);

		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		Indekser(lotNr,type,lotPart,subname,&IndekserOpt);	

	
	}



}

