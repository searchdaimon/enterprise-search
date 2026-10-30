#include <stdlib.h>
#include <string.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/ir.h"
#include "../common/revindex.h"
#include "../common/url.h"
#include "../common/bstr.h"



int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int lotNr;
	int i;
	unsigned int DocID;
	char domainArray[10][64];	
	char url[201];
	char domain[64];
	FILE *revindexFilesHa[NrOfDataDirectorys];
	char *cpnt, *cpntlast;
	unsigned int WordID;
	int bucket;
	unsigned short hits;
	int nr;
	int y;
	unsigned char lang;
	char **Data;
  	int Count, TokCount;
	int indexedNr;
	FILE *FH;

        if (argc < 3) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1 www");
               exit(0);
        }

	lotNr = atoi(argv[1]);
	char *subname = argv[2];
	DocID = 0;

	if ((FH = lotOpenFileNoCasheByLotNr(lotNr,"DocumentIndex","r",'r',subname)) == NULL) {
                perror("DocumentIndex");
        	exit(1);
        }
	fclose(FH);

	revindexFilesOpenLocal(revindexFilesHa,lotNr,"Url","wb",subname);

	indexedNr = 0;
	while (DIGetNext (&DocumentIndexPost,lotNr,&DocID,subname)) {


		if (DocumentIndexPost.Url[0] == '\0') {

		}
		else if (strncmp(DocumentIndexPost.Url,"http://",7) != 0) {
			//printf("no http: %s\n",DocumentIndexPost.Url);
		}
		else if (!find_domain(DocumentIndexPost.Url,domain,sizeof(domain))) {
			//printf("!find_domain %s\n",DocumentIndexPost.Url);
		}
		else {
			//printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);

			#ifdef DEBUG
			printf("url: \"%s\", DocID %u\n",DocumentIndexPost.Url,DocID);
			printf("dd: %s\n",domain);
			#endif



  			TokCount = split(domain, ".", &Data);

			hits = 1;
			for (i=(TokCount-1);i>=0;i--) {
			

			

				if (strcmp(Data[i],"www") != 0) {
				
					#ifdef DEBUG
					printf("\t\"%s\" %i\n",Data[i],hits);
					#endif
			
					WordID = crc32boitho(Data[i]);

                			bucket = WordID % NrOfDataDirectorys;

					//ToDo: just one hit. What if we have domains that contain the same word several times?
					//	such as ntnu.ntnu.no ?? Probably not that likely so we skip that problem for now
					nr = 1; 
                
        	        		fwrite(&DocID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
					//runarb: 13 may 2007. we have switched to using a number for language.
					//should then read this from DocumentIndex if it exists, but it isn't stored there
					//must tell IndexRes how we do it there
        	        		//fprintf(revindexFilesHa[bucket],"aa ");
					lang = 0;
					fwrite(&lang,sizeof(unsigned char),1,revindexFilesHa[bucket]);

        	        		fwrite(&WordID,sizeof(unsigned int),1,revindexFilesHa[bucket]);
        	        		fwrite(&nr,sizeof(unsigned int),1,revindexFilesHa[bucket]);

        	        		for(y=0;y<nr;y++) {
        	        		        //printf("\thits %i\n",pagewords.revIndex[i].hits[y]);
        		        	        fwrite(&hits,sizeof(unsigned short),1,revindexFilesHa[bucket]);
			                }
				
			
					++hits;
				}
				else {
					//printf("\tskipt: %s %i\n",Data[i],hits);

					++hits;
				}

				++indexedNr;
			}
  			FreeSplitList(Data);

			
		}
	}


	//DIClose();

	printf("indexed %i\n",indexedNr);
}

