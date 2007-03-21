/*
 * Wrapper for fdisk so the raid scripts can get disk information without being
 * root.
 *
 * Written by: Eirik A. Nygaard
 *             15.12.2006
 */

#include <sys/types.h>

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>


int
main(int argc, char **argv)
{
	char cmd[1024];
	char *p;
	
	if (argc != 2) {
		fprintf(stderr, "Syntax: fdiskwrap device\n");
		exit(1);
	}

	if (setuid(0) != 0) {
		perror("Unable to setuid(0)\n");
		exit(2);
	}

	/* verify input */
	for (p = argv[1]; *p; p++) {
		if (!isalnum(*p)) {
			fprintf(stderr, "Illegal argument.\n");
			exit(10);
		}
	}
	
	snprintf(cmd, sizeof(cmd), "/sbin/fdisk -l /dev/%s", argv[1]);
	system(cmd);

	return(0);
}

