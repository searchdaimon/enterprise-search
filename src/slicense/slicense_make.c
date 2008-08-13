#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include "license.h"

int main(int argc, char **argv) {
#ifndef WITH_MAKE_LICENSE
	fprintf(stderr, "Not compiled with WITH_MAKE_LICENSE\n");
	exit(1);
#else
	if (argc < 2) {
		fprintf(stderr, "Usage: %s num_licenses\n", argv[0]);
		exit(1);
	}

	unsigned short int cmd_users;
	cmd_users = (unsigned short int) strtoul(argv[1], NULL, 10);

	char *key;
	key = make_license(cmd_users);
	key = human_readable_key(key);

	unsigned short int users;
	unsigned int serial;
	if (!get_licenseinfo(key, &serial, &users))
		errx(1, "Invalid license key generated");

	printf("key: %s\n", key);
	printf("serial: %d\n", serial);
	printf("users: %d\n", users);

	free(key);

	return 0;
#endif
}
