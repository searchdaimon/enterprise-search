#include <stdio.h>
#include "../common/sha1.h"
#include "../common/define.h"

int main (int argc, char *argv[]) {

        char buff[1024];
	FILE *NYEFILE;
	SHA1Context sha;	
	char filname[128];

	int postcount, filcount;

	struct updateFormat NyeUrlPost;

	int err;

        if (argc < 2) {
                printf("Dette programet lager en nyurlfil fra en tekst fil som er /\n separert\n\n\tUsage: ./addManuellUrlFile directory til nye filer (må slutte på /) < urlfil\n");
                exit(0);
        }


	postcount = 0;
	filcount = 0;
	while (gets(buff) != NULL) {

		//hvis count er større en maks poster vi skal ha i filen setter vi den til 0, slik at en ny blir åpnet
                if (postcount > 4000000) {
                        postcount = 0;
                }

		
		//hvsi vi har post cont 0 er vi på begyndelsen til en ny fil, og må åpne denne
		if (postcount == 0) {
			//hvis dette ikke er den første filen lukker vi den vi har åpen		
			if (filcount != 0) {
				fclose(NYEFILE);
			}

			sprintf(filname,"%s%i",argv[1],filcount);

			if ((NYEFILE = fopen(filname,"wb")) == NULL) {
                		perror(filname);
                		exit(1);
		        }
			filcount++;
		}

		//printf("%s\n",buff);

		strncpy(NyeUrlPost.url,buff,sizeof(NyeUrlPost.url));

		/***********************************************************************
		Kalkulerer sha1 verdi
		************************************************************************/
		err = SHA1Reset(&sha);
        	if (err) {
            		printf("SHA1Reset Error %d.\n", err );
        	}


        	err = SHA1Input(&sha, (const unsigned char *) NyeUrlPost.url, strlen(NyeUrlPost.url));
        	if (err) {
                	printf("SHA1Input Error %d.\n", err );
        	}

        	err = SHA1Result(&sha, NyeUrlPost.sha1);
        	if (err) {
            		printf("SHA1Result Error %d, could not compute message digest.\n", err );
        	}

        	//printf("%s",Message_Digest);
		/************************************************************************/

		NyeUrlPost.linktext[0] = '\0';
		NyeUrlPost.DocID_from = 0;	

		fwrite(&NyeUrlPost,sizeof(struct updateFormat),1,NYEFILE);

		
                postcount++;		
	}

	fclose(NYEFILE);
}

