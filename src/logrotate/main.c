#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <bits/posix2_lim.h>

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <err.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINES 10000

#define MIN(a, b) (a) < (b) ? (a) : (b)

int unknowndaemon = 0;

int fdlock;

char *
lockpath(char *daemon)
{
	char *p;
	char *bh;

	bh = getenv("BOITHOHOME");
	if (bh == NULL)
		err(1, "BOITHOHOME not set");
	asprintf(&p, "%s/var/%s.log.lock", bh, daemon);
	if (p == NULL)
		err(1, "asprintf()");

	return p;
}

char *
pidpath(char *daemon)
{
	char *p;
	char *bh;

	bh = getenv("BOITHOHOME");
	if (bh == NULL)
		err(1, "BOITHOHOME not set");
	asprintf(&p, "%s/var/%s.gpid", bh, daemon);
	if (p == NULL)
		err(1, "asprintf()");

	return p;
}


void
rotatelog(char *path)
{
	struct stat st;
	FILE *fp;
	char *buf;
	size_t n, i, j;

	printf("Rotating: %s\n", path);

	if (stat(path, &st) == -1) {
		warn("Unable to stat: %s", path);
		return;
	}

	if (st.st_size <= MAX_LINES * LINE_MAX) {
		printf("No need to rotate log...\n");
		//return;
	}

	buf = malloc(MAX_LINES * LINE_MAX);
	if (buf == NULL)
		return;
	fp = fopen(path, "r");
	if (fp == NULL) {
		free(buf);
		return;
	}
	fseek(fp, st.st_size - MAX_LINES*LINE_MAX, SEEK_SET);

	/* Slurp in file */
	n = 0;
	while ((i = fread(buf+n, 1, 1024, fp)) > 0) {
		n += i;
	}

	fclose(fp);
	unlink(path);
	fp = fopen(path, "w");
	if (fp == NULL) {
		free(buf);
		warn("Losing data in %s", path);
		return;
	}

	i = 0;
	while (i < n) {
		j = fwrite(buf+i, 1, MIN(1024, n), fp);
		i += j;
	}
	fclose(fp);
	free(buf);
}

void
daemon_hup(char *daemon)
{
	char *path;
	char buf[2048];
	FILE *fp;
	pid_t pid;

	printf("Suppose to contact daemon... %s\n", daemon);

	path = pidpath(daemon);
	fp = fopen(path, "r");
	free(path);
	if (fp == NULL)
		err(1, "Unable to get pid for daemon: %s, %s", daemon, path);

	fread(buf, 1, 2048, fp);
	pid = atoi(buf);
	fclose(fp);

	path = lockpath(daemon);
	printf("Locking log file\n");
	fdlock = creat(path, S_IRWXU|S_IRGRP|S_IXGRP);
	if (fdlock == -1)
		err(1, "open(lock)");
	if (flock(fdlock, LOCK_EX) == -1)
		err(1, "flock()");

	kill(-pid, SIGUSR2); // HUP!
}

void
daemon_droplocks(char *daemon)
{
	printf("droping locks... %s\n", daemon);
	flock(fdlock, LOCK_UN);
	close(fdlock);
}

int
main(int argc, char **argv)
{
	int i;

	if (argc < 3) {
		errx(1, "%s daemon logfile1 [logfileN ..]", argv[0]);
	}

	if (*argv[0] == '\0')
		unknowndaemon = 1;

	if (!unknowndaemon)
		daemon_hup(argv[1]);

	for (i = 2; i < argc; i++)
		rotatelog(argv[i]);

	if (!unknowndaemon)
		daemon_droplocks(argv[1]);

	return 0;
}
