#include "../common/define.h"
#include <stdio.h>


int main (int argc, char *argv[]) {

        struct updateFormat updatePost;
        int i, y;
	FILE *nyeurler;

	printf("size of format %i\n",sizeof(updatePost));

        if (argc < 2) {
                printf("Gi meg noen uppdatfiler å lese\n");
                exit(0);
        }


	printf("Skal lese %i filer\n",argc -1);

	for (i=1; i < argc; i++) {

		printf("leser %s\n",argv[i]);

		if ((nyeurler = fopen(argv[i],"rb")) == NULL){
			perror(argv[i]);
			exit(1);
		}

		while (!feof(nyeurler)) {
		
			fread(&updatePost,sizeof(struct updateFormat),1,nyeurler);
			
			printf("%s %s %u ",updatePost.url,updatePost.linktext,updatePost.DocID_from);
			//printf("%u\n",updatePost.DocID_from);
			//debug vise sha1'en
			for (y=0;y<20;y++) {
				printf("%i",(int)updatePost.sha1[y]);
			}
			printf("\n");
			
		}		

		fclose(nyeurler);
	}



}
