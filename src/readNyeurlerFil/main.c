

#include <stdio.h>


int main (int argc, char *argv[]) {

	//burde hat annte feil navn her, men jeg er trøtt :)
        FILE *LINKDBFILE;

	//A20 A200 A50
	struct update_blocka
	{
    		unsigned char       sha1[20];
    		unsigned char       url[200];
    		unsigned char       linktext[50];
   	//	unsigned int        DocID_from;
	};

	
        struct update_blocka nyeurlerPost;
        char *cpointer;

        if (argc < 2) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./BrankCalculate linkdb\n");
                exit(0);
        }

        if ((LINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read\n");
                perror(argv[1]);
                exit(1);
        }

	//printf("sizeof: %i\n",sizeof(nyeurlerPost));

        while (!feof(LINKDBFILE)) {
                fread(&nyeurlerPost,sizeof(nyeurlerPost),1,LINKDBFILE);
		
		if ((cpointer = strstr(nyeurlerPost.url," ")) != NULL) {
			*cpointer = '\0';
			printf("%s\n",nyeurlerPost.url);
		}
       
		printf("%s\n",nyeurlerPost.url);
        }



        fclose(LINKDBFILE);
}

