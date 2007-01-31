#include <sys/time.h>


double getTimeDifference(struct timeval *start_time, struct timeval *end_time) {

        double timediff;

        timediff = ((*end_time).tv_usec - (*start_time).tv_usec);
        timediff = timediff / 1000000;
        timediff = ((*end_time).tv_sec - (*start_time).tv_sec) + timediff;

	return timediff;
}

