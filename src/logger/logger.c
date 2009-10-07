#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <err.h>
#include <errno.h>

#include "logger.h"

#include "../common/strlcpy.h"
#include "../common/boithohome.h"

#define CONFIG_WATCHER_POLL 1

static logger_t logger;
static char bblog_path[PATH_MAX];
static char bblog_configpath[PATH_MAX];
static severity_t default_severity = WARN;
static pthread_t config_watcher_thread;

void
bblog_set_default_severity(severity_t severity)
{
	default_severity = severity;
}

#if 0
static void *
config_watcher(void *dummy __attribute__((unused)))
{
#ifdef CONFIG_WATCHER_POLL
	/* Add polling here */
#else
	int fd, wd;
	int tries;

	if ((fd = inotify_init()) == -1) {
		warn("Unable to initialize inotify");
		return;
	}

	tries = 0;
 retry_add_watch:
 	/* XXX: Returns 1 (stdin) but does not seem to modify the fd so read() will not work */
	if ((wd = inotify_add_watch(fd, bblog_configpath, IN_CREATE|IN_MODIFY)) == -1) {
		if (errno == ENOENT && tries == 0) {
			int newfile = creat(bblog_configpath, 0644);
			if (newfile != -1)
				close(newfile);
			tries++;
			goto retry_add_watch;
		}
		warn("Unable to add inotify watcher for: %s", bblog_configpath);
		return;
	}

	for (;;) {
		struct inotify_event *buffer;
		char data[sizeof(buffer)+FILENAME_MAX];
		size_t buffer_size = sizeof(buffer), n;

		n = read(wd, &data, sizeof(data));
		buffer = (struct inotify_event *)&data;
		warn("we got a notify thingy...");
	}
#endif
}
#endif

static void
bblog_setup_syslog(void)
{
	if (logger.appenders & LOGGER_APPENDER_SYSLOG) {
		openlog(logger.name, LOG_PID, LOG_LOCAL6);
	}
}

static void
bblog_teardown_syslog(void)
{
	if (logger.appenders & LOGGER_APPENDER_SYSLOG) {
		closelog();
	}
}

int
bblog_init(char *name)
{
	char path[PATH_MAX];
	char *appenders, *severity;
	int default_appenders = LOGGER_APPENDER_SYSLOG;

	if ((appenders = getenv("BBLOGGER_APPENDERS")) != NULL)
		default_appenders = atoi(appenders);

	if ((severity = getenv("BBLOGGER_SEVERITY")) != NULL)
		default_severity = atoi(severity);

	logger.appenders = default_appenders;
	logger.max_severity = default_severity;
	strlcpy(logger.name, name, sizeof(logger.name));
	snprintf(path, sizeof(path), "config/%s.logger.conf", name);
	strlcpy(bblog_configpath, bfile(path), sizeof(bblog_configpath));

	snprintf(path, sizeof(path), "logs/%s.log", name);
	strlcpy(bblog_path, bfile(path), sizeof(bblog_path));

	bblog_setup_syslog();

#if 0
	// Config watcher
	if (pthread_create(&config_watcher_thread, NULL, config_watcher, NULL) != 0)
		warn("Unable to start config watcher thread");
#endif

	return 1;
}

void
bblog_set_severity(severity_t severity)
{
	logger.max_severity = severity;
}

unsigned int
bblog_get_appenders(void)
{
	return logger.appenders;
}

severity_t
bblog_get_severity(void)
{
	return logger.max_severity;
}

void
bblog_set_appenders(unsigned int appenders)
{
	int close_syslog = 0, open_syslog = 1;

	if ((logger.appenders & LOGGER_APPENDER_SYSLOG) != 
	    (appenders & LOGGER_APPENDER_SYSLOG)) {
		if (logger.appenders & LOGGER_APPENDER_SYSLOG) {
			close_syslog = 1;
		} else {
			open_syslog = 1;
		}
	}
	logger.appenders = appenders;
	if (open_syslog)
		bblog_setup_syslog();
	if (close_syslog)
		bblog_teardown_syslog();
}

unsigned int priority_map_to_syslog[] = {
	LOG_ERR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
};

const char *priority_map_to_string[] = {
	"ERROR",
	"WARNING",
	"INFO",
	"DEBUG",
};

static void
bblog_file(FILE *fp, severity_t severity, const char *format, va_list ap, int write_time)
{
	char str[2048];
	size_t len;

	len = 0;
	if (write_time) {
		time_t t;
		struct tm tm;

		t = time(NULL);
		len += strftime(str, sizeof(str), "%d/%m/%y %T: ", localtime_r(&t, &tm));
	}
	
	if (severity != CLEAN)
		len += snprintf(str+len, sizeof(str)-len, "%s: ", priority_map_to_string[severity]);
	len += vsnprintf(str+len, sizeof(str)-len, format, ap);

	fprintf(fp, "%s\n", str);
}

void
bblog(severity_t severity, const char *str, ...)
{
	va_list ap;

	if (severity > logger.max_severity)
		return;

	va_start(ap, str);
	if (logger.appenders & LOGGER_APPENDER_STDERR) {
		bblog_file(stderr, severity, str, ap, 0);
	}
	if (logger.appenders & LOGGER_APPENDER_STDOUT) {
		bblog_file(stdout, severity, str, ap, 0);
	}
	if (logger.appenders & LOGGER_APPENDER_FILE) {
		FILE *fp = fopen(bblog_path, "a");
		if (fp != NULL) {
			bblog_file(fp, severity, str, ap, 1);
			fclose(fp);
		}
	}
	if (logger.appenders & LOGGER_APPENDER_SYSLOG) {
		vsyslog(priority_map_to_syslog[severity], str, ap); 
	}
	va_end(ap);
}

void
bblog_destroy(void)
{
	bblog_teardown_syslog();
	bblog_path[0] = '\0';
}
