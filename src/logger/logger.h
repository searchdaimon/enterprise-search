#ifndef _SRC_LOGGER_LOGGER_H_
#define _SRC_LOGGER_LOGGER_H_

#include <stdarg.h>

typedef enum {
	CLEAN,
	ERROR,
	WARN,
	INFO,
	DEBUGINFO,
} severity_t;

typedef struct _logger {
	char name[64];
	unsigned int appenders;
	severity_t max_severity;
} logger_t;

#define LOGGER_APPENDER_STDOUT	1
#define LOGGER_APPENDER_STDERR	2
#define LOGGER_APPENDER_SYSLOG	4
#define LOGGER_APPENDER_FILE	8

void bblog_set_default_severity(severity_t severity);
int bblog_init(char *name);
void bblog_set_severity(severity_t severity);
void bblog_set_appenders(unsigned int appenders);
unsigned int bblog_get_appenders(void);
severity_t bblog_get_severity(void);
void bblog(severity_t severity, const char *str, ...);
void bblogv(severity_t severity, const char *str, va_list ap);
void bblog_errno(severity_t severity, const char *str, ...);
void bblog_destroy(void);

#endif
