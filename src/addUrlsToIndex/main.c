#include "../common/define.h"
#include "../common/DocumentIndex.h"

#include <stdio.h>
#include <string.h>


int main (int argc, char *argv[]) {

        FILE *UDFILE;
        struct udfileFormat udfilePost;
	char *cpoinet;
	struct DocumentIndexFormat DocumentIndexPost;
	int count;

	char *pointer;
	int i;
        //check that we were told which file to use
        if (argc < 2) {
                printf("Usage: ./addanchors UDFILE\n\n");
                exit(1);
        }

        if ((UDFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read UDFILE ");
                perror(argv[1]);
                exit(1);
        }

	count = 0;
        while(!feof(UDFILE)) {
                fread(&udfilePost,sizeof(struct udfileFormat),1,UDFILE);

		//does not look like this is null terminated, but space padded.
		udfilePost.url[152] = '\0';
		if ((cpoinet = (char *)strchr(udfilePost.url,' ')) != NULL) {
			*cpoinet = '\0';
		}


		//printf("%u - %s\n",udfilePost.DocID,udfilePost.url);


		/*
        	initialize the structure. We clear everything in it by accessing memory directly.
        	This is done so that we don't get any garbage after \0, since that would
        	also be stored.

        	We don't need to know the format of the structure.
        	*/
        	
        	pointer = (char *)&DocumentIndexPost;
        	for(i=0; i < sizeof(DocumentIndexPost); i++) {
        	        //printf("%c\n",pointer[i]);
        	        pointer[i] = ' ';
       		}

		
		strncpy(DocumentIndexPost.Url,udfilePost.url,sizeof(DocumentIndexPost.Url));
		DocumentIndexPost.htmlSize = 0;
		DocumentIndexPost.imageSize = 0;
		DocumentIndexPost.IPAddress = 0;
		DocumentIndexPost.response = 0;
		DocumentIndexPost.AntallFeiledeCrawl = 0;


		DIWrite(&DocumentIndexPost,udfilePost.DocID);
		
		if ((count % 100000) == 0) {printf("%i\n",count);};
		count++;
	}

	DIClose();
}



