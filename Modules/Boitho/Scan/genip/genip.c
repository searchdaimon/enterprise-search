/*
 * $Id: genip.c,v 1.1 2007/06/06 22:04:08 dagurval Exp $
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "targets.h"

#define	BUFFER_MAX_LEN		128

enum	method { M_NMAP, M_IPRANGE };

int	count = 0;

void usage (char *name) {
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "    %s [-i <filename>] [<target-spec] [...]]\n", name);
	fprintf(stderr, "    %s -r <ip-address> <ip-address>\n", name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    -h  Display usage information.\n");
	fprintf(stderr, "    -i  Read target specifications from the given file. (\"-\" = Standard in)\n");
	fprintf(stderr, "    -r  Specify IP range mode.  Two, and only two, IP addresses must be\n");
	fprintf(stderr, "        specified in dotted-quad notation.\n");
}

void
process_file (char *filename) {
	TARGET_LIST	targets;
	struct in_addr	target;
	FILE		*infile;
	char		buffer[BUFFER_MAX_LEN], *cp, *tok_store;
	int		line = 0;

	if (strcasecmp(filename, "-") == 0) {
		if ((infile = fdopen(0, "r")) == NULL)
			err(EX_IOERR, "fdopen() failed");
	} else {
		if ((infile = fopen(filename, "r")) == NULL)
			err(EX_IOERR, "fopen(%s) failed", filename);
	}

	while (fgets(buffer, BUFFER_MAX_LEN-1, infile)) {
		line++;
		cp = strtok_r(buffer, " \t\r\n", &tok_store);
		while (cp != NULL && cp[0] != '#') {
			if (init_target_list(&targets, cp)) {
				while (next_target(&targets, &target)) {
					printf("%s\n", inet_ntoa(target));
					count++;
				}
			} else
				errx(EX_USAGE,
				    "Bad target specification on line %d: %s",
				    line, target_list_err());

			cp = strtok_r(NULL, " \t\r\n", &tok_store);
		}
		continue;

	}

	if (fclose(infile) != 0)
		warn("fclose(%s) failed", filename);
}

int main (int argc, char *argv[]) {
	int		i, method = M_NMAP;
	TARGET_LIST	targets;
	struct in_addr	target;
	u_int32_t		start, end;

	while ((i = getopt(argc, argv, "hri:")) != -1) {
		switch (i) {
		case 'h':
			usage(argv[0]);
			exit(EX_OK);
			/* NOTREACHED */
		case 'r':
			method = M_IPRANGE;
			break;
		case 'i':
			process_file(optarg);
			break;
		default:
			exit(EX_USAGE);
		}
	}
	argc -= optind;
	argv += optind;

	if (method == M_IPRANGE) {
		if (argc != 2)
			errx(EX_USAGE, "Not enought targets for range specified!");

		if (!inet_aton(argv[0], &target))
			errx(EX_USAGE, "Invalid IP address!");
		start = ntohl(target.s_addr);

		if (!inet_aton(argv[1], &target))
			errx(EX_USAGE, "Invalid IP address!");
		end = ntohl(target.s_addr);

		for (i = start; i <= end; i++) {
			target.s_addr = htonl(i);
			printf("%s\n", inet_ntoa(target));
			count++;
		}
	} else {
		for (i = 0; i < argc; i++) {
			if (init_target_list(&targets, argv[i])) {
				while (next_target(&targets, &target)) {
					printf("%s\n", inet_ntoa(target));
					count++;
				}
			} else
				errx(EX_USAGE,
				    "Bad target specification on line: %s",
				    target_list_err());
		}
	}

	if (count == 0)
		errx(EX_USAGE, "No targets specified!");

	return(EX_OK);
}
