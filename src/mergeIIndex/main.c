#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "../common/iindex.h"


int main (int argc, char *argv[]) {

	//printf("argc %i\n",argc);

	int DocIDcount = 0;

        if (argc == 7) {

		//lager for bare en bøtte
		
		int startIndex = atoi(argv[1]);
        	int stoppIndex = atoi(argv[2]) +1;	
		char *type = argv[3];
		char *lang = argv[4];
		char *subname = argv[5];
		int bucket = atol(argv[6]);
	
		mergei(bucket,startIndex,stoppIndex,type,lang,subname,&DocIDcount);

		printf("DocIDcount: %i\n",DocIDcount);


	}
	else if (argc == 6) {

		//skal lage for alle bøttene
		int i;

		int startIndex = atoi(argv[1]);
        	int stoppIndex = atoi(argv[2]) +1;	
		char *type = argv[3];
		char *lang = argv[4];
		char *subname = argv[5];
		
		for (i=0;i<=63;i++) {
			#ifdef DEBUG
			printf("bucket: %i\n",i);
			#endif
			mergei(i,startIndex,stoppIndex,type,lang,subname,&DocIDcount);
		}

		printf("DocIDcount: %i (/64)\n",DocIDcount);

	}
	else {
                printf("Dette programet printer ut en iindex.\n\n");
		printf("\tUse:\n\n\t./mergeIIndex fralot tillot type (Main | Anchor) språk subname [bucket]\n\n");
                exit(0);
        }


	return 0;

}

