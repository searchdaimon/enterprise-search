#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/define.h"

int main (int argc, char *argv[]) {

	FILE *LINKDBFILE;



	struct linkdb_block linkdbPost;
	struct DocumentIndexFormat FromDocumentIndexPost;
	struct DocumentIndexFormat ToDocumentIndexPost;

	int FoundFromInfo;
	int FoundToInfo;

        if (argc < 2) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./BrankCalculate linkdb\n");
                exit(0);
        }

	if ((LINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }

	
	while (!feof(LINKDBFILE)) {
		fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE);
		printf("%u -> %u\n",linkdbPost.DocID_from,linkdbPost.DocID_to);
	}	



	fclose(LINKDBFILE);
}

