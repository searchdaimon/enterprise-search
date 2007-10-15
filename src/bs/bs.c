#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bs.h"

#ifdef WITH_THREAD
	#include <pthread.h>
#endif

void *bs_init(struct bs *s, int max) {

	s->count = 0;
	s->max = max;

	if ((s->t = malloc(sizeof(void *) * s->max)) == NULL) {
		perror("bs_init: malloc s");
		exit(1);
	}

	#ifdef WITH_THREAD
		printf("init mutex\n");
		pthread_mutex_init(&s->mutex, NULL);
	#endif

	printf("bs_init: s->count %i, s->max %i\n",s->count,s->max);
}

int bs_add(struct bs *s, void *data) {

	int forret = 1;

	#ifdef WITH_THREAD
	        pthread_mutex_lock(&s->mutex);		
	#endif


	if ((s->count +1) > s->max) {
		printf("can't add to stack. Hav max elements from before. Have %i, max %i\n",s->count,s->max);
		forret = 0;
	}
	else {
		s->t[s->count] = data;

		++s->count;
	
		#ifdef DEBUG
		printf("added element to bs. Count is now %i\n",s->count);
		#endif
	}
	#ifdef WITH_THREAD
	        pthread_mutex_unlock(&s->mutex);		
	#endif


	return forret;
}

int bs_get(struct bs *s, void **data) {

	int forret = 1;

	
	#ifdef WITH_THREAD
	        pthread_mutex_lock(&s->mutex);		
	#endif

	#ifdef DEBUG
	printf("bs_get: destacking %i\n",s->count);
	#endif

	if (s->count == 0) {
		printf("can't get from stack. Hav 0 elements.\n");
		forret = 0;
	}
	else {
		--s->count;

		*data = s->t[s->count];
	}
	#ifdef WITH_THREAD
	        pthread_mutex_unlock(&s->mutex);		
	#endif

	return forret;
}

