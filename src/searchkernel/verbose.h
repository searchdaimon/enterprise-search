#ifndef _SD_VERBOSE__H_
#define _SD_VERBOSE__H_

#include <string.h>

extern int globalOptVerbose;


#define vboprintf(str, args...) if (globalOptVerbose) printf(str, ##args)

#endif //#ifndef _SD_VERBOSE__H_
