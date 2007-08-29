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



void tq_init(struct tqFormat *tqh) {


  	/* Initialize mutex and condition variable objects */
  	pthread_mutex_init(&tqh->count_mutex, NULL);
	pthread_cond_init (&tqh->count_empty_cv, NULL);
	pthread_cond_init (&tqh->count_full_cv, NULL);


     	tqh->count = 0;
	tqh->have_data = 1;
	
}

int tq_get(struct tqFormat *tqh, void **data) {

	int my_id = 0;

  	while(tqh->have_data) {

    		pthread_mutex_lock(&tqh->count_mutex);

    		/* 
    		Check the value of count and signal waiting thread when condition is
    		reached.  Note that this occurs while mutex is locked. 
    		*/
		//hvis vi har data, og køen er bare halfull så ber vi om mer
    		//if ((tqh->count == ( MAX_Q / 2 )) && (tqh->have_data)) {
    		if ((tqh->count == 0) && (tqh->have_data)) {
			//har ikke flere elementer i kø. Vil vekke opp produsenten
      			pthread_cond_signal(&tqh->count_empty_cv);

			#ifdef DEBUG
      			printf("inc_count(): thread %d, count = %d  Threshold reached.\n", my_id, tqh->count);
			printf("inc_count(): thread %d, waiting for data\n", my_id);
			#endif
			//venter til køen er ful igjen
			pthread_cond_wait(&tqh->count_full_cv, &tqh->count_mutex);

			#ifdef DEBUG
			printf("inc_count(): thread %d, got data to read. Count is %i\n",my_id,tqh->count);
			#endif

      		}

		*data = NULL;
		// vi kan bli vekket av fler grunner. Hvis vi ikke har mer data avslutter vi
		if (!tqh->have_data) {
			//#ifdef DEBUG
			printf("inc_count(): thread %d, don't have eny more data\n", my_id);
			//#endif
		}
		else if (tqh->count == 0) {
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

	int my_id = 0;

  	/*
  	Lock mutex and wait for signal.  Note that the pthread_cond_wait routine
  	will automatically and atomically unlock mutex while it waits. 
  	Also, note that if COUNT_LIMIT is reached before this routine is run by
  	the waiting thread, the loop will be skipped to prevent pthread_cond_wait
  	from never returning.
  	*/
  	pthread_mutex_lock(&tqh->count_mutex);

  		if (tqh->count == MAX_Q) {
			//venter på at køen skal gå tom
		    	pthread_cond_wait(&tqh->count_empty_cv, &tqh->count_mutex);
			#ifdef DEBUG
    			printf("watch_count(): thread %d Condition signal received.\n", my_id);
			#endif
   		}

		tqh->q[tqh->count] = data;
		++tqh->count;

		#ifdef DEBUG
		printf("added new data to work on. count %i\n",tqh->count);
		#endif
		//sender signal om at vi nå har data igjen
		pthread_cond_signal(&tqh->count_full_cv);
    
	pthread_mutex_unlock(&tqh->count_mutex);

}

void *tq_end(struct tqFormat *tqh) {

	int my_id = 0;


	pthread_mutex_lock(&tqh->count_mutex);

		tqh->count = 0;

	  	tqh->have_data = 0;

	pthread_mutex_unlock(&tqh->count_mutex);

	//vekker trådene. Kan være de har stoppet opp da vi ikke hadde mer data. Vil nå aldri kommer mer
	printf("waking threads\n");
	pthread_cond_signal(&tqh->count_full_cv);
	pthread_cond_broadcast(&tqh->count_full_cv);



	//printf("watch_count(): thread %d, exiting.\n", my_id);

}

void *tq_free(struct tqFormat *tqh) {

	//kan vi gjøre dette. Hva om de er i bruk??
 	pthread_mutex_destroy(&tqh->count_mutex);
  	pthread_cond_destroy(&tqh->count_empty_cv);
  	pthread_cond_destroy(&tqh->count_full_cv);

}
