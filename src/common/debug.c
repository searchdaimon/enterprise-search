#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void debug(const char *fmt, ...) {

        #ifdef DEBUG


        va_list     ap;

        va_start(ap, fmt);

                printf("debug: ");
                vprintf(fmt,ap);
                printf("\n");




        va_end(ap);

        #endif

}


void bwarn(const char *fmt, ...) {

        #ifdef DEBUG


        va_list     ap;

        va_start(ap, fmt);

                printf("bwarn: ");
                vprintf(fmt,ap);
                printf("\n");




        va_end(ap);

	//kansje skrive det til fil?
        #endif

}


