#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef WITH_THREAD

	//toDo hvorfor funker ikke __thread her ???
	// __thread static char _errormsg[512] = { "" };
	static char _errormsg[512] = { "" };
#else
	static char _errormsg[512] = { "" };
#endif


void bverror(const char *fmt,va_list     ap) {

	#ifdef DEBUG
		printf("bverror: ");	
		vprintf(fmt,ap);
		printf("\n");
	#endif

	vsprintf(_errormsg,fmt,ap);
}

void berror(const char *fmt, ...) {

	va_list     ap;
    	va_start(ap, fmt);

	#ifdef DEBUG	
		printf("berror: ");	
		vprintf(fmt,ap);
		printf("\n");
	#endif

	vsprintf(_errormsg,fmt,ap);
	va_end(ap);
}
void berror_safe(const char *str) {

	#ifdef DEBUG	
		printf("berror: %s\n", str);	
	#endif

	sprintf(_errormsg, "%s", str);
}



void bperror(const char *fmt, ...) {

        char *syserror = strerror(errno);

        va_list     ap;

        va_start(ap, fmt);

        #ifdef DEBUG
                printf("crawlperror: ");
                vprintf(fmt,ap);
                printf(": %s\n",syserror);
        #endif

        vsprintf(_errormsg,fmt,ap);
        strcat(_errormsg,": ");
        strcat(_errormsg,syserror);

        va_end(ap);
}


char *bstrerror() {
	return _errormsg;
}
