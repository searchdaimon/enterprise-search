#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

#ifdef DEBUG

void debug(const char *fmt, ...) {

        va_list     ap;

        va_start(ap, fmt);

                printf("debug: ");
                vprintf(fmt,ap);
                printf("\n");

        va_end(ap);

}

void bwarn(const char *fmt, ...) {

        va_list     ap;

        va_start(ap, fmt);

                printf("bwarn: ");
                vprintf(fmt,ap);
                printf("\n");

        va_end(ap);

}

#endif
