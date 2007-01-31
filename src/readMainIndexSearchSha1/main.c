#include "../common/define.h"
#include "../common/lot.h"

#include <stdio.h>


int main (int argc, char *argv[]) {

        struct mainIndexFormat mainIndexPost;
        FILE *MainIndex;
	int i;
	unsigned int DocID;
	unsigned char sha1[20] = {23,203,68,117,252,36,189,197,53,125,230,117,169,62,158,31,182,84,35,29};
        if (argc < 3) {
                printf("Gi meg en MainIndex\n");
                exit(0);
        }


        MainIndex = fopen(argv[1],"rb");
	DocID = atoi(argv[2]);

        while (!feof(MainIndex)) {
		fread(&mainIndexPost,sizeof(struct mainIndexFormat),1,MainIndex);

		if (memcmp(mainIndexPost.sha1,sha1,20) == 0) {
			printf("%u-%i ",mainIndexPost.DocID,rLotForDOCid(mainIndexPost.DocID));
			for (i=0;i<20;i++) {
                                printf("%i",(int)mainIndexPost.sha1[i]);
                	}
			printf("\n");
			//exit(1);
		}
	}

	fclose(MainIndex);

}
