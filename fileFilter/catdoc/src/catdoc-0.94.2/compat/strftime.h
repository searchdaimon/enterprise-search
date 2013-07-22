#ifndef STRFTIME_H
#define STRFTIME_H
#include <stdlib.h>
#include <time.h>

size_t strftime(char *s,size_t max, const char *format,
		const struct tm *tm);

#endif
