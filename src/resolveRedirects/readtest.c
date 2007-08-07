
#include <stdio.h>

#include "resolveRedirects.h"

int
main(int argc, char **argv)
{
	FILE *fp;
	struct redirects redir;

	if (argc < 2)
		err(1, "Usage: ./readtest mappingsfile");

	if ((fp = fopen(argv[1], "r")) == NULL)
		err(1, "fopen()");

	while (fread(&redir, sizeof(redir), 1, fp) == 1) {
		printf("%u => %u (reason: %hu)\n", redir.DocID, redir.redirectTo, redir.response);
	}

	return 0;
}

