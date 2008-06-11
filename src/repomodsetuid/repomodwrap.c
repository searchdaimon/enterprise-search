
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
		errx(1, "Usage: ./repomodwrap cmd");

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

#ifdef JUSTATEST
	if (uid != webuid) {
		errno = EPERM;
		perror("Started as the wrong user");
		return 99;
	}
#endif

	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments.\n");
		return 98;
	}

	boithohome = getenv("BOITHOHOME");
	if (boithohome == NULL) {
		err(1, "getenv(BOITHOHOME)");
		return 105;
	}
	snprintf(program, sizeof(program), "%s/perl/repomod.pl", boithohome);

	if (setuid(0) != 0) {
		perror("Could not setuid(0)");
		return 97;
	}
	if (seteuid(0) != 0) {
		perror("Could not seteuid(0)");
		return 96;
	}

	if (strcmp(argv[1], "production") == 0) {
		ret = execlp(perlprog, "perl", program, "production", NULL, perlenv, NULL);
	} else if (strcmp(argv[1], "testing") == 0) {
		ret = execlp(perlprog, "perl", program, "testing", NULL, perlenv, NULL);
	} else if (strcmp(argv[1], "devel") == 0) {
		ret = execlp(perlprog, "perl", program, "devel", NULL, perlenv, NULL);
	} else {
		errx(1, "Wrong argument: %s", argv[1]);
	}

	perror("Unable to exec program");

	return 100;
}

