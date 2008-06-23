#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/DocumentIndex.h"
#include "../common/boithohome.h"
#include "../common/logs.h"
#include "../common/time.h"




//struct DIArrayFormat {
//        struct DocumentIndexFormat docindex;
//        unsigned int DocID;
//	char gced;
//};

struct gcaoptFormat {
	unsigned int MaxAgeDiflastSeen;
	int dryRun;
	FILE *log;
	int keept;
	int gced;

};

int
gcdecide(int LotNr, char *subname, struct gcaoptFormat *gcaopt)
{
	int i;

	char path[1024];
	char path3[1024];
	FILE *GCEDFH;
	unsigned int DocID;

	#ifdef BLACK_BOKS
		time_t newest_document;
	#endif
	FILE *DOCINDEXFH;


	struct DocumentIndexFormat *DIArray;


	//åpner dokument indeks får å teste at vi har en, hvis ikke kan vi bare avslutte.
        if ( (DOCINDEXFH = lotOpenFileNoCasheByLotNr(LotNr,"DocumentIndex","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a DocumentIndex file\n");
		#endif

                return 0;
        }
	fclose(DOCINDEXFH);

	blog(gcaopt->log,1,"Runing gc for collection \"%s\", lot nr %i",subname,LotNr);

	if ((DIArray = malloc(sizeof(struct DocumentIndexFormat) * NrofDocIDsInLot)) == NULL) {
		perror("gcrepo: malloc DIArray");
		exit(1);
	}

	#ifdef BLACK_BOKS
		newest_document = 0;	
		printf("newest_document: %s\n",ctime_s(&newest_document));
	#endif

	//leser inn hele
	for (i=0;i<NrofDocIDsInLot;i++) {
		//DIArray[i].DocID = 0;
		//DIArray[i].gced = 0;

		if (!DIRead(&DIArray[i], LotDocIDOfset(LotNr) +i, subname)) {
			#ifdef DEBUG
				fprintf(stderr, "Unable to locate a DI for %u\n", LotDocIDOfset(LotNr) +i);
			#endif
			continue;
		}

		#ifdef BLACK_BOKS
		if ((DIArray[i].lastSeen != 0) && (newest_document < DIArray[i].lastSeen)) {
                                newest_document = DIArray[i].lastSeen;
                }
		#endif

		//DIArray[i].DocID = LotDocIDOfset(LotNr) +i;

	}

	#ifdef BLACK_BOKS
		blog(gcaopt->log,1,"Newest document: %s",ctime_s(&newest_document));
	#endif



	//går gjenom alle på jakt etter de som kan slettes
	for (i=0;i<NrofDocIDsInLot;i++) {

		//if (DIArray[i].DocID == 0) {
		//	continue;
		//}
	
		if (DIS_isDeleted(&DIArray[i])) {
			continue;
		}

		#ifdef DEBUG
			#ifdef BLACK_BOKS
				printf("dokument \"%s\", lastSeen: %s",
					DIArray[i].Url,
					ctime_s(&DIArray[i].lastSeen));
			#endif
		#endif

		#ifdef BLACK_BOKS
		if ((DIArray[i].lastSeen != 0) && (newest_document > (DIArray[i].lastSeen + gcaopt->MaxAgeDiflastSeen))) {
			//DIArray[i].gced = 1;


			//sletter
			DIS_delete(&DIArray[i]);

			blog(gcaopt->log,1,"dokument \"%s\" can be deleted. Last seen: %s, DocID %u",DIArray[i].Url,ctime_s(&DIArray[i].lastSeen),LotDocIDOfset(LotNr) +i);
			++gcaopt->gced;
		
		} 
		else {
			++gcaopt->keept;
		}
		#endif
	}

	//lokker filen repo.wip
	lotCloseFiles();


	printf("writing to DI..\n");
	for (i=0;i<NrofDocIDsInLot;i++) {

		//if (DIArray[i].DocID != 0) {
			DIWrite(&DIArray[i], LotDocIDOfset(LotNr) +i, subname, "DocumentIndex.wip");
		//}
	}
	printf("..done\n");

	//lagrer hvilkene filer vi har slettet
	GCEDFH =  lotOpenFileNoCasheByLotNr(LotNr,"gced","a", 'e',subname);

	for (i=0;i<NrofDocIDsInLot;i++) {
		//ToDo:
		//dette betyr vel at vi bare tar bort dokumenter vi selv sletter fra iindex. Hva hvis IndexerLot har beordret sleting?.
		//skulle vi heller ha brukt DIS_isDeleted her?
		//if ((DIArray[i].DocID != 0) && (DIArray[i].gced)) {
		if (DIS_isDeleted(&DIArray[i])) {
			DocID = LotDocIDOfset(LotNr) +i;
			if (fwrite(&DocID,sizeof(DocID),1,GCEDFH) != 1) {
				perror("can't write gc file");
			}
		}
	}
	#ifdef DI_FILE_CASHE
		closeDICache();
	#endif

	fclose(GCEDFH);
	free(DIArray);



	/* And we have a race... */
	GetFilPathForLot(path, LotNr, subname);
	GetFilPathForLot(path3, LotNr, subname);

	strcat(path, "DocumentIndex.wip");
	strcat(path3, "DocumentIndex");

	if (gcaopt->dryRun != 1) {
		printf("renaming %s -> %s\n",path,path3);
		if (rename(path, path3) != 0) {
			perror("can't rename di");
		}
	}



	return 0;
}


int
main(int argc, char **argv)
{

	int LotNr;
	char *subname;
	struct gcaoptFormat gcaopt;

	gcaopt.MaxAgeDiflastSeen  = 86400;
	gcaopt.dryRun  = 0;
	gcaopt.log = NULL;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"t:dl"))!=-1) {
                switch (c) {
                        case 't':
                                gcaopt.MaxAgeDiflastSeen  = atou(optarg);
                                break;
			case 'd':
				gcaopt.dryRun = 1;
				break;
			case 'l':
				gcaopt.log = fopen(bfile("logs/gc"),"ab");
				break;
                        default:
                                exit(1);
                }

        }
        --optind;


	gcaopt.keept = 0;
	gcaopt.gced = 0;

	
	if (argc < 2)
		errx(1, "Usage: ./gcrepo subname [ lotnr ]");

	subname = argv[1 +optind];

	#ifndef BLACK_BOKS
		fprintf("dette fungerer bare med black boks for nå\n");
		exit(1);
	#endif

	if ((argc -optind) == 3) {
		LotNr = atoi(argv[2 +optind]);
		gcdecide(LotNr,subname, &gcaopt);
	}
	else {
		for(LotNr=1;LotNr<maxLots;LotNr++) {
			gcdecide(LotNr,subname, &gcaopt);
		}
	}


	blog(gcaopt.log,1,"keept %i, gced %i",gcaopt.keept,gcaopt.gced);

	return 0;
}

