#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/poprank.h"


int main (int argc, char *argv[]) {
	FILE *POPFILE;
	int DocID;
	int LastDocID = -1;
	int increasement = 0; 
	int i;
	struct popl popha;

	if (argc < 2) {
		printf("No popfile given\n\nUsage:\n\t./PoprankMerge popfil\n\n");
		exit(1);
	}
	if ((POPFILE = fopen(argv[1],"rb")) == NULL) {
		perror(argv[1]);
	}
	popopen(&popha,"/home/boitho/config/popindex");
	
	while (! feof(POPFILE)) {
	//for (i=0;i<10;i++) {
		fread(&DocID,sizeof(DocID),1,POPFILE);
		//printf("%i\n",DocID);
	
		if (DocID == LastDocID) {
			increasement++;
		}
		else {

			//printf("%i - %i\n",LastDocID,increasement);

			popadd(&popha,LastDocID,increasement);		
	
			increasement = 1;
			LastDocID = DocID;
		}
		//printf("%i\n",DocID);

	}

	popclose(&popha);
}


