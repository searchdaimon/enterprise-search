#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include "../common/crc32.h"

#include "../IndexerRes/IndexerRes.h"
#include "../common/integerindex.h"
#include "../searchFilters/searchFilters.h"

#include "../common/bstr.h"


#include "../parse_summary/summary.h"

#ifdef BLACK_BOKS
//acllot includes
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

struct alclotFormat {
	char subname[maxSubnameLength];
	char openmode[4];
	int lotNr;
	int *h;
};


struct aclusernameFormat {
	char username[MAX_USER_NAME_LEN];
	int len;
};

static unsigned int alclot_hashfromkey(void *ky)
{
    struct aclusernameFormat *k = (struct aclusernameFormat *)ky;
    return crc32boithonl((*k).username,(*k).len);
}

static int alclot_equalkeys(void *k1, void *k2)
{
	//hvis di ikke her samme lengde er de ikke like
	if ((*(struct aclusernameFormat *)k1).len != (*(struct aclusernameFormat *)k2).len) {
		return 0;
	}
	else {
	//hvis vi derimot her samme lenge so må vi sjkke
    		return (0 == memcmp((*(struct aclusernameFormat *)k1).username,(*(struct aclusernameFormat *)k2).username,(*(struct aclusernameFormat *)k1).len));
	}
}


void alclot_init(struct alclotFormat **alclot,char subname[],char openmode[],int lotNr) {

	(*alclot) = malloc(sizeof(struct alclotFormat));
	strcpy((**alclot).subname,subname);
	strcpy((**alclot).openmode,openmode);
	(**alclot).lotNr = lotNr;

	//struct hashtable *h;
	(**alclot).h = (int *)create_hashtable(200, alclot_hashfromkey, alclot_equalkeys);



}
void alclot_close(struct alclotFormat *alclot) {

	FILE *fp;

	fp = lotOpenFileNoCasheByLotNr((*alclot).lotNr,"acllist",(*alclot).openmode, 'e',(*alclot).subname);

	struct hashtable **h; //temp

	struct aclusernameFormat *aclusername;
	int *value;
	printf("acls for lot:\n");
	if (hashtable_count((struct hashtable *)(*alclot).h) > 0)
        {
        	struct hashtable_itr *itr;

                itr = hashtable_iterator((struct hashtable *)(*alclot).h);
                do {
                	aclusername 	= (struct aclusernameFormat *)hashtable_iterator_key(itr);
                        value 		= (int *)hashtable_iterator_value(itr);

                        printf("acl user \"%s\", len %i, nr %i\n",(*aclusername).username,(*aclusername).len,(*value));
			fwrite((*aclusername).username,(*aclusername).len,1,fp);
			fwrite("\n",1,1,fp);

               	} while (hashtable_iterator_advance(itr));
                free(itr);

    	}
	printf("acl list end\n");
	fclose(fp);
        hashtable_destroy((struct hashtable *)(*alclot).h,1);
}
void alclot_add(struct alclotFormat *alclot,char acl[]) {


	struct hashtable **h; //temp

  	char **Data;
  	int Count, TokCount;
	int *oldvalue;
	int *value;

  	TokCount = split(acl, ",", &Data);

  	Count = 0;
  	while( (Data[Count] != NULL) ) {

		#ifdef DEBUG
		printf("god user \"%s\"\n",Data[Count]);
		#endif

		struct aclusernameFormat *aclusername = malloc(sizeof(struct aclusernameFormat));

		(*aclusername).len = strnlen(Data[Count],MAX_USER_NAME_LEN);
		strscpy((*aclusername).username,Data[Count],MAX_USER_NAME_LEN);

		if (NULL == (oldvalue = hashtable_search((struct hashtable *)(*alclot).h,aclusername) )) {
			//printf("not found!. Vil insert first");
			value = malloc(sizeof(int));
			(*value) = 1;

			if (! hashtable_insert((struct hashtable *)(*alclot).h,aclusername,value) ) {
	                        printf("cant insert!\n");
	                	exit(-1);
	                }
		}
		else {
			//printf("username exist with %i. ++ing\n",(*oldvalue));
			++(*oldvalue);	
		}

		++Count;
	}

  	FreeSplitList(Data);

}

#endif

int makePreParsedSummary(char HtmlBuffer[], int HtmlBufferSize, unsigned int DocID, FILE *SFH,
	unsigned int *SummaryPointer,unsigned short *SummarySize) {

/*
#ifndef BLACK_BOKS

	int n;

	char *SummeryBuf;
	int SummeryBufLen;

	Bytef *WorkBuffer;
	uLong WorkBufferSize;

	SummeryBufLen = (HtmlBufferSize * 2);

	SummeryBuf = malloc(SummeryBufLen);

                                char        *titleaa =NULL , *body = NULL, *metakeyw = NULL, *metadesc = NULL;


                                generate_summary( HtmlBuffer, HtmlBufferSize, &titleaa, &body, &metakeyw,&metadesc );

                                SummeryBufLen = snprintf(SummeryBuf,SummeryBufLen,"%s\n%s\n%s",titleaa,metadesc,body);


                                //printf("tit %s\n",titleaa);


                                if (titleaa != NULL) free(titleaa);
                                if (body != NULL) free(body);
                                if (metakeyw != NULL) free(metakeyw);
                                if (metadesc != NULL) free(metadesc);

        			WorkBufferSize = (ceil(SummeryBufLen * 1.001) +12);
				WorkBuffer = malloc(WorkBufferSize);

                                if ( (n = compress(WorkBuffer,&WorkBufferSize,(Bytef *)SummeryBuf,SummeryBufLen)) != 0) {
                                        printf("compress error. Code: %i. WorkBufferSize %i, SummeryBufLen %i\n",n,WorkBufferSize,SummeryBufLen);
                                }
                                else {


                                        (*SummaryPointer) = ftell(SFH);
                                        (*SummarySize) = (sizeof(DocID) + WorkBufferSize);

					//write it to disk
                                        fwrite(&DocID,sizeof(DocID),1,SFH);
                                        fwrite(WorkBuffer,WorkBufferSize,sizeof(char),SFH);



                                }
				free(WorkBuffer);


	free(SummeryBuf);

#endif
*/
}

int main (int argc, char *argv[]) {

        int lotNr;
	char lotServer[64];
	int pageCount;
	int i;

	FILE *ADULTWEIGHTFH, *SFH;
	unsigned char awvalue;

        unsigned int FiltetTime;
        unsigned int FileOffset;
	off_t DocIDPlace;
	unsigned char langnr;

        char htmlcompressdbuffer[524288];  //0.5 mb
        char imagebuffer[524288];  //0.5 mb
	
	int httpResponsCodes[nrOfHttpResponsCodes];

	struct ReposetoryHeaderFormat ReposetoryHeader;
	struct DocumentIndexFormat DocumentIndexPost;
	unsigned long int radress;
	FILE *revindexFilesHa[NrOfDataDirectorys];
	struct adultFormat adult;
	unsigned int lastIndexTime;
	unsigned int optMustBeNewerThen = 0;
	unsigned int optrEindex = 0;


	globalIndexerLotConfig.collectUrls = 0;
	globalIndexerLotConfig.urlfilter = NULL;

	extern char *optarg;
       	extern int optind, opterr, optopt;
	char c;
	while ((c=getopt(argc,argv,"neu:"))!=-1) {
                switch (c) {
                        case 'n':
                                optMustBeNewerThen = 1;
                                break;
                        case 'e':
                                optrEindex = 1;
                                break;

                        case 'u':
				printf("optopt \"%s\"\n",optarg);
				globalIndexerLotConfig.collectUrls = 1;
				if (strcmp(optarg,"-") == 0) {
					printf("Will collect all urls\n");

				}
				else {
					split(optarg,",",&globalIndexerLotConfig.urlfilter);
					printf("will only collect url of ending:\n");
					i=0;
					while( (globalIndexerLotConfig.urlfilter[i] != NULL) ) {
    						printf("\t\t%i\t\"%s\"\n", i, globalIndexerLotConfig.urlfilter[i++]);
					}

				}

				//exit(1);
                                break;
			default:
                                          exit(1);
                }
	}
	--optind;

	printf("argc %i, optind %i\n",argc,optind);


        if ((argc - optind)!= 3) {
                printf("Dette programet indekserer en lot. Usage:\n\tIndexerLot lotNr subname\n");
                exit(0);
        }

	for(i=0;i<nrOfHttpResponsCodes;i++) {
		httpResponsCodes[i] = 0;
	}

	lotNr = atoi(argv[1 +optind]);
	//strncpy(subname,argv[2 + optind],sizeof(subname) -1);
	subname = argv[2 + optind];

	char *acl = NULL;

	printf("subname %s, lotNr %i\n",subname,lotNr);

        langdetectInit();

	addNewUrlOpen(&global_addNewUrlha);
	addNewUrlOpen(&global_addNewUrlha_pri1);
	addNewUrlOpen(&global_addNewUrlha_pri2);

	//find server based on lotnr
	lotlistLoad();
	lotlistGetServer(lotServer,lotNr);

	unsigned int HtmlBufferLength;
	char HtmlBuffer[524288];
	int nerror;
	char openmode[4];
	char domain[65];

	printf("vil index lot nr %i at %s\n",lotNr,lotServer);

	adultLoad(&adult);


	//temp: må hente dette fra slot server eller fil
	FileOffset = 0;

	pageCount = 0;
/*
	if (0) {


		printf("will ges pages by net\n");

		revindexFilesOpenNET(revindexFilesHa);

		while (rGetNextNET(lotServer,lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,&radress,FiltetTime,FileOffset,subname)) {


				global_source_url_havpri = url_havpri1(ReposetoryHeader.url);


                                global_curentDocID = ReposetoryHeader.DocID;
                                if (strchr(ReposetoryHeader.url,'?') == 0) {
                                        global_curentUrlIsDynamic = 0;
                                }
                                else {
                                        global_curentUrlIsDynamic = 1;
                                }
		
			
			
			handelPage(lotServer,lotNr,&ReposetoryHeader,htmlcompressdbuffer,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult,&langnr);
			//datta skal uansett kopieres over
			//kopierer over di data
			copyRepToDi(&DocumentIndexPost,&ReposetoryHeader);

			DocumentIndexPost.RepositoryPointer = radress;
			DocumentIndexPost.Sprok = langnr;

			//skiver til DocumentIndex
			DIWriteNET(lotServer,&DocumentIndexPost,ReposetoryHeader.DocID,subname);


			++pageCount;
		
			//temp: 
			//if(pageCount > 999) {
			//	printf("Exeting after only %i docs\n",pageCount);
			//	break;
			//}

		}

		printf("Sending pages\n");

		revindexFilesSendNET(revindexFilesHa,lotNr);

	}
	else {
*/
		printf("Wil acess files localy\n");


		//sjekker om vi har nokk palss
		if (!lotHasSufficientSpace(lotNr,10240,subname)) {
			printf("insufficient disk space\n");
			exit(1);
		}


		//finner siste indekseringstid
		lastIndexTime =  GetLastIndexTimeForLot(lotNr,subname);

		printf("lastIndexTime %u\n",lastIndexTime);

		if (optrEindex == 1) {
			FiltetTime = 0;
			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (lastIndexTime == 0) {
			FiltetTime = 0;

			//opner får skriving (vil overskrive eventuelle gamle filer). 
			strcpy(openmode,"wb");
		}
		else if (optMustBeNewerThen != 0) {
			//regner ut hvor ny den må være
			printf("Isnet new. Last indexed %s",ctime((time_t *)&lastIndexTime));
			exit(1);
		}		
		else if(lastIndexTime != 0) {
			printf("lastIndexTime is not 0, but %i\n",lastIndexTime);
			FiltetTime = lastIndexTime;

			//opner for appending
			strcpy(openmode,"ab");
		}
		else {
			printf("sholden happend!\n");
			exit(1);
		}

		revindexFilesOpenLocal(revindexFilesHa,lotNr,"Main",openmode,subname);

		#ifdef BLACK_BOKS		
			struct alclotFormat *alclot;
			alclot_init(&alclot,subname,openmode,lotNr);
		#endif


		ADULTWEIGHTFH = lotOpenFileNoCasheByLotNr(lotNr,"AdultWeight",openmode, 'e',subname);
		SFH = lotOpenFileNoCasheByLotNr(lotNr,"summary",openmode,'r',subname);
		
		//temp:Søker til problemområdet
		//FileOffset = 334603785;		

		while (rGetNext(lotNr,&ReposetoryHeader,htmlcompressdbuffer,sizeof(htmlcompressdbuffer),imagebuffer,&radress,FiltetTime,FileOffset,subname,&acl)) {

				memset(&DocumentIndexPost,0, sizeof(DocumentIndexPost));

				//printf("D: %u, R: %lu\n",ReposetoryHeader.DocID, radress);

				#ifndef BLACK_BOKS

				if (!find_domain_no_subname(ReposetoryHeader.url,domain,sizeof(domain)) ) {
					printf("can't find domain. Url \"%s\"\n",ReposetoryHeader.url);
					continue;
				}

				if (filterDomainNrOfLines(domain)) {
					debug("To many lines in domaine. Domain \"%s\"\n",domain);
					continue;
				}

				if (filterDomainLength(domain)) {
					debug("To long domaine. Domain \"%s\"\n",domain);
					continue;
				}

				if (filterTLDs(domain)) {
					debug("bannet TLD. Domain \"%s\"\n",domain);
					continue;
				}
				#endif

				global_source_url_havpri = url_havpri1(ReposetoryHeader.url);


				//if (global_source_url_havpri) {
				//	printf("source_url_havpri %s\n",ReposetoryHeader.url);
				//}

                                global_curentDocID = ReposetoryHeader.DocID;
                                if (strchr(ReposetoryHeader.url,'?') == 0) {
                                        global_curentUrlIsDynamic = 0;
                                }
                                else {
                                        global_curentUrlIsDynamic = 1;
                                }
				#ifdef DEBUG
					printf("url: %s, DocID %u\n",ReposetoryHeader.url,ReposetoryHeader.DocID);
				#endif
	
				#ifdef URLSONLY
					if (ReposetoryHeader.response == 200) {	

						if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)HtmlBufferLength,htmlcompressdbuffer,ReposetoryHeader.htmlSize)) != 0) {
							#ifdef DEBUG
                        				printf("uncompress error. Code: %i for DocID %u-%i\n",error,DocID,rLotForDOCid(DocID));
							#endif
                        				continue;
				                }

						//får å bare ekstrahere urler
						//printf("URLSONLY\n");			
						handelPage_urlsonly(lotServer,lotNr,&ReposetoryHeader,HtmlBuffer,HtmlBufferLength,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult);
	
						++pageCount;
					}
				#else
				#ifdef URLPRISONLY
					//pare prioriterte linker
					//printf("URLPRISONLY\n");
					if (ReposetoryHeader.response == 200) {	
						if (global_source_url_havpri) {
							if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)HtmlBufferLength,htmlcompressdbuffer,ReposetoryHeader.htmlSize)) != 0) {
								#ifdef DEBUG
                        					printf("uncompress error. Code: %i for DocID %u-%i\n",error,DocID,rLotForDOCid(DocID));
								#endif
        	                				continue;
					                }
	
							handelPage_urlsonly(lotServer,lotNr,&ReposetoryHeader,HtmlBuffer,HtmlBufferLength,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult);
	
							++pageCount;
						}
					}
				#else			
					if ((ReposetoryHeader.response >= 200) && (ReposetoryHeader.response <= 299)) {	

						HtmlBufferLength = sizeof(HtmlBuffer);
						if ( (nerror = uncompress((Bytef*)HtmlBuffer,(uLong *)&HtmlBufferLength,(Bytef*)htmlcompressdbuffer,ReposetoryHeader.htmlSize)) != 0) {
							#ifdef DEBUG
                        				printf("uncompress error. Code: %i for DocID %u-%i\n",nerror,ReposetoryHeader.DocID,rLotForDOCid(ReposetoryHeader.DocID));
							#endif
                        				continue;
				                }
						//usikker her. Skal det vare +1? strlen() blir da en større en HtmlBufferLength
						//HtmlBuffer[HtmlBufferLength +1] = '\0';

						//printf("document \"%s\" %i b\n",HtmlBuffer,HtmlBufferLength);

						handelPage(lotServer,lotNr,&ReposetoryHeader,HtmlBuffer,HtmlBufferLength,imagebuffer,revindexFilesHa,&DocumentIndexPost,ReposetoryHeader.DocID,httpResponsCodes,&adult,&langnr);

						//lager summery
						/*
						temp
						makePreParsedSummary(HtmlBuffer,HtmlBufferLength,ReposetoryHeader.DocID,
							SFH,&DocumentIndexPost.SummaryPointer,&DocumentIndexPost.SummarySize);
						*/
						//printf("lang %s\n",lang);

						DocumentIndexPost.crc32 = crc32boithonl(HtmlBuffer,HtmlBufferLength);
						sprintf(DocumentIndexPost.Sprok,"%i",langnr);

						//setter anatll utgående linker
						//bruker en unsigned char. Kan ikke ha flere en 255 
						if (pagewords.nrOfOutLinks > 255) {
							DocumentIndexPost.nrOfOutLinks = 255;
						}
						else {
							DocumentIndexPost.nrOfOutLinks = pagewords.nrOfOutLinks;
						}

						//printf("nrOfOutLinks %i\n",(int)DocumentIndexPost.nrOfOutLinks);

						//printf("AdultWeight %i\n",(int)DocumentIndexPost.AdultWeight);

						//printf("%i : strlen %i, len %i, crc32 %i. end: \"%c%c%c\"\n",ReposetoryHeader.DocID,strlen(HtmlBuffer),HtmlBufferLength,DocumentIndexPost.crc32,HtmlBuffer[HtmlBufferLength -2],HtmlBuffer[HtmlBufferLength -1],HtmlBuffer[HtmlBufferLength]);

						//setter adult vekt
                				if (DocumentIndexPost.AdultWeight >= AdultWeightForXXX) {
                        				//printf("DocID: %u, %hu, url: %s\n",DocID,DocumentIndexPost.AdultWeight,DocumentIndexPost.Url);
                        				//mark as adult
                        				awvalue = 1;
                				}
                				else {
                        				//not adult
                        				awvalue = 0;
                				}
						DocIDPlace = ((ReposetoryHeader.DocID - LotDocIDOfset(lotNr)) * sizeof(unsigned char));
						//printf("DocID %u, DocIDPlace %i\n",ReposetoryHeader.DocID,DocIDPlace);
						fseek(ADULTWEIGHTFH,DocIDPlace,SEEK_SET);

                				fwrite(&awvalue,sizeof(awvalue),1,ADULTWEIGHTFH);

					}
					else {
						//ikke 200->299 side
					}

					//data skal kopieres over uanset hva som skjer
					//kopierer over di data
					copyRepToDi(&DocumentIndexPost,&ReposetoryHeader);

					DocumentIndexPost.RepositoryPointer = radress;


					//skiver til DocumentIndex
					DIWrite(&DocumentIndexPost,ReposetoryHeader.DocID,subname);

					#ifdef BLACK_BOKS
						//handel acl
						alclot_add(alclot,acl);

						debug("time %u\n",ReposetoryHeader.time);

						iintegerSetValue(&ReposetoryHeader.time,sizeof(int),ReposetoryHeader.DocID,"dates",subname);
						//printf("filtypes \"%c%c%c%c\"\n",ReposetoryHeader.doctype[0],ReposetoryHeader.doctype[1],ReposetoryHeader.doctype[2],ReposetoryHeader.doctype[3]);
						//normaliserer
						for(i=0;i<4;i++) {
							ReposetoryHeader.doctype[i] = btolower(ReposetoryHeader.doctype[i]);
						}
						iintegerSetValue(&ReposetoryHeader.doctype,4,ReposetoryHeader.DocID,"filtypes",subname);
					#endif

					++pageCount;

				#endif
				#endif
			
			if ((pageCount % 10000) == 0) {
                        	printf("%i\n",pageCount);
                        }

			/*			
			if(pageCount > 9) {
				printf("Exeting after only %i docs\n",pageCount);
				break;
				//exit(1);
			}
			*/

		}

		#ifdef BLACK_BOKS
			alclot_close(alclot);
		#endif

		//skriver riktig indexstide til lotten
		setLastIndexTimeForLot(lotNr,httpResponsCodes,subname);
		
		fclose(ADULTWEIGHTFH);
		fclose(SFH);

		// vi må ikke kopiere revindex filene da vi jobber på de lokale direkte
//	}


	printf("indexed %i pages\n\n\n",pageCount);

	 langdetectDestroy();

	if (globalIndexerLotConfig.urlfilter != NULL) {
		FreeSplitList(globalIndexerLotConfig.urlfilter);
	}

	return 0;
}

