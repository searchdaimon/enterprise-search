
#ifndef _GETDATE_H_
#define _GETDATE_H_

#include <sys/types.h>
#include <sys/timeb.h>

#include <time.h>

struct timerange {
	time_t start, end;
};

time_t get_date(char *, struct timeb *, int, struct timerange *);
void init_getdate(void);

#define LANGUAGE_EN 0
#define LANGUAGE_NO 1

#define LANGUAGE_LAST 1
#define LANGUAGE_MAX (LANGUAGE_LAST+1)


#define EPOCH           1970
#define HOUR(x)         ((time_t)(x) * 60)
#define SECSPERDAY      (24L * 60L * 60L)


#endif
