
#include <sys/types.h>
#include <sys/stat.h>

#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>


#ifndef WEB_USER
#define WEB_USER "apache"
#endif
#ifndef EXEC_USER
#define EXEC_USER "phonehome"
#endif

#define __unused __attribute__((__unused__))

int
main(int argc, char **argv, char **envp __unused)
{
	char *perlprog = "/usr/bin/perl";
	char program[1024];
	char *boithohome;
	int ret;
	struct passwd *pwd;
	uid_t webuid, execuid;
	uid_t uid = getuid();
	char *perlenv[] = { NULL };
	struct stat st;

	if (argc < 2)
		errx(1, "Usage: ./setuidcaller cmd");

#if 0
	if (stat(program, &st) == -1) {
		fprintf(stderr, "Unable to find client script %s: ", program);
		perror("");
		return 82;
	}
#endif

	if ((pwd = getpwnam(WEB_USER)) == NULL) {
		perror("Could not find web user");
		return 80;
	}
	webuid = pwd->pw_uid;
	if ((pwd = getpwnam(EXEC_USER)) == NULL) {
		perror("Could not find exec user");
		return 81;
	}
	execuid = pwd->pw_uid;


	if (uid != webuid) {
		errno = EPERM;
		perror("Started as the wrong user");
		return 99;
	}

	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments.\n");
		return 98;
	}

	boithohome = getenv("BOITHOHOME");
	if (boithohome == NULL) {
		err(1, "getenv(BOITHOHOME)");
		return 105;
	}
	snprintf(program, sizeof(program), "%s/bin/bb-client.pl", boithohome);

	if (setuid(0) != 0) {
		perror("Could not setuid(0)");
		return 97;
	}
	if (seteuid(0) != 0) {
		perror("Could not seteuid(0)");
		return 96;
	}
	if (setuid(execuid) != 0) {
		perror("Could not setuid()");
		return 95;
	}
	if (seteuid(execuid) != 0) {
		perror("Could not seteuid()");
		return 94;
	}

	if (strcmp(argv[1], "start") == 0) {
		ret = execlp(perlprog, "perl", program, "start", NULL, perlenv, NULL);
	} else if (strcmp(argv[1], "stop") == 0) {
		ret = execlp(perlprog, "perl", program, "stop", NULL, perlenv, NULL);
	} else {
		ret = execlp(perlprog, "perl", program, "running", NULL, perlenv, NULL);
	}

	perror("Unable to exec program");

	return 100;
}

