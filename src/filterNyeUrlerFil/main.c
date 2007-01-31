#include "../common/define.h"
#include <stdio.h>


int main (int argc, char *argv[]) {

        struct updateFormat updatePost;
        int i, y;
	FILE *nyeurler;
	char name_FilterInn[512];
	char name_FilterOut[512];
	FILE *FH_FilterInn,*FH_FilterOut;

	printf("size of format %i\n",sizeof(updatePost));

        if (argc < 2) {
                printf("Gi meg noen uppdatfiler å lese\n");
                exit(0);
        }


	printf("Skal lese %i filer\n",argc);

	for (i=1; i < argc; i++) {

		//printf("leser %s\n",argv[i]);

		nyeurler = fopen(argv[i],"rb");

		//lager navn for de filene som sal inneholde resultatet

		sprintf(name_FilterInn,"%s_FilterInn",argv[i]);
		sprintf(name_FilterOut,"%s_FilterOut",argv[i]);

		FH_FilterInn = fopen(name_FilterInn,"wb");
		FH_FilterOut = fopen(name_FilterOut,"wb");

		printf("filter from %s to %s / %s\n",argv[i],name_FilterInn,name_FilterOut);

		while (!feof(nyeurler)) {
		
			fread(&updatePost,sizeof(struct updateFormat),1,nyeurler);
			
			if (strstr(updatePost.url,".no/") != NULL) {


				//printf("%s %s %u ",updatePost.url,updatePost.linktext,updatePost.DocID_from);
				//printf("%u\n",updatePost.DocID_from);
				//debug vise sha1'en
				//for (y=0;y<20;y++) {
				//	printf("%i",(int)updatePost.sha1[y]);
				//}
				//printf("\n");

				if ((fwrite(&updatePost,sizeof(struct updateFormat),1,FH_FilterInn)) == 0){
					perror(name_FilterInn);
					exit(1);
				}
			}
			else {
				if ((fwrite(&updatePost,sizeof(struct updateFormat),1,FH_FilterOut)) == 0){
                                        perror(name_FilterOut);
					exit(1);
                                }

			}
			
		}		

		fclose(nyeurler);
		unlink(argv[i]);

		fclose(FH_FilterInn);
		fclose(FH_FilterOut);
	}



}
