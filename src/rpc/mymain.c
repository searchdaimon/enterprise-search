
#include <stdlib.h>
#include <stdio.h>

#include "../suggest/suggest.h"
#include "main.h"

int
mymain(int argc, char **argv)
{
	int words;
	struct suggest_data *sd;

	if ((sd = suggest_init()) == NULL) {
		fprintf(stderr, "Could not initiate suggest data\n");
		exit(1);
	}
	printf("Starting suggest daemon.\n");
	words = suggest_read_frequency(sd, "../suggest/UnikeTermerMedForekomst.ENG");
	suggest_most_used(sd);
	suggest_data = sd;
	printf("Ready to play.\n");

	return 0;
}

