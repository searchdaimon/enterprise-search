#ifndef _SCAN_H_
#define _SCAN_H_

#include "../crawl/crawl.h"

int scanSMB(int (*scan_found_share)(char share[]),char host[],char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...));

#endif	// _SCAN_H_

