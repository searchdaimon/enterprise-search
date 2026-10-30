#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/sha1.h"
#include "../common/define.h"
#include "../common/url.h"

int main (int argc, char *argv[]) {

        char buff[1024];
	FILE *NYEFILE;
	SHA1Context sha;	
	char filname[128];

	int postcount, filcount, count;

	struct updateFormat NyeUrlPost;

	int err;

        if (argc < 2) {
                printf("Dette programet lager en nyurlfil fra en tekst fil som er \\n separert\n\n\tUsage: ./addManuellUrlFile directory til nye filer (må slutte på /) < urlfil\n");
                exit(0);
        }


	postcount = 0;
	filcount = 0;
	count = 0;
	//while (gets(buff) != NULL) {
	while (fgets(buff, sizeof(buff), stdin) != NULL) {


		if (buff[strlen(buff) -1] == '\n') {
			buff[strlen(buff) -1] = '\0';
		}

		//normalize
		if (!url_normalization(buff,sizeof(buff))) {
	                printf("Error: bad status from url_normalization. Url : \"%\"s\n",buff);
			continue;
	        }

		if (strncmp(buff,"ms:",3) == 0) {
			//if we have manually determined beforehand that they are ok they will start with "ms:"
			//we should not run any tests on whether they are ok or not
			//strcpy(buff,buff+3);
			memmove(buff,buff+3,strlen(buff));
			//printf("ms: \"%s\"\n",buff);
		}
		else if (!legalUrl(buff)) {
			//printf("bad url: \"%s\"\n",buff);
			continue;
		}
		
		//if count is larger than the max records we want in the file we set it to 0, so that a new one is opened
                if (postcount > 4000000) {
                        postcount = 0;
                }

		
		//if we have record count 0 we are at the beginning of a new file, and must open it
		if (postcount == 0) {
			//if this is not the first file we close the one we have open		
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
		Calculate the sha1 value
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

		
                ++postcount;		
		++count;
	}

	fclose(NYEFILE);

	printf("added %i urls\n",count);
}

