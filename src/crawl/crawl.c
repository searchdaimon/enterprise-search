#include <errno.h>


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crawl.h"

#ifdef WITH_THREAD
        __thread static char _warnmsg[512] = { "" };
        //__thread static char _errormsg[512] = { "" };
#else
        static char _warnmsg[512] = { "" };
        //static char _errormsg[512] = { "" };
#endif

void crawlWarn(const char *fmt, ...) {



        va_list     ap;

        va_start(ap, fmt);

        //#ifdef DEBUG
                vprintf(fmt,ap);
		printf("\n");
        //#endif

        vsprintf(_warnmsg,fmt,ap);



        va_end(ap);

}

char *strcrawlWarn() {
        return _warnmsg;
}
/*
void crawlperror(const char *fmt, ...) {

	//extern int errno;

	char *syserror = strerror(errno);

        va_list     ap;

        va_start(ap, fmt);

        //#ifdef DEBUG
		printf("crawlperror: ");
                vprintf(fmt,ap);
                printf(": %s\n",syserror);
        //#endif

        vsprintf(_errormsg,fmt,ap);
	strcat(_errormsg,": ");
	strcat(_errormsg,syserror);


        va_end(ap);


}
char *strcrawlError() {
        return _errormsg;
}
*/

void *collectionReset (struct collectionFormat *collection) {

 collection->resource 		= NULL;
 collection->connector 		= NULL;
 collection->password 		= NULL;
 collection->query1 		= NULL;
 collection->query2 		= NULL;
 collection->collection_name 	= NULL;
 collection->user 		= NULL;
 collection->lastCrawl 		= 0;
 collection->host 		= NULL;
 //int auth_id;
 //unsigned int id;
 collection->userprefix 	= NULL;
 //char **users;
 collection->extra 		= NULL;
 collection->test_file_prefix 	= NULL;
}
