/******************************************************************************
* FILE: condvar1.c
* DESCRIPTION:
*   Example code for using Pthreads condition variables.  The main thread
*   creates three threads.  Two of those threads increment a "count" variable,
*   while the third thread watches the value of "count".  When "count" 
*   reaches a predefined limit, the waiting thread is signaled by one of the
*   incrementing threads.
* SOURCE: Adapted from example code in "Pthreads Programming", B. Nichols
*   et al. O'Reilly and Associates. 
* LAST REVISED: 04/05/05  Blaise Barney
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tq.h"


void tq_init(struct tqFormat *tqh, int qlen) {


  	/* Initialize mutex and condition variable objects */
  	pthread_mutex_init(&tqh->count_mutex, NULL);
	pthread_cond_init (&tqh->count_empty_cv, NULL);
	pthread_cond_init (&tqh->count_full_cv, NULL);


     	tqh->count = 0;
	tqh->have_data = 1;
	tqh->qlen = qlen;

	printf("qlen %i\n",tqh->qlen);


	if ((tqh->q = malloc( (sizeof(void *) * tqh->qlen) )) == NULL) {
		perror("malloc");
		exit(1);
	}

}

int tq_get(struct tqFormat *tqh, void **data) {

	int my_id = 0;

  	while((tqh->have_data) || (tqh->count != 0)) {

    		pthread_mutex_lock(&tqh->count_mutex);

    		/* 
    		Check the value of count and signal waiting thread when condition is
    		reached.  Note that this occurs while mutex is locked. 
    		*/
		//if we have data, and the queue is only half full then we ask for more
    		//if ((tqh->count == ( tqh->qlen / 2 )) && (tqh->have_data)) {
    		if ((tqh->count == 0) && (tqh->have_data)) {
			//no more elements in the queue. Will wake up the producer
      			pthread_cond_signal(&tqh->count_empty_cv);

			#ifdef DEBUG
      			printf("inc_count(): thread %d, count = %d  Threshold reached.\n", my_id, tqh->count);
			printf("inc_count(): thread %d, waiting for data\n", my_id);
			#endif
			//wait until the queue is full again
			pthread_cond_wait(&tqh->count_full_cv, &tqh->count_mutex);

			#ifdef DEBUG
			printf("inc_count(): thread %d, got data to read. Count is %i\n",my_id,tqh->count);
			#endif

      		}

		*data = NULL;
		// we can be woken for several reasons. If we have no more data we finish
		//if (!tqh->have_data) {
		//	//#ifdef DEBUG
		//	printf("inc_count(): thread %d, don't have eny more data\n", my_id);
		//	//#endif
		//}
		//else 
		if (tqh->count == 0) {
			#ifdef DEBUG
			printf("inc_count(): thread %d, got woken up, but don't have data\n",my_id);
			#endif
		}
		else {
			#ifdef DEBUG
			printf("inc_count(): thread %d, suptracting 1 from count\n", my_id);
			#endif

    			tqh->count--;
	
			*data = tqh->q[tqh->count];
			#ifdef DEBUG			
    			printf("inc_count(): thread %d, count = %d, unlocking mutex\n",  my_id, tqh->count);
			#endif
		}

		//the queue is half full. If the producer is sleeping it can start working again
    		if ((tqh->count == ( tqh->qlen / 2 )) && (tqh->have_data)) {
      			pthread_cond_signal(&tqh->count_empty_cv);
		}


		pthread_mutex_unlock(&tqh->count_mutex);



		if (*data != NULL) {
			return 1;
		}



    	}

	//#ifdef DEBUG
    	printf("inc_count(): thread %d, count = %d, exiting\n", my_id, tqh->count);
	//#endif

	return 0;


}

void tq_add(struct tqFormat *tqh, void *data) {

	#ifdef DEBUG
		int my_id = 0;
	#endif

  	/*
  	Lock mutex and wait for signal.  Note that the pthread_cond_wait routine
  	will automatically and atomically unlock mutex while it waits. 
  	Also, note that if COUNT_LIMIT is reached before this routine is run by
  	the waiting thread, the loop will be skipped to prevent pthread_cond_wait
  	from never returning.
  	*/
  	pthread_mutex_lock(&tqh->count_mutex);

  		if (tqh->count == tqh->qlen) {
			//wait for the queue to run empty
		    	pthread_cond_wait(&tqh->count_empty_cv, &tqh->count_mutex);
			#ifdef DEBUG
    			printf("watch_count(): thread %d Condition signal received.\n", my_id);
			#endif
   		}
		
		//if we now have 0 elements in the queue we wake the threads. No reason to wake them every time
		if (tqh->count == 0) {
			//signal that we have data again
			pthread_cond_signal(&tqh->count_full_cv);
		}
		tqh->q[tqh->count] = data;
		++tqh->count;

		#ifdef DEBUG
		printf("added new data to work on. count %i\n",tqh->count);
		#endif
		//signal that we have data again
		//pthread_cond_signal(&tqh->count_full_cv);
    
	pthread_mutex_unlock(&tqh->count_mutex);

}

void tq_end(struct tqFormat *tqh) {

	#ifdef DEBUG
	int my_id = 0;
	#endif

	pthread_mutex_lock(&tqh->count_mutex);

		//tqh->count = 0;

	  	tqh->have_data = 0;

	pthread_mutex_unlock(&tqh->count_mutex);

	//wake the threads. They may have stopped when we had no more data. Will now never get more
	printf("waking threads\n");
	pthread_cond_signal(&tqh->count_full_cv);
	pthread_cond_broadcast(&tqh->count_full_cv);



	//printf("watch_count(): thread %d, exiting.\n", my_id);

}

void tq_free(struct tqFormat *tqh) {

	//can we do this. What if they are in use??
	//should not be the case, since we only call this after we have waited for all threads
 	pthread_mutex_destroy(&tqh->count_mutex);
  	pthread_cond_destroy(&tqh->count_empty_cv);
  	pthread_cond_destroy(&tqh->count_full_cv);

}
