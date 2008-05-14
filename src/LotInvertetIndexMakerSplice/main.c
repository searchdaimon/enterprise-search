/**********************************************************************************
Dette programet spleiser sammen en iindex hvis den har fragmenterte poster, som en anker index
**********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/mgsort.h"

#include "../common/define.h"
#include "../common/lot.h"

#define maxHitsToSplice 1000000
#define maxtermrecord 10000000

struct revIndexArrayFomat {
	unsigned int DocID;
        unsigned long WordID;
	unsigned char langnr;
        unsigned long nrOfHits;
        unsigned short hits[MaxsHitsInIndex];
};

int IndekserSplice(char iindexPath[]);

int main (int argc, char *argv[]) {


	int lotNr;
	int lotPart;
	char path[256];
	char iipath[256];
	unsigned lastIndexTime;
	char subname[maxSubnameLength];


        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"v"))!=-1) {
                switch (c) {
                        case 'v':
                                break;
                        default:
                                          exit(1);
                }
        }
        --optind;

	printf("lot %s, %i\n",argv[1],argc);


	if ((argc -optind)== 4) {
		lotNr = atoi(argv[2 +optind]);
		strncpy(subname,argv[3 +optind],sizeof(subname) -1);

                //finner siste indekseringstid
                lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);


                if(lastIndexTime == 0) {
                        printf("lastIndexTime is 0\n");
                        exit(1);
                }

               //sjekker om vi har nokk palss
                if (!lotHasSufficientSpace(lotNr,4096,subname)) {
                        printf("insufficient disk space\n");
                        exit(1);
                }


        	printf("Indexing all buvkets for lot %i\n",lotNr);

		for (lotPart=0;lotPart<64;lotPart++) {
			//printf("indexint part %i for lot %i\n",lotPart,lotNr);

			//"$revindexPath/$revindexFilNr.txt";
			GetFilPathForLot(path,lotNr,subname);
			//ToDo: må sette språk annen plass
			sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);


                        sprintf(iipath,"%s%i.txt",iipath,lotPart);

			IndekserSplice(iipath);	



		}
	}
	else if ((argc - optind) == 5) {
		lotNr = atoi(argv[2 +optind]);
		strncpy(subname,argv[3 +optind],sizeof(subname) -1);
		lotPart = atoi(argv[4 +optind]);
		printf("indexint part %i for lot %i\n",lotPart,lotNr);

		//"$revindexPath/$revindexFilNr.txt";
		GetFilPathForLot(path,lotNr,subname);
		//ToDo: må sette språk annen plass
		//aa sprintf(iipath,"%siindex/%s/index/aa/%i.txt",path,argv[1 +optind],lotPart);
                        //ToDo: må sette språk annen plass
                        sprintf(iipath,"%siindex/%s/index/aa/",path,argv[1 +optind]);


                        sprintf(iipath,"%s%i.txt",iipath,lotPart);



			IndekserSplice(iipath);	

	
	}
	else {
		printf("usage: ./LotInvertetIndexMaker type lotnr subname [ lotPart ]\n\n");

	}

	//GetFilPathForLot(lotNr);

}

//copy a memory area, and return the size copyed
static inline size_t memcpyrc(void *s1, const void *s2, size_t n) {
//size_t memcpyrc(void *s1, const void *s2, size_t n) {
        memcpy(s1,s2,n);

        return n;
}

int compare_elements (const void *p1, const void *p2) {


        if ((*(unsigned short *)p1) < (*(unsigned short *)p2))
                return -1;
        else
                return (*(unsigned short *)p1) > (*(unsigned short *)p2);

}

void addindex( char **termrecord,int *termrecordlen,int termrecordsize,int *totalTermAntall, 
	unsigned int DocID,unsigned char langnr,unsigned int TermAntall,unsigned short *allhit) {

	int i;
	int len;
	#ifdef DEBUG
	printf("writeindex: DocID: %u, langnr: %i, nr: %u. Hits: ",DocID,(int)langnr,TermAntall);
	#endif
	len = sizeof(DocID) + sizeof(langnr)+ sizeof(TermAntall) + (sizeof(unsigned short) * TermAntall);

	if ((len + (*termrecordlen)) > termrecordsize) {
		printf("writeindex: can't add record if %i bytes. Have used %i of %i left\n",len,(*termrecordlen),maxtermrecord);
		return;
	}

	//sorterer hittene
	if (TermAntall > 1) {
		qsort(allhit, TermAntall , sizeof(unsigned short), compare_elements);
	}
	(*termrecord) += memcpyrc((*termrecord),&DocID,sizeof(DocID));
	(*termrecord) += memcpyrc((*termrecord),&langnr,sizeof(langnr));
	(*termrecord) += memcpyrc((*termrecord),&TermAntall,sizeof(TermAntall));

	(*termrecord) += memcpyrc((*termrecord),allhit,sizeof(unsigned short) * TermAntall);

	#ifdef DEBUG
	for (i = 0;i < TermAntall; i++) {
		printf("%i ",allhit[i]);
	}
	#endif

	(*totalTermAntall) += 1;
	(*termrecordlen) += len;
	#ifdef DEBUG
	printf("\n");
	#endif
}

void writeterm(FILE *FH, char *termrecord,int termrecordlen,unsigned int totalTermAntall, unsigned int term) {

	#ifdef DEBUG
	printf("writeterm: term %u, totalTermAntall %u \n",term,totalTermAntall);
	#endif

	fwrite(&term,sizeof(term),1,FH);
	fwrite(&totalTermAntall,sizeof(totalTermAntall),1,FH);

	fwrite(termrecord,sizeof(char),termrecordlen,FH);
}

int IndekserSplice(char iindexPath[]) {

	int i,y;

	FILE *fileha;

        unsigned long DocID;
        unsigned long TermAntall;
        unsigned short hit, lasthit;

	unsigned long term;
	unsigned long Antall;
	unsigned char langnr;

	unsigned short *allhit = malloc(sizeof(unsigned short) * maxHitsToSplice);
	char *termrecord = malloc(maxtermrecord);
	char *termrecordptr;

	int termrecordlen;
	unsigned int allAntall;
	unsigned int lastDocID;
	unsigned char lastlangnr;
	int newTermAntall;

	#ifdef DEBUG
	printf("iindexPath %s\n",iindexPath);
	#endif

	if ((fileha = fopen(iindexPath,"rb")) == NULL) {
		perror(iindexPath);
		//exit(1);
		return;
	}

	char outfile[512];
	FILE *outfh;

	snprintf(outfile,sizeof(outfile),"%s_splicetmp",iindexPath); 
	
        if ((outfh = fopen(outfile,"w")) == NULL) {
                perror(outfile);
                exit(EXIT_FAILURE);
        }


        #ifdef DEBUG
		printf("iinde file \"%s\"\n",iindexPath);
        	printf("tmp outfile: \"%s\"\n",outfile);
        #endif

	int n;
	while (!feof(fileha)) {
		//wordid hedder
        	if ((n=fread(&term,sizeof(unsigned long),1,fileha)) != 1) {
			#ifdef DEBUG
				printf("last hit\n");
			#endif
			break;
		}
		//printf("n %i\n",n);
        	if(fread(&Antall,sizeof(unsigned long),1,fileha)!= 1) {
                        printf("can't read Antall\n");
                        break;
                }

		#ifdef DEBUG
		printf("term: %u antall: %u\n",term,Antall);
		#endif

		lastDocID = 0;
		allAntall = 0;
		termrecordlen = 0;
		newTermAntall = 0;
		termrecordptr = termrecord;

		lasthit = 0;
		for (i=0;i<Antall;i++) {
			//side hedder
			if(fread(&DocID,sizeof(unsigned long),1,fileha)!= 1) {
                       	        printf("can't read DocID\n");
                        	break;
                	}
			if(fread(&langnr,sizeof(char),1,fileha)!= 1) {
                                printf("can't read langnr\n");
                                break;
                        }
        		if(fread(&TermAntall,sizeof(unsigned long),1,fileha)!= 1) {
                                printf("can't read term antall\n");
                                break;
                        }

			if ((DocID != lastDocID) && (lastDocID != 0)) {
				addindex(&termrecordptr,&termrecordlen,maxtermrecord,&newTermAntall,lastDocID,lastlangnr,allAntall,allhit);
				allAntall = 0;
				lasthit = 0;
			}

			#ifdef DEBUG
			printf("DocID: %u, langnr: %i, nr: %u. Hits: ",DocID,(int)langnr,TermAntall);
			#endif

			for (y = 0;y < TermAntall; y++) {
					if (allAntall < maxHitsToSplice) {
                		        	//fread(&allhit[allAntall],sizeof(unsigned short),1,fileha);
                		        	if(fread(&hit,sizeof(unsigned short),1,fileha)!= 1) {
                                			printf("can't read normal hit\n");
                                			break;
                        			}
						#ifdef DEBUG
						printf("%i,",allhit[allAntall]);
						#endif
						//ToDo: tar ikke med dublikater. Vil det skape problemer for anker indexkser??
						if (hit != lasthit) {
							allhit[allAntall] = hit;
							
							++allAntall;
						}
						else {
							#ifdef DEBUG
							printf("hit is a dublicate. %i == %i?\n",(int)hit,(int)lasthit);
							#endif
						}

						lasthit = hit;
					}
					else {
                		        	if(fread(&hit,sizeof(unsigned short),1,fileha) != 1) {
							fprintf(stderr,"can't read hit (hit was past max, probobly a bug). hit was nr %i of %i\n",y,TermAntall);
							perror("fread");
							goto IndekserSpliceError;
						}
						#ifdef DEBUG
						printf("%i,",hit);
						#endif
					}

			}

			#ifdef DEBUG
			printf("\n");
			#endif
			lastlangnr = langnr;
			lastDocID = DocID;

		}	
		//siste for denne termen
		addindex(&termrecordptr,&termrecordlen,maxtermrecord,&newTermAntall,lastDocID,lastlangnr,allAntall,allhit);

		writeterm(outfh,termrecord,termrecordlen,newTermAntall, term);

	}

	fclose(fileha);

       	//må lokke før vi kopierer. Hvis ikke kan det være data i bufferen som ikke blir med.
        fclose(outfh);
    	//close(outfd);


        //sletter den gamle filen
        if (unlink(iindexPath) != 0) {
                perror(iindexPath);
        }

	#ifdef DEBUG
	printf("renaming %s -> %s\n",outfile,iindexPath);
	#endif
        //flytter den nye filen slik at den blir output filen
        if (rename(outfile,iindexPath) == -1) {
                perror("rename");
                exit(EXIT_FAILURE);
        }


        //free(outfile);


	free(allhit);
	free(termrecord);

	return 1;

	//hånterer error. Vi sletter index filen, da det er noe feil med den, og vi ikke
	//ønsker å dra med den feilen videre.
	IndekserSpliceError:

	fprintf(stderr,"Error: will delte index \"%s\"\n",iindexPath);		

	//lokker filer
	fclose(outfh);	
	fclose(fileha);

        //sletter den gamle filen
        if (unlink(iindexPath) != 0) {
                perror(iindexPath);
        }

	return 0;
}

