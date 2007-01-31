#include "../common/define.h"
#include <stdio.h>


int main (int argc, char *argv[]) {

        struct mainIndexFormat mainIndexPost;
        FILE *MainIndex;
	int i;
	unsigned int DocID;

        if (argc < 3) {
                printf("Gi meg en MainIndex\n");
                exit(0);
        }


        MainIndex = fopen(argv[1],"rb");
	DocID = atoi(argv[2]);

        while (!feof(MainIndex)) {
		fread(&mainIndexPost,sizeof(struct mainIndexFormat),1,MainIndex);

		if (DocID == mainIndexPost.DocID) {
			printf("%u ",mainIndexPost.DocID);
			for (i=0;i<20;i++) {
                                printf("%u,",(unsigned int)mainIndexPost.sha1[i]);
                	}
			printf("\n");
			exit(1);
		}
	}

	fclose(MainIndex);

}
