#include "../common/lot.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>

int main () {

	char FilePath[255];
	unsigned int DocID;
	FILE *Thumbnale;
	char buff[6144];	//6kb
	int c;


	if (getenv("QUERY_STRING") == NULL) {
		printf("Content-type: text/html\n\n");
		printf("No DocID\n");
		exit(1);
	}
	DocID = atol(getenv("QUERY_STRING"));
	
	
	GetFilPathForThumbnaleByDocID(FilePath,DocID);


	if ((Thumbnale = fopen(FilePath,"rb")) == NULL) {
		printf("Content-type: text/html\n\n");
		perror(FilePath);
		exit(1);
	}

		
	printf("Content-type: image/jpeg\n\n");

	
	while ((c = getc(Thumbnale)) != EOF) {
		putchar(c);
	}
}
