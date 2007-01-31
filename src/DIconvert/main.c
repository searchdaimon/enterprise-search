#include "../common/lot.h"

struct DocumentIndexFormatOLD {
        char Url[201];
        char Sprok[4];
        unsigned short Offensive_code;
        char Dokumenttype[4];
        unsigned int CrawleDato;
        unsigned short AntallFeiledeCrawl;
        unsigned short AdultWeight;
        unsigned int RepositoryPointer;
        unsigned short htmlSize;
        unsigned short imageSize;
        unsigned int ResourcePointer;
        unsigned short ResourceSize;
        unsigned long int IPAddress;
        unsigned short response;
        unsigned short userID;
        double clientVersion;
};

struct DocumentIndexFormatNEW {
        char Url[201];
        char Sprok[4];
        unsigned short Offensive_code;
        char Dokumenttype[4];
        unsigned int CrawleDato;
        unsigned short AntallFeiledeCrawl;
        unsigned short AdultWeight;
        unsigned int RepositoryPointer;
        unsigned short htmlSize;
        unsigned short imageSize;
        unsigned int ResourcePointer;
        unsigned short ResourceSize;
        unsigned long int IPAddress;
        unsigned short response;
        unsigned short userID;
        double clientVersion;
	//new: 28.mars 2006
	unsigned char nrOfOutLinks;
	unsigned int SummaryPointer;
	unsigned short SummarySize;
	unsigned int CRC32;
};


main (int argc, char *argv[]) {

	struct DocumentIndexFormatOLD DocumentIndexOLD;
	struct DocumentIndexFormatNEW DocumentIndexNEW;

	char DIOLDPatch[512], DITEMPPatch[512];
	char subname[64];
	int lotNr;
	
	FILE *DIFHOLD,*DIFHNEW;

	if (argc < 3) {
                printf("Dette programet indekserer en lot. Usage:\n\tIndexerLot lotNr subname\n");
                exit(0);
        }

        lotNr = atoi(argv[1]);
        strncpy(subname,argv[2],sizeof(subname) -1);
	
	GetFilPathForLot(DIOLDPatch,lotNr,subname);
	
	sprintf(DIOLDPatch,"%sDocumentIndex",DIOLDPatch);
	sprintf(DITEMPPatch,"%s_OLD",DIOLDPatch);


	//tester om vi har noen DI
	if (fopen(DIOLDPatch,"r") == NULL) {
		printf("dosent hav any DI\n");
		exit(1);
	}

	printf("%s -> %s\n",DIOLDPatch,DITEMPPatch);

	if (rename(DIOLDPatch,DITEMPPatch) != 0) {
		perror("rename");
		exit(1);
	}

	DIFHOLD = lotOpenFileNoCasheByLotNr(lotNr,"DocumentIndex_OLD","rb",'r',subname);
	DIFHNEW = lotOpenFileNoCasheByLotNr(lotNr,"DocumentIndex","wb",'r',subname);
	

	while (!feof(DIFHOLD)) {

		if (fread(&DocumentIndexOLD,sizeof(DocumentIndexOLD),1,DIFHOLD) != 1 ) {
			//perror("fread");
			//exit(1);
		}

		/******** memo copy ***********************/
		memcpy(DocumentIndexNEW.Url,		DocumentIndexOLD.Url,201);
		memcpy(DocumentIndexNEW.Sprok,		DocumentIndexOLD.Sprok,4);
		memcpy(DocumentIndexNEW.Dokumenttype,	DocumentIndexOLD.Dokumenttype,4);

		DocumentIndexNEW.Offensive_code 	= DocumentIndexOLD.Offensive_code;
		DocumentIndexNEW.CrawleDato 		= DocumentIndexOLD.CrawleDato;
		DocumentIndexNEW.AntallFeiledeCrawl	= DocumentIndexOLD.AntallFeiledeCrawl;
		DocumentIndexNEW.AdultWeight		= DocumentIndexOLD.AdultWeight;
		DocumentIndexNEW.RepositoryPointer	= DocumentIndexOLD.RepositoryPointer;
		DocumentIndexNEW.htmlSize		= DocumentIndexOLD.htmlSize;
		DocumentIndexNEW.imageSize		= DocumentIndexOLD.imageSize;
		DocumentIndexNEW.ResourcePointer	= DocumentIndexOLD.ResourcePointer;
		DocumentIndexNEW.ResourceSize		= DocumentIndexOLD.ResourceSize;
		DocumentIndexNEW.IPAddress		= DocumentIndexOLD.IPAddress;
		DocumentIndexNEW.response		= DocumentIndexOLD.response;
		DocumentIndexNEW.userID			= DocumentIndexOLD.userID;
		DocumentIndexNEW.clientVersion		= DocumentIndexOLD.clientVersion;


		DocumentIndexNEW.nrOfOutLinks		= 0;
		DocumentIndexNEW.SummaryPointer		= 0;
		DocumentIndexNEW.SummarySize		= 0;
		DocumentIndexNEW.CRC32			= 0;


		if (fwrite(&DocumentIndexNEW,sizeof(DocumentIndexNEW),1,DIFHNEW) != 1) {
			perror("fwrite");
			exit(1);
		}

	}

	remove(DITEMPPatch);
}
