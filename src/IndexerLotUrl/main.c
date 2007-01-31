#include <stdlib.h>
#include <string.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/ir.h"
#include "../common/revindex.h"

#define subname "www"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int lotNr;
	unsigned int DocID;
	char domainArray[10][64];	
	char url[201];
	char domain[64];
	FILE *revindexFilesHa[NrOfDataDirectorys];
	char *cpnt, *cpntlast;
	unsigned long WordID;
	int bucket;
	unsigned short hits;
	int nr;
	int y;

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	lotNr = atoi(argv[1]);
	DocID = 0;

	revindexFilesOpenLocal(revindexFilesHa,lotNr,"Url","wb",subname);


	while (DIGetNext (&DocumentIndexPost,lotNr,&DocID,subname)) {
		if (DocumentIndexPost.Url[0] == '\0') {

		}
		else if (strncmp(DocumentIndexPost.Url,"http://",7) != 0) {
			//printf("no http: %s\n",DocumentIndexPost.Url);
		}
		else if (!find_domain(DocumentIndexPost.Url,domain)) {
			//printf("!find_domain %s\n",DocumentIndexPost.Url);
		}
		else {
			//printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);

			

			//printf("dd: %s\n",domain);
			
			cpntlast = domain;

			hits = 1;
			while ((cpnt = strrchr(cpntlast,'.')) != NULL) {
	
				cpntlast[cpnt - cpntlast] = '\0';;			
			
				++cpnt;

	
			
				if ((strcmp(cpnt,"www") != 0) && (!isStoppWord(cpnt))) {
				
					//printf("\t%s %i\n",cpnt,hits);

			
					WordID = crc32boitho(cpnt);

                			bucket = WordID % NrOfDataDirectorys;

					//ToDo: bare en hit. Hva om vi har doener som inneholder samme ord flere ganger?
					//	som ntnu.ntnu.no ?? Kanje ikke så sansynelig så skipper det problemet for nå
					nr = 1; 
                
        	        		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
        	        		fprintf(revindexFilesHa[bucket],"aa ");

        	        		fwrite(&WordID,sizeof(unsigned long),1,revindexFilesHa[bucket]);
        	        		fwrite(&nr,sizeof(unsigned long),1,revindexFilesHa[bucket]);

        	        		for(y=0;y<nr;y++) {
        	        		        //printf("\thits %i\n",pagewords.revIndex[i].hits[y]);
        		        	        fwrite(&hits,sizeof(unsigned short),1,revindexFilesHa[bucket]);
			                }
				
			
					++hits;
				}
				else {
					//printf("\tskipt: %s %i\n",cpnt,hits);

					++hits;
				}


			}

			
		}
	}


	//DIClose();


}

