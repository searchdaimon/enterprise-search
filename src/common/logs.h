#ifndef _LOGS__H_
#define _LOGS__H_


#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void blog(FILE *LOG, int level, const char *fmt, ...);
int openlogs(FILE **LOGACCESS, FILE **LOGERROR, char name[]);
void closelogs(FILE *LOGACCESS, FILE *LOGERROR);



#endif
