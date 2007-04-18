#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static void
setrlimits(void)
{
	struct rlimit rlim;

	if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
		perror("getrlimit");
		return;
	}
	if (rlim.rlim_max == RLIM_INFINITY || rlim.rlim_cur < rlim.rlim_max) {
		rlim.rlim_cur = rlim.rlim_max;
		printf("rlim_cur: %d\n", rlim.rlim_cur);
		if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
			perror("setrlimit");
		}
	}
	if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
		perror("getrlimit");
		return;
	}
	printf("Soft: %d Hard: %d\n", rlim.rlim_cur, rlim.rlim_max);
}

#ifndef WCOREDUMP
#error "No WCOREDUMP support"
#endif

int
get_coredumppath(int pid, char *buf, size_t len)
{
	FILE *fp;
	char *tmpbuf, *p;
	int foundpidmark = 0;

	if ((tmpbuf = malloc(len * sizeof(char))) == NULL)
		return -1;

	fp = fopen("/proc/sys/kernel/core_pattern", "r");
	if (fp == NULL) {
		free(tmpbuf);
		return -1;
	}

	fread(tmpbuf, 1, len, fp);
	fclose(fp);

	p = tmpbuf;
	for (p = tmpbuf; *p; p++) {
		if (*p == '%') {
			p++;
			if (*p == '\0')
				break;
			/* Only allow %p and %% */
			if (!(*p == 'p' || *p == '%')) {
				free(tmpbuf);
				return -1;
			}
			/* Rewrite string so it will print correctly */
			if (*p == 'p') {
				*p = 'd';
				foundpidmark = 1;
			}
		} else if (*p == '\n') {
			*p = '\0';
			break;
		}
	}
	if (!foundpidmark) {
		free(tmpbuf);
		return -1;
	}

	snprintf(buf, len, tmpbuf, pid);
	free(tmpbuf);
	
	return 0;
}

int
grab_coredump(char *buf, int pid)
{
	struct stat st;

	/* Does the file exist? */
	if (stat(buf, &st) == -1) {
		return -1;
	}

	fprintf(stderr, "Found the file!\n");

	return 0;
}

int
main(void)
{
	int pid;

	/* We need to set the core dump size */
	setrlimits();

	pid = fork();
	if (pid == 0) { /* Child */
		execlp("/bin/sh", "sh", "dump.sh", NULL);
	} else if (pid > 0) { /* Parent */
		int status;

		while (waitpid(pid, &status, 0) <= 0) {
			fprintf(stderr, "Looping...\n");
		}
		if (WIFSIGNALED(status) && WCOREDUMP(status)) {
			char buf[1024];

			/* Need to find the coredump if we get here... */
			if (get_coredumppath(pid, buf, sizeof(buf)) == 0) {
				printf("Got a coredump! %s\n", buf);
				grab_coredump(buf, pid);
			} else {
				fprintf(stderr, "Unable to locate coredump\n");
			}
		}
	} else { /* Error */
		perror("fork()");
	}

	return 0;
}
