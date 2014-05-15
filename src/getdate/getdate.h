#ifndef _GETDATE_H_
#define _GETDATE_H_

#include <time.h>
#include "getdate.tab.h"

struct datelib {
        time_t start;
        time_t end;

        struct tm tmstart;

        struct {
                int year, month, week, day;
        } modify;

	int frombigbang;

#ifdef __INGETDATE
        enum yytokentype lowest;
#else
	int __padding1;
#endif
};


int sd_getdate(char *, struct datelib *);

#endif /* _GETDATE_H_ */
