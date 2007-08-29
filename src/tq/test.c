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

#define NUM_THREADS  5

int     thread_ids[] = {0,1,2,3,4};


void *inc_count(void *idp) 
{

  	struct tqFormat *tqh = (struct tqFormat *)idp;

  int my_id = (int)pthread_self();

  //int my_id = *(int *)idp;
  void *data;
	int j;
	double result = 0;
	while(tq_get(tqh,&data)) {

		printf("data %c\n",*(char *)data);

		free(data);
		
    		printf("inc_count(): thread %d, working on data\n",  my_id);

		// Do some work so threads can alternate on mutex lock 
		for (j=0; j < 10000000; j++) {
			result = result + result;
		}
  		printf("inc_count(): thread %d, done working on data\n",  my_id);
		
	}

	
	return 0;
}

void *add_data(void *idp) 
{

  	struct tqFormat *tqh = (struct tqFormat *)idp;

  	int my_id = 0;
  	int i;

	char c;
	char *cp;

  	printf("Starting watch_count(): thread %d\n", my_id);

	for (i=0; i<27; i++) {

		c = (char)((int)'a' + i);
		//c = 1;


		cp = malloc(sizeof(char));
		*cp = c;

		//#ifdef DEBUG
		printf("trying to add %c\n",*cp);
		//#endif

		tq_add(tqh,cp);

	}


	tq_end(tqh);	


	return 0;
}

int main(int argc, char *argv[])
{
  int i;
  pthread_t threads[5];
  pthread_attr_t attr;

  struct tqFormat *tqh = malloc(sizeof(struct tqFormat));

  /* For portability, explicitly create threads in a joinable state */
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  tq_init(tqh);


  for (i = 0; i < NUM_THREADS; i++) {
	pthread_create(&threads[i], &attr, inc_count, (void *)tqh);
  }

  add_data((void *)tqh);

  /* Wait for all threads to complete */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  printf ("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

  /* Clean up and exit */
  pthread_attr_destroy(&attr);
  pthread_exit (NULL);


}

