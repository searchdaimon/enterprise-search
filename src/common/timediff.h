#ifndef _TIMEDIFF__H_
#define _TIMEDIFF__H_

#include <sys/time.h>

double getTimeDifference(struct timeval *start_time, struct timeval *end_time);

#endif
