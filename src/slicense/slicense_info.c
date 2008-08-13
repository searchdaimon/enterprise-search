#include <stdlib.h>
#include <stdio.h>

#include "license.h"

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s license\n", argv[0]);
		exit(1);
	}

	unsigned short int users;
	unsigned int serial;

	if (!get_licenseinfo(argv[1], &serial, &users)) {
		printf("valid: no\n");
		exit(1);
	}
	printf("valid: yes\n");
	printf("serial: %d\n", serial);
	printf("users: %d\n", users);

	return 0;
}

