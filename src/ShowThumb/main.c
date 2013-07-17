#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../cgi-util/cgi-util.h"


#include "../common/define.h"
#include "../common/debug.h"
#include "../common/lot.h"
#include "../common/reposetory.h"

//#define subname "www"


int main(int argc, char *argv[]) {

	int res;
	int i;
	char FilePath[255];
	FILE *REPOSETORY;
	char *imageBuf;

	unsigned char jpeghedder[] = { 0xFF,0xD8,0xFF,0xE0,0x00,0x10,0x4A,0x46 };
	unsigned char pnghedder[] = { 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };

	off_t iPointer;
	//unsigned long int iPointer;
	int iSize;
	int LotNr;
	char subname[64];


        if (getenv("QUERY_STRING") != NULL) {
        

        	 // Initialize the CGI lib
        	res = cgi_init();

        	// Was there an error initializing the CGI???
        	if (res != CGIERR_NONE) {
        	        printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
        	        exit(0);
        	}

        	if (cgi_getentrystr("P") == NULL) {
        	        perror("Did'n receive any query.");
        	}

		//iPointer = cgi_getentryint("P");
		//iPointer = atol(cgi_getentrystr("P"));
		iSize = cgi_getentryint("S");
		LotNr = cgi_getentryint("L");
		iPointer = strtoul(cgi_getentrystr("P"), (char **)NULL, 10);

		#ifdef BLACK_BOX
			strcpy(subname,cgi_getentrystr("C"));
		#else
			strcpy(subname,"www");
		#endif

	}
	else if (argc > 4) {
		//iPointer = 9695;
		//iSize = 2288;
		//LotNr = 14;

		LotNr = atoi(argv[1]); //L
		//iPointer = atoi(argv[2]); //P
		iPointer = strtoul(argv[2], (char **)NULL, 10);
		
		iSize = atoi(argv[3]); //s

		strcpy(subname,argv[4]);
		

	}
	else {
		printf("no query given.\n\n\tUsage: ShowThumb LotNr iPointer iSize subname\n");
		exit(1);
	}

	//må legge til størelsen på hedderen også
	//iPointer += sizeof(struct ReposetoryHeaderFormat);

	imageBuf = malloc(iSize);

	//printf("Content-type: text/html\n\n");


	GetFilPathForLot(FilePath,LotNr,subname);
	sprintf(FilePath,"%sreposetory",FilePath);	

	//printf("path %s\n",FilePath);

	if ((REPOSETORY = (FILE *)fopen64(FilePath,"rb")) == NULL) {
		fprintf(stderr,"Can't read udfile \"%s\".\n",FilePath);
                perror(FilePath);
                exit(1);
        }

	if (fseeko64(REPOSETORY,iPointer,SEEK_SET) != 0) {
		perror("Cant seek");
                exit(1);
	}

 
	if ((i=fread(imageBuf,iSize,1,REPOSETORY)) == -1) {
            perror("Cant read image");
            exit(1);
        }

	if (memcmp(jpeghedder,imageBuf,8) == 0) {
		printf("Content-type: image/jpeg\n\n");
		fwrite(imageBuf,iSize,1,stdout);
	}
	else if (memcmp(pnghedder,imageBuf,8) == 0) {
		printf("Content-type: image/png\n\n");
		fwrite(imageBuf,iSize,1,stdout);
	}
	else {
		fprintf(stderr,"dident hav a valid image hedder. Got %c%c%c%c%c%c%c\n",imageBuf[0],imageBuf[1],imageBuf[2],imageBuf[3],imageBuf[4],imageBuf[5],imageBuf[6],imageBuf[7]);
		fprintf(stderr,	"iSize: %i, LotNr: %i, iPointer: %u, subname: %s\n",iSize,LotNr,(unsigned int)iPointer,subname);

		printf("Location:http://www.boitho.com/images/spacer.jpg\n\n");
	} 

	fclose(REPOSETORY);

	free(imageBuf);
}
