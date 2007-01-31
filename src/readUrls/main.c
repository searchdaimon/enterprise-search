
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/url.h"

int main (int argc, char *argv[]) {

	FILE *UPDATEFILE;
	char buf[1024];
	int len;

	//tester for at vi har fåt hvilken fil vi skal bruke
	if (argc < 2) {
		printf("Usage: ./addanchors anchorfile\n\n\tanchorfile, fil med tekster på linker\n\n");
		exit(1);
	}

	if ((UPDATEFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read anchorfile ");
                perror(argv[1]);
                exit(1);
        }

	while(fgets(buf,sizeof(buf),UPDATEFILE) != NULL) {

		len = strlen(buf);
		--len;
		if (buf[len] == '\n') {

			buf[len] = '\0';
			if (gyldig_url(buf)) {
				printf("ok: \"%s\"\n",buf);	
			}
			else {
				printf("bad: \"%s\"\n",buf);	
			}
		}
		else {
			printf("overflowed buffer. Las char vas char nr %i as \"%c\"\n",len,buf[len]);
		}
		
	}
	fclose(UPDATEFILE);
	
}



