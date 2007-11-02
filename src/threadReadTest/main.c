

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
                                371272506-743,
                                449070273-899,
                                427562667-856,
                                434252509-869,
                                379396303-759,
                                482116672-965,
                                380231151-761,
                                380020391-761,
                                365349074-731,
                                375108409-751,
                                379825609-760,
                                364224402-729,
                                365004425-731,
                                364295678-729,
                                360065044-721,


                                377042825-755,
                                360236360-721,
                                377521512-756,
                                365147151-731,
                                437383676-875,
                                432157421-865,
                                431596327-864,
                                379080601-759,
                                371822405-744,
                                428337498-857,
                                481205721-963,
                                471782822-944,
                                459920157-920,
                                437496380-875,
                                435345364-871,
                                434087953-869,
                                382269200-765,
                                373228293-747,
                                369480379-739,
                                365289437-731,


                                381959664-764,
                                375096993-751,
                                379176446-759,
                                478382793-957,
                                437698703-876,
                                429181069-859,
                                431032720-863,
                                438492586-877,
                                444065646-889,
                                362099705-725,
                                375618400-752,
                                457654293-916,
                                444279857-889,
                                436088207-873,
                                381067319-763,
                                429627334-860,
                                427215943-855,
                                376360691-753,
                                377128314-755,
                                372410365-745,


                                379734260-760,
                                385111697-771,
                                383198164-767,
                                371013744-743,
                                365643161-732,
                                439285612-879,
                                381080293-763,
                                372393685-745,
                                457770429-916,
                                481892951-964,
                                480310489-961,
                                479656292-960,
                                467310435-935,
                                374074985-749,
                                431154864-863,
                                373813473-748,
                                372162602-745,
                                466098487-933,
                                364103208-729,
                                373671168-748,


                                380169837-761,
                                336076000-673,
                                369966978-740,
                                379432046-759,
                                381738946-764,
                                370172486-741,
                                371192083-743,
                                361703264-724,
                                339251484-679,
                                379840545-760,
                                378471806-757,
                                374129191-749,
                                370268693-741,
                                367995358-736,
                                368424437-737,
                                363801059-728,
                                361610820-724,
                                360382883-721,
                                360157768-721,
                                359701188-720,

                                380248867-761,
                                437956581-876,
                                444137924-889,
                                461338779-923,
                                381774388-764,
                                447391496-895,
                                445948565-892,
                                437139205-875,
                                431574150-864,
                                379626746-760,
                                375964204-752,
                                383585675-768,
                                383535136-768,

                                383207369-767,
                                365691324-732,
                                366000077-733,
                                383590483-768,
                                369240327-739,
                                369049520-739,
                                364283827-729,

                                367844305-736,
                                379957551-760,
                                373753396-748,
                                360415705-721,
                                385140379-771,
                                372357020-745,
                                365230207-731,
                                361603356-724,
                                359747220-720,
                                467284462-935,
                                463164049-927,
                                479278163-959,
                                445510691-892,
                                438088929-877,
                                431940866-864,
                                435626317-872,
                                451288020-903,
                                367326978-735,

                                437807532-876,
                                373978474-748,

                                385503696-772,
                                379801916-760,
                                381171978-763,
                                371895283-744,
                                371799252-744,
                                365976437-732,
                                369560837-740,
                                499189847-999,
                                481116663-963,
                                471194082-943,
                                442423493-885,
                                378222996-757,
                                427401265-855,
                                431577635-864,
                                377983625-756,
                                385267249-771,
                                433192026-867,
                                428112673-857,
                                379147122-759,
                                363995569-728


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

