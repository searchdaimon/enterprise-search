
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/file.h>

#include "../common/timediff.h"
#include "../common/boithohome.h"

//#define DEBUG
#define DEBUG_TIME

#ifdef DEBUG_TIME
	#include <sys/time.h>
#endif

#define DP_LOCK_FILE "var/dp.lock"

/* Original pthread function */
static size_t (*fread_orig)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static size_t (*fwrite_orig)(const void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static int (*fclose_orig)(FILE *stream) = NULL;
static FILE * (*fopen_orig)(const char *path, const char *mode) = NULL;


static int DP_LOCK;

/* Library initialization function */
void __attribute__ ((constructor)) wooinit(void);


static inline void dp_disklock() {

	#ifdef DEBUG_TIME
		struct timeval start_time, end_time;
		int was_locked = 0;

		if (flock(DP_LOCK,LOCK_NB |LOCK_EX) != 0) {
			printf("lock is bussy, denying\n");
			gettimeofday(&start_time, NULL);
			was_locked = 1;
		}
	#endif
	//wait to noone ellse want to use the disk
	flock(DP_LOCK,LOCK_EX);

	//release the lock so noone els have to wait for me
	flock(DP_LOCK,LOCK_UN);

	#ifdef DEBUG_TIME
		if (was_locked) {
			gettimeofday(&end_time, NULL);
			printf("dp_disklock: did pause in %f sec\n",getTimeDifference(&start_time,&end_time));
		}
	#endif

}

void wooinit(void) {


	//open the lock file
	//if ((DP_LOCK =open(DP_LOCK_FILE,O_CREAT|O_RDWR,S_IRWXU)) == -1) {
	if ((DP_LOCK =open(bfile(DP_LOCK_FILE),O_CREAT|O_RDWR,S_IRWXU)) == -1) {
		perror(DP_LOCK_FILE);
		exit(EXIT_FAILURE);
	}

	//fwrite
 	fwrite_orig = dlsym(RTLD_NEXT, "fwrite");

    	if(fwrite_orig == NULL)
    	{
        	char *error = dlerror();
        	if(error == NULL)
        	{
        	    error = "fwrite is NULL";
        	}
        	fprintf(stderr, "%s\n", error);
        	exit(EXIT_FAILURE);
    	}


	//fread
 	fread_orig = dlsym(RTLD_NEXT, "fread");

    	if(fread_orig == NULL)
    	{
        	char *error = dlerror();
        	if(error == NULL)
        	{
        	    error = "fread is NULL";
        	}
        	fprintf(stderr, "%s\n", error);
        	exit(EXIT_FAILURE);
    	}


	//foepn
 	fopen_orig = dlsym(RTLD_NEXT, "fopen64");

    	if(fopen_orig == NULL)
    	{
        	char *error = dlerror();
        	if(error == NULL)
        	{
        	    error = "fopen is NULL";
        	}
        	fprintf(stderr, "%s\n", error);
        	exit(EXIT_FAILURE);
    	}
	

	//fclose
 	fclose_orig = dlsym(RTLD_NEXT, "fclose");

    	if(fclose_orig == NULL)
    	{
        	char *error = dlerror();
        	if(error == NULL)
        	{
        	    error = "fclose is NULL";
        	}
        	fprintf(stderr, "%s\n", error);
        	exit(EXIT_FAILURE);
    	}

    	fprintf(stderr, "pthreads: using write hooks\n");

}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {

	dp_disklock();

	#ifdef DEBUG
  	printf("fread(%d) is called\n", size*nmemb);    
 	#endif

  	return(fread_orig(ptr,size,nmemb,stream));

}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {

	dp_disklock();

	#ifdef DEBUG
  	printf("fwrite(%d) is called\n", size*nmemb);    
 	#endif

  	return(fwrite_orig(ptr,size,nmemb,stream));
}

int fclose(FILE *stream) {

	dp_disklock();

	#ifdef DEBUG
  	printf("fclose is called\n");    
 	#endif

	return(fclose_orig(stream));

}

FILE *fopen(const char *path, const char *mode) {

	dp_disklock();

	#ifdef DEBUG
  	printf("fopen(%s) is called\n", path);    
 	#endif

	return(fopen_orig(path,mode));
}

