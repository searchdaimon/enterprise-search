#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WITH_THREAD

	//toDo hvorfor funker ikke __thread her ???
	// __thread static char _errormsg[512] = { "" };
	static char _errormsg[512] = { "" };
#else
	static char _errormsg[512] = { "" };
#endif

void berror(const char *fmt, ...) {



	va_list     ap;

    	va_start(ap, fmt);

	printf("in berror\n");

	//#ifdef DEBUG	
		printf("berror: ");	
		vprintf(fmt,ap);
		printf("\n");
	//#endif

	vsprintf(_errormsg,fmt,ap);
	


	va_end(ap);

}

void bperror(const char *fmt, ...) {

        extern int errno;

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


char *bstrerror() {
	return _errormsg;
}
