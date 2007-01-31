#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/url.h"
#include "../common/crc32.h"
#include "../parse_summary/summary.h"

#include <stdlib.h>
#include <zlib.h>




main (int argc, char *argv[]) {
	

	int LotNr;
	char lotPath[255];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	unsigned long int radress;

	char HtmlBuffer[524288];
	char imagebuffer[524288];
	char WorkBuff[524288];
	char subname[maxSubnameLength];
	uLong HtmlBufferSize,WorkBuffSize;
	int error,y,i,spacecount;
	int count;
	int offset;
	unsigned char rank;
	unsigned pageCrc32;
	FILE *SFH, *RANKFH;
	struct DocumentIndexFormat DocumentIndexPost;

	if (argc < 3) {
		printf("Error ingen lotnr spesifisert.\n\nEksempel på bruk for å lese lot 2:\n\trread 2 www\n");
		exit(1);
	}

	LotNr = atoi(argv[1]);
	strncpy(subname,argv[2],sizeof(subname) -1);

	printf("lotnr %i\n",LotNr);

	GetFilPathForLot(lotPath,LotNr,subname);
	printf("Opning lot at: %s for %s\n",lotPath,subname);

		//sjekker om vi har nokk palss
                if (!lotHasSufficientSpace(LotNr,4096,subname)) {
                        printf("insufficient disk space\n");
                        exit(1);
                }

	SFH = lotOpenFileNoCasheByLotNr(LotNr,"summary","wb",'r',subname);
	RANKFH = lotOpenFileNoCasheByLotNr(LotNr,"Brank","rb",'r',subname);

	//loppergjenom alle
	count = 0;
	while (rGetNext(LotNr,&ReposetoryHeader,WorkBuff,sizeof(WorkBuff),imagebuffer,&radress,0,0,subname)) {

		if ((ReposetoryHeader.response == 200) || (url_havpri(ReposetoryHeader.url))) {



			//finner rank for siden
			offset = ((ReposetoryHeader.DocID - LotDocIDOfset(LotNr)) * sizeof(unsigned char));
			fseek(RANKFH,offset,SEEK_SET);				
			fread(&rank,sizeof(unsigned char),1,RANKFH);
	
			//hvsi ranken er for liten gir vi faen i å lage summery for den
			if (rank > 40) {
			
				WorkBuffSize = ReposetoryHeader.htmlSize;
				HtmlBufferSize = sizeof(HtmlBuffer);
				if ( (error = uncompress((Bytef *)&HtmlBuffer,&HtmlBufferSize,WorkBuff,WorkBuffSize)) != 0) {
					#ifndef NOWARNINGS
                	        		printf("uncompress error. Code: %i\n",error);
					#endif
					continue;
                		}
                	

                	        	//er det ikke \0 med i buferren ??
        	                HtmlBuffer[HtmlBufferSize] = '\0';

				pageCrc32 = crc32boitho(HtmlBuffer);


				char        *titleaa, *body, *metakeyw, *metadesc = NULL;

				generate_summary( HtmlBuffer, HtmlBufferSize, &titleaa, &body, &metakeyw,&metadesc );

				printf("tit %s\n",titleaa);

				sprintf(WorkBuff,"%s\n%s\n%s",titleaa,metadesc,body);

				if (titleaa != NULL) free(titleaa);
                		if (body != NULL) free(body);
                		if (metakeyw != NULL) free(metakeyw);
                		if (metadesc != NULL) free(metadesc);


				WorkBuffSize = strlen(WorkBuff);
				HtmlBufferSize = sizeof(HtmlBuffer);
				if ( (error = compress((Bytef *)&HtmlBuffer,&HtmlBufferSize,WorkBuff,WorkBuffSize)) != 0) {
					printf("compress error. Code: %i\n",error);
				}	
				else {
	
					DIRead(&DocumentIndexPost,ReposetoryHeader.DocID,subname);
		
					DocumentIndexPost.SummaryPointer = ftell(SFH);
					DocumentIndexPost.SummarySize = (sizeof(ReposetoryHeader.DocID) + HtmlBufferSize);					
					DocumentIndexPost.crc32 = pageCrc32;
			
					DIWrite(&DocumentIndexPost,ReposetoryHeader.DocID,subname);

					fwrite(&ReposetoryHeader.DocID,sizeof(ReposetoryHeader.DocID),1,SFH);
					fwrite(&HtmlBuffer,HtmlBufferSize,sizeof(char),SFH);	

    
					
				}




				printf("DocId: %u url: %s res %hi htmls %hi time %lu. Rank %u\n",ReposetoryHeader.DocID,ReposetoryHeader.url,ReposetoryHeader.response,ReposetoryHeader.htmlSize,ReposetoryHeader.time,rank);

				//if ((count % 1000) == 0) {
				//	printf("%i\n",count);
				//}			

				//if (count == 10000) {
				//	exit(1);
				//}

				++count;

		
			}
			else {

			}


		}
		//printf("################################\n%s##############################\n",htmlbuffer);

		
	}
	
	fclose(SFH);
	fclose(RANKFH);
}
