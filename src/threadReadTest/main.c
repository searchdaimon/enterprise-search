

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
                        //vboprintf("Can't read post for %u-%i\n",(*PagesResults).TeffArray->iindex[i].DocID,rLotForDOCid((*PagesResults).TeffArray->iindex[i].DocID));
                        continue;
                }

		printf("url: \"%s\"\n",DocumentIndex.Url);

		htmlBufferSize = max_html_size;
		if (rReadHtml(htmlBuffer,&htmlBufferSize,DocumentIndex.RepositoryPointer,DocumentIndex.htmlSize,DocID,
				subname,&ReposetoryHeader,NULL,NULL,DocumentIndex.imageSize) != 1) {

			printf("can't read html\n");
			continue;
		}
	}

}

int main () {

	int ret,i;

	struct thargsF thargs;

	unsigned int DocIDs[] = {
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

