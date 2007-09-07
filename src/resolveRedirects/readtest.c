
#include <stdio.h>

#include "resolveRedirects.h"

int
main(int argc, char **argv)
{
	FILE *fp;
	struct redirects redir;
	char *subname;
	int LotNr;
	char filename[1024];

	if (argc < 2)
		err(1, "Usage: ./readtest lotnr subname");

	LotNr = atoi(argv[1]);
	subname = argv[2];

	if ((fp = fopen(argv[1], "r")) == NULL)
		err(1, "fopen()");

	while (fread(&redir, sizeof(redir), 1, fp) == 1) {
		printf("%u => %u (reason: %hu)\n", redir.DocID, redir.redirectTo, redir.response);
	}

	fclose(fp);

	return 0;
}

