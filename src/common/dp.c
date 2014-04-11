#include <stdio.h>
#include <stdlib.h>

#include "lot.h"

#ifdef WITH_THREAD
        #include <pthread.h>


#define dp_devises 63

static pthread_mutex_t locks[dp_devises];

void dp_init() {

	int i;

	for(i=0;i<dp_devises;i++) {
		pthread_mutex_init(&locks[i], NULL);
	}
}

int dp_lock(int lot) {

	int dev = GetDevIdForLot(lot);

	#ifdef DEBUG
		printf("looking lot %i, dev %i\n",lot,dev);
	#endif

       	return pthread_mutex_lock(&locks[dev]);
}


int dp_unlock(int lot) {

	int dev = GetDevIdForLot(lot);

	#ifdef DEBUG
		printf("UNlooking lot %i, dev %i\n",lot,dev);
	#endif

       	return pthread_mutex_unlock(&locks[dev]);
}

#endif
