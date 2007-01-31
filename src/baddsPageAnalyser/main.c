#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>


#include "../IndexerRes/IndexerRes.h"


int main (int argc, char *argv[]) {

        int lotNr;
	char lotServer[64];
	int pageCount;
	int i;

	FILE *ADULTWEIGHTFH;
	unsigned char awvalue;

        unsigned int FiltetTime;
        unsigned int FileOffset;
	off_t DocIDPlace;

        char htmlcompressdbuffer[524288];  //0.5 mb
        char imagebuffer[524288];  //0.5 mb
	
	int httpResponsCodes[nrOfHttpResponsCodes];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	struct DocumentIndexFormat DocumentIndexPost;
	unsigned long int radress;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	struct adultFormat adult;
	unsigned int lastIndexTime;

        if (argc < 2) {
                printf("Dette programet analyserer en url. Usage:\n\tbaddsPageAnalyser url\n");
                exit(0);
        }

	char url[1024];

	char *HtmlBuffer = NULL;


	adultLoad(&adult);
	
	strncpy(url,argv[1],sizeof(url));

	httpGet(&HtmlBuffer,url);

					//setter opp en alarm slik at run_html_parser blir avbrut hvis den henger seg 
	alarm_got_raised = 0;
	signal(SIGALRM, html_parser_timout);
					
	alarm( 5 );
	//parser htmlen

	run_html_parser(url , HtmlBuffer, strlen(HtmlBuffer), fn );
	alarm( 0);
	if(alarm_got_raised) {
		printf("run_html_parser did time out.\n");
	}
	else {

	}

	unsigned short AdultWeight;
	wordsMakeRevIndex(&adult,&AdultWeight);

	//sortere på forekomst
	pagewordsSortOnOccurrence();

	for(i=0;i<pagewords.revIndexnr;i++) {
                printf("%lu forekomster %i (+1) '%s'\n",pagewords.revIndex[i].WordID,pagewords.revIndex[i].nr,pagewords.revIndex[i].word);

	}

	return 0;
}




