
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "resolveRedirects.h"
#include "../common/define.h"
#include "../common/reposetoryNET.h"
#include "../common/reposetory.h"

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

	GetFilPathForLot(filename, LotNr, subname);
	strcat(filename, "redirmap");

	if ((fp = fopen(filename, "r")) == NULL)
		err(1, "fopen(%s)", filename);

	char *text;
	size_t len;

	len = 1048576 * 5; //5MB
	if ((text = malloc(len)) == NULL) {
		perror("malloc");
		exit(1);
	}

	while (fread(&redir, sizeof(redir), 1, fp) == 1) {

		printf("%u => %u (reason: %hu)\n", redir.DocID, redir.redirectTo, redir.response);
		readHTMLNET(subname, redir.redirectTo, text, len);
		addResource(LotNr, subname, redir.DocID, text, strlen(text));
	//	printf("And got html: ''%s''\n", text);
	}

	free(text);

	fclose(fp);

	return 0;
}

