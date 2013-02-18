#include "../crawl/crawl.h"

int scanSMB(int (*scan_found_share)(char share[]),char host[],char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...));

