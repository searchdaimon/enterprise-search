#include <stdio.h>
#include <stdlib.h>

#include "lot.h"

#ifdef WITH_THREAD
        #include <pthread.h>
#endif


#define dp_devises 63

#ifdef WITH_THREAD
	static pthread_mutex_t locks[dp_devises];
#endif

void dp_init() {

	#ifdef WITH_THREAD
		int i;

		for(i=0;i<dp_devises;i++) {
			pthread_mutex_init(&locks[i], NULL);
		}
	#endif
}

int dp_lock(int lot) {

	#ifdef WITH_THREAD
		int dev = GetDevIdForLot(lot);

		#ifdef DEBUG
			printf("looking lot %i, dev %i\n",lot,dev);
		#endif

        	return pthread_mutex_lock(&locks[dev]);
	#else
		return 0;
	#endif
}


int dp_unlock(int lot) {

	#ifdef WITH_THREAD
		int dev = GetDevIdForLot(lot);

		#ifdef DEBUG
			printf("UNlooking lot %i, dev %i\n",lot,dev);
		#endif

        	return pthread_mutex_unlock(&locks[dev]);
	#else
		return 0;
	#endif
}
