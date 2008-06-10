/*
 * Catchdumper / everrun
 *
 * Constantly respawn a program, and save coredumps.
 *
 * Eirik A. Nygaard
 * April 2007
 */

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
#include <fcntl.h>

#define COREDUMPDIR "/coredumps/saved"

static void
setrlimits(void)
{
	struct rlimit rlim;

	rlim.rlim_cur = RLIM_INFINITY;
	rlim.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
		perror("setrlimit");
	}
	if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
		perror("getrlimit");
		return;
	}
	printf("%d %d\n", rlim.rlim_cur, rlim.rlim_max);
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

	if ((fp = fopen("/proc/sys/kernel/core_pattern", "r")) == NULL) {
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
dup_file(char *from, char *to)
{
	int fdfrom, fdto;
	int n;
	char buf[10240];

	if ((fdfrom = open(from, O_RDONLY, 0)) == -1) {
		perror("open(from)");
		return -1;
	}
	if ((fdto = open(to, O_WRONLY|O_EXCL|O_CREAT)) == -1) {
		perror("open(to)");
		close(fdfrom);
		return -1;
	}
	
	while ((n = read(fdfrom, buf, sizeof(buf))) > 0) {
		if (write(fdto, buf, n) == -1)
			perror("write()");
	}
	if (n == -1) {
		perror("read()");
		close(fdfrom);
		close(fdto);
	}

	close(fdfrom);
	close(fdto);
	chmod(to, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	
	return 0;
}

int
grab_coredump(char *buf, int pid, char *progname)
{
	struct stat st;
	char path[1024];
	char *p, *prognametmp;

	/* Does the file exist? */
	if (stat(buf, &st) == -1) {
		return -1;
	}
	if ((prognametmp = strdup(progname)) == NULL) {
		return -1;
	}

	for (p = prognametmp; *p != '\0'; p++) {
		if (*p == '/') {
			*p = '-';
		}
	}
	snprintf(path, sizeof(path), "%s/%s.%d", COREDUMPDIR, prognametmp, pid);
	free(prognametmp);

	if (dup_file(buf, path)) {
		perror("Unable to duplicate corefile, removing old one anyway");
		unlink(buf);
		return -1;
	}

	unlink(buf);

	return 0;
}

int
dumpcatcher(char *prog, char **argv)
{
	int pid;

	pid = fork(); 
	if (pid == 0) { /* Child */
		execvp(prog, argv);
	} else if (pid > 0) { /* Parent */
		int status;

		while (waitpid(pid, &status, 0) <= 0) {
			; // It should not iterate
		}
		if (WIFSIGNALED(status) && WCOREDUMP(status)) {
			char buf[1024];

			puts("Dumped core...\n");
			/* Need to find the coredump if we get here... */
			if (get_coredumppath(pid, buf, sizeof(buf)) == 0) {
				if (grab_coredump(buf, pid, prog) < 0)
					perror("Unable to grab coredump");
			} else {
				fprintf(stderr, "Unable to locate coredump.\n");
			}
		}
	} else { /* Error */
		perror("fork()");
	}

	return 0;
}

int
main(int argc, char **argv)
{
	char **newargv;
	int i;

	if (argc < 2) {
		fprintf(stderr, "Syntax: ./everrun prog [arg1 [argn ...]]\n");
		exit(1);
	}
	newargv = malloc((argc)*sizeof(char *));
	for (i = 0; i < argc; i++) {
		newargv[i] = argv[i+1];
	}
	newargv[i] = NULL;

	/* We need to set the core dump size */
	setrlimits();

	printf("Running: `");
	for (i = 0; i < argc-1; i++) {
		printf("%s%s", i==0?"":" ", newargv[i]);
	}
	puts("`");

	for (;;) {
		puts("Trying...\n");
		dumpcatcher(newargv[0], newargv);
		sleep(1); // Avoid hammering
	}

	return 0;
}
