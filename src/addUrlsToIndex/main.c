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
        //tester for at vi har fåt hvilken fil vi skal bruke
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

		//ser ikke ut som om dette er null terminert., menspacpaddet.
		udfilePost.url[152] = '\0';
		if ((cpoinet = (char *)strchr(udfilePost.url,' ')) != NULL) {
			*cpoinet = '\0';
		}


		//printf("%u - %s\n",udfilePost.DocID,udfilePost.url);


		/*
        	inaliserer strukten. Vi sletter alt i den ved å aksesere minne direkte.
        	Dette gjøres slik at vi ikke får noe støy etter \0, da det også vil bli
        	lagret.

        	Vi ikke trenger å kjene formatet på strukturen.
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



