

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <string.h>

#include "../common/DocumentIndex.h"

#define subname "www"
#define max_html_size 900000

#ifdef WITH_THREAD
        #include <pthread.h>
	#define NROF_GENERATEPAGES_THREADS 3

#endif


struct thargsF {
	int nrOfDocID;
	int readDocID;
	unsigned int *DocIDs;
	pthread_mutex_t mutex;
};

unsigned int NexDocID(struct thargsF *thargs) {

	unsigned int ret;

#ifdef WITH_THREAD
        pthread_mutex_lock(&thargs->mutex);
#endif

	if (thargs->readDocID < thargs->nrOfDocID) {
		ret = thargs->DocIDs[thargs->readDocID];
		++thargs->readDocID;
	}
	else {
		ret = 0;
	}

#ifdef WITH_THREAD
        pthread_mutex_unlock(&thargs->mutex);
#endif

	return ret;

}

void *generatePagesResults(void *arg)
{

        struct thargsF * thargs = (struct thargsF *)arg;

	struct DocumentIndexFormat DocumentIndex;
	struct ReposetoryHeaderFormat ReposetoryHeader;
	
	int canDIRead = 0;
	int canrReadHtml = 0;	

	unsigned int htmlBufferSize;
       	char *htmlBuffer;

        if ((htmlBuffer = malloc(max_html_size)) == NULL) {
                perror("can't malloc");
                return;
        }


	unsigned int DocID;

	printf("in thread\n");

	while (( DocID = NexDocID(thargs) ) != 0) {

		//leser DI
		if (!DIRead_fmode(&DocumentIndex,DocID,subname,'r')) 
		{
                        //hvis vi av en eller annen grun ikke kunne gjøre det kalger vi
                        printf("Can't read DI post for %u-%i\n",DocID,rLotForDOCid(DocID));
                        continue;
                }
		else {
			++canDIRead;
		}
		printf("url: \"%s\"\n",DocumentIndex.Url);

		htmlBufferSize = max_html_size;
		
		if (DocumentIndex.htmlSize == 0) {

		}
		else if (rReadHtml(htmlBuffer,&htmlBufferSize,DocumentIndex.RepositoryPointer,DocumentIndex.htmlSize,DocID,
				subname,&ReposetoryHeader,NULL,NULL,DocumentIndex.imageSize) != 1) {

                        printf("Can't read html post for %u-%i\n",DocID,rLotForDOCid(DocID));

			continue;
		}
		else {
			++canrReadHtml;
		}
	}

	printf("canDIRead %i\n",canDIRead);
	printf("canrReadHtml %i\n",canrReadHtml);
}

int main () {

	int ret,i;

	struct thargsF thargs;

	unsigned int DocIDs[] = {
                                377071633,
                                431100155,
                                448484188,
                                432144012,
                                455617227,
                                371272506,
                                449070273,
                                427562667,
                                434252509,
                                379396303,
                                482116672,
                                380231151,
                                380020391,
                                365349074,
                                375108409,
                                379825609,
                                364224402,
                                365004425,
                                364295678,
                                360065044,


                                377042825,
                                360236360,
                                377521512,
                                365147151,
                                437383676,
                                432157421,
                                431596327,
                                379080601,
                                371822405,
                                428337498,
                                481205721,
                                471782822,
                                459920157,
                                437496380,
                                435345364,
                                434087953,
                                382269200,
                                373228293,
                                369480379,
                                365289437,


                                381959664,
                                375096993,
                                379176446,
                                478382793,
                                437698703,
                                429181069,
                                431032720,
                                438492586,
                                444065646,
                                362099705,
                                375618400,
                                457654293,
                                444279857,
                                436088207,
                                381067319,
                                429627334,
                                427215943,
                                376360691,
                                377128314,
                                372410365,


                                379734260,
                                385111697,
                                383198164,
                                371013744,
                                365643161,
                                439285612,
                                381080293,
                                372393685,
                                457770429,
                                481892951,
                                480310489,
                                479656292,
                                467310435,
                                374074985,
                                431154864,
                                373813473,
                                372162602,
                                466098487,
                                364103208,
                                373671168,


                                380169837,
                                336076000,
                                369966978,
                                379432046,
                                381738946,
                                370172486,
                                371192083,
                                361703264,
                                339251484,
                                379840545,
                                378471806,
                                374129191,
                                370268693,
                                367995358,
                                368424437,
                                363801059,
                                361610820,
                                360382883,
                                360157768,
                                359701188,

                                380248867,
                                437956581,
                                444137924,
                                461338779,
                                381774388,
                                447391496,
                                445948565,
                                437139205,
                                431574150,
                                379626746,
                                375964204,
                                383585675,
                                383535136,

                                383207369,
                                365691324,
                                366000077,
                                383590483,
                                369240327,
                                369049520,
                                364283827,

                                367844305,
                                379957551,
                                373753396,
                                360415705,
                                385140379,
                                372357020,
                                365230207,
                                361603356,
                                359747220,
                                467284462,
                                463164049,
                                479278163,
                                445510691,
                                438088929,
                                431940866,
                                435626317,
                                451288020,
                                367326978,

                                437807532,
                                373978474,

                                385503696,
                                379801916,
                                381171978,
                                371895283,
                                371799252,
                                365976437,
                                369560837,
                                499189847,
                                481116663,
                                471194082,
                                442423493,
                                378222996,
                                427401265,
                                431577635,
                                377983625,
                                385267249,
                                433192026,
                                428112673,
                                379147122,
                                363995569


			/*
				125506676,
				125500032,
				125546676,
				125500033,
				125500034,
				125506676,
				125506675,
				125506671,
				125516679,
				125546674,
				125506676,
				125556679,
				125536674,
				125506373,
				125519676,
				125546696,
				125529676,
				125525636,
				125511626,
				125517646,
				125534686,
				125505576,
				125546676,
				125506646,
				125536576,
				125516696,
				125506676
			*/
			};

	thargs.readDocID = 0;
	thargs.nrOfDocID = (sizeof(DocIDs) / sizeof(unsigned int));
	thargs.DocIDs = DocIDs;
	
	printf("will look up %i DocID's\n",thargs.nrOfDocID);

	#ifdef WITH_THREAD
	pthread_t threadid[NROF_GENERATEPAGES_THREADS];

	
	ret = pthread_mutex_init(&thargs.mutex, NULL);

	//ret = pthread_mutex_init(&PagesResults.mutextreadSyncFilter, NULL);

	//låser mutex. Vi er jo enda ikke kalre til å kjøre
	pthread_mutex_lock(&thargs.mutex);
	
	//start som thread that can get the pages
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {

		ret = pthread_create(&threadid[i], NULL, generatePagesResults, &thargs);		
	}
	#endif




	#ifdef WITH_THREAD

	//vi har data. Lå tårdene jobbe med det
	pthread_mutex_unlock(&thargs.mutex);

	//venter på trådene
	for (i=0;i<NROF_GENERATEPAGES_THREADS;i++) {
		ret = pthread_join(threadid[i], NULL);
	}

	//free mutex'en
	//ret = pthread_mutex_destroy(&PagesResults.mutex);
	#endif
}

