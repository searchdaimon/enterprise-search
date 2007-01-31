#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

#include "../httpGet/httpGet.h"
#include "../searchFilters/searchFilters.h"
#include "ppcXml.h"
#include "../parse_summary/summary.h"
#include "../common/bstr.h"
#include "../common/define.h"

#include <ctype.h>



void getPpcAds(char provider[],struct ppcPagesFormat *ppcPages_gloabal, int *nrOfPpcPages_global, struct queryNodeHederFormat *queryNodeHeder) {

	int i, y;
	char buff[200];
       	char *htmlstring = NULL;
	char url[1024];
	char        *titleaa, *body, *metakeyw, *metadesc;
	float defultbid;
	int casheable;
	char cashename[1024];
	char cashefile[1024];
	FILE *CACHE;
	int hascashe;
	char queryUrlEncoded[80];


	struct ppcPagesFormat ppcPages[10];
	int nrOfPpcPages = 0;



	printf("%s\n",provider);


	strscpy(queryUrlEncoded,(*queryNodeHeder).query,sizeof(queryUrlEncoded));
	strsandr(queryUrlEncoded," ","%20");

	//inaliserer beskrivelse
	for(i=0;i<10;i++) {
		ppcPages[i].description[0] = '\0';
	}

	if (strcmp(provider,"revenuepilot") == 0) {
		sprintf(url,"http://search.revenuepilot.com/servlet/search?mode=xml2&id=24605&tid=0&perpage=5&filter=on&skip=0&related=off&ip=%s&keyword=%s",(*queryNodeHeder).userip,queryUrlEncoded);
		casheable = 0;
	}
	else if (strcmp(provider,"hent") == 0) {
		sprintf(url,"http://www.hent.no/hent/xml.php?cdata=0&strict=1&affiliate=boitho&Terms=%s&IP=%s",queryUrlEncoded,(*queryNodeHeder).userip);
		casheable = 0;
	}
	else if (strcmp(provider,"searchboss") == 0) {
		sprintf(url,"http://xmlfeed.spaex.com/search/searchxml1.asp?p=3092&t=%s&ip=%s",queryUrlEncoded,(*queryNodeHeder).userip);
		casheable = 0;
	}
	else if (strcmp(provider,"amazon") == 0) {

		if (((*queryNodeHeder).AmazonAssociateTag[0] == '\0') 
			|| ((*queryNodeHeder).AmazonSubscriptionId[0] == '\0')) {
			strcpy((*queryNodeHeder).AmazonAssociateTag,BoithosAmazonAssociateTag);
			strcpy((*queryNodeHeder).AmazonSubscriptionId,BoithosAmazonSubscriptionId);

			#ifdef DEBUG
			printf("diden get a AmazonAssociateTag or AmazonSubscriptionId. Useing Boithos\n");
			#endif
			defultbid = 0.1;
		}
		else {
			defultbid = 0; //sory. får ikke penger får egen anonser
		}

		sprintf(url,"http://webservices.amazon.com/onca/xml?Service=AWSECommerceService&SubscriptionId=%s&AssociateTag=%s&Operation=ItemSearch&Keywords=%s&SearchIndex=Books&ResponseGroup=Medium",(*queryNodeHeder).AmazonSubscriptionId,(*queryNodeHeder).AmazonAssociateTag,queryUrlEncoded);
		casheable = 1;
		
		
		sprintf(cashename,"amazon_Q:%sAT:%sST:%s",(*queryNodeHeder).query,(*queryNodeHeder).AmazonAssociateTag,(*queryNodeHeder).AmazonSubscriptionId);
	}
	else {
                printf("unknow prowider: %s\n",provider);
        }


	hascashe = 0;
	if (casheable) {

		sprintf(cashefile,"%s/%s",cashedir,cashename);

		if ((CACHE = fopen(cashefile,"rb")) != NULL) {
                	hascashe = 1;
			printf("has cashe. Useing it\n");

			flock(fileno(CACHE),LOCK_SH);

			if(fread(&nrOfPpcPages,sizeof(nrOfPpcPages),1,CACHE) != 1){
				printf("error reading cashe nrOfPpcPages\n");
				perror("cashe");

			}
			for (i=0; i<nrOfPpcPages;i++) {
				if (fread(&ppcPages[i],sizeof(struct ppcPagesFormat),1,CACHE) != 1) {
					printf("error reading cashe for %i\n\n",i);
					perror("cashe");
					hascashe = 0;
				}
			}
			fclose(CACHE);
		}
	}


	if (!hascashe) {
		httpGet(&htmlstring,url);

		#ifdef DEBUG
			printf("queried %s\n",url);
			FILE *FH;
        	        FH = fopen("boithoads.out","w");
        	        fprintf(FH,"%s",htmlstring);
        	        fclose(FH);
		#endif


		if (strcmp(provider,"revenuepilot") == 0) {
		
		
			parsPpcAds(htmlstring,ppcPages,&nrOfPpcPages,"RESULTS","LISTING",NULL,"TITLE","DESCRIPTION","LINK","BID","DOMAIN");


			//legger til brukernavn
			for(i=0;i<nrOfPpcPages;i++) {
				strcpy(ppcPages[i].user,"boitho_rpilot");
				ppcPages[i].thumbnail[0] = '\0';
				ppcPages[i].keyword_id = 0;
				ppcPages[i].allrank = 10000;
				ppcPages[i].DocID = 0;
				ppcPages[i].DocID = 0;
				find_domain_no_www(ppcPages[i].url,ppcPages[i].domain,sizeof(ppcPages[i].domain));
			}

			//Får av og til sider som har 0.00 som verdi. Forkaster de
			y=0;
			for(i=0;i<nrOfPpcPages;i++) {
				//hvis det er større en 0 får det være med.
				if (ppcPages[i].bid > 0) {
					//printf("ok uri %s, size %i\n",ppcPages[i].uri,sizeof(ppcPages[y]));
					memcpy(&ppcPages[y],&ppcPages[i],sizeof(ppcPages[y]));
					++y;
				}
				else {
					printf("bad ppc %s, bod only %f\n",ppcPages[i].uri,ppcPages[i].bid);
				}
			}

			nrOfPpcPages = y;


			for(i=0;i<nrOfPpcPages;i++) {
				//printf("%i : %s\n",i,ppcPages[i].url);
				//sprintf(ppcPages[i].url,"aaaa %s",ppcPages[i].url);
				strcpy(buff,"http://");
				strcat(buff,ppcPages[i].url);
				strcpy(ppcPages[i].url,buff);
				strcat(ppcPages[i].url,"/");
				//printf("done %i : %s\n",i,ppcPages[i].url);

				for(y=0;y<strlen(ppcPages[i].title);y++) {

                        		if(
                        			(isalnum(ppcPages[i].title[y]))
                        			|| (43 == (unsigned int)ppcPages[i].title[y])
                        			|| (34 == (unsigned int)ppcPages[i].title[y])
                        			|| (32 == (unsigned int)ppcPages[i].title[y])
                        			|| (45 == (unsigned int)ppcPages[i].title[y])
                        			|| (128 < (unsigned int)ppcPages[i].title[y])

                        		) {
                        		        //gjø ingenting for nå
                        		}
                        		else {
                                		ppcPages[i].title[y] = ' ';
                        		}

                		}
				for(y=0;y<strlen(ppcPages[i].description);y++) {

                        		if(
                        			(isalnum(ppcPages[i].description[y]))
                        			|| (43 == (unsigned int)ppcPages[i].description[y])
                        			|| (34 == (unsigned int)ppcPages[i].description[y])
                        			|| (32 == (unsigned int)ppcPages[i].description[y])
                	        		|| (45 == (unsigned int)ppcPages[i].description[y])
        	                		|| (128 < (unsigned int)ppcPages[i].description[y])

        	                	) {
        	                	        //gjø ingenting for nå
        	                	}
        	                	else {
        	                        	ppcPages[i].description[y] = ' ';
        	                	}
	
        	        	}

			}

		}
		else if (strcmp(provider,"hent") == 0) {

		
			parsPpcAds(htmlstring,ppcPages,&nrOfPpcPages,"dsxout","listing","results","title","description","redirect","BID","url");


			//legger til brukernavn
			for(i=0;i<nrOfPpcPages;i++) {
				strcpy(ppcPages[i].user,"boitho_hent");
				ppcPages[i].thumbnail[0] = '\0';
				ppcPages[i].keyword_id = 0;
				ppcPages[i].allrank = 10000;
				ppcPages[i].DocID = 0;
				find_domain_no_www(ppcPages[i].url,ppcPages[i].domain,sizeof(ppcPages[i].domain));
			}

			//hent sender også ut urler uten tracking redirekt. Filtrerer de ut.
			y=0;
			for(i=0;i<nrOfPpcPages;i++) {
				if (strstr(ppcPages[i].uri,"http://www.hent.no/hent/r.php?") != NULL) {
					//printf("ok uri %s, size %i\n",ppcPages[i].uri,sizeof(ppcPages[y]));
					memcpy(&ppcPages[y],&ppcPages[i],sizeof(ppcPages[y]));
					++y;
				}
				else {
					//printf("bad uri %s\n",ppcPages[i].uri);
				}
			}

			nrOfPpcPages = y;



		}
		else if (strcmp(provider,"searchboss") == 0) {

		
		
			parsPpcAds(htmlstring,ppcPages,&nrOfPpcPages,"Results","Result",NULL,"Title","Description","LinkUrl","Bid","DispUrl");



			//legger til brukernavn
			for(i=0;i<nrOfPpcPages;i++) {
				strcpy(ppcPages[i].user,"boitho_sboss");
				ppcPages[i].thumbnail[0] = '\0';
				ppcPages[i].keyword_id = 0;
				ppcPages[i].allrank = 10000;
				ppcPages[i].DocID = 0;
				find_domain_no_www(ppcPages[i].url,ppcPages[i].domain,sizeof(ppcPages[i].domain));
			}

		}
		else if (strcmp(provider,"amazon") == 0) {





			struct nodenamesFormat NodeName = {"ItemSearchResponse","OperationRequest->Items->Item->DetailPageURL","OperationRequest->Items->Item->ItemAttributes->Title","OperationRequest->Items->Item->EditorialReviews->EditorialReview->Content","","","OperationRequest->Items->Item->DetailPageURL","OperationRequest->Items->Item->SmallImage->URL","OperationRequest->Items->Item->SmallImage->Width","OperationRequest->Items->Item->SmallImage->Height"};
		
			
			parsPpcAmazon(htmlstring,ppcPages,&nrOfPpcPages,&NodeName);


		
			//legger til brukernavn
                	for(i=0;i<nrOfPpcPages;i++) {

				strscpy(ppcPages[i].uri,ppcPages[i].url,sizeof(ppcPages[i].uri));

                	        strscpy(ppcPages[i].user,"boitho_amazon",sizeof(ppcPages[i].user));
                	}

			for(i=0;i<nrOfPpcPages;i++) {
			
				ppcPages[i].keyword_id = 0;
				ppcPages[i].allrank = 10000 - i;
				ppcPages[i].DocID = 0;
				ppcPages[i].bid = defultbid;
				//find_domain_no_www(ppcPages[i].url,ppcPages[i].domain,sizeof(ppcPages[i].domain));
				ppcPages[i].domain[0] = '\0';

			

				//noen feed har html i fidene. Parser det ned
				generate_summary( ppcPages[i].description, strlen(ppcPages[i].description), &titleaa, &body, &metakeyw, &metadesc );
					strsandr(body,"<div>","");
					strsandr(body,"</div>","");
					strsandr(body,"<span>","");
					strsandr(body,"</span>","");

					strscpy(ppcPages[i].description,body,sizeof(ppcPages[i].description));

				if (titleaa != NULL) free(titleaa);
        	                if (body != NULL) free(body);
        	                if (metakeyw != NULL) free(metakeyw);
        	                if (metadesc != NULL) free(metadesc);
			
			}
		}
		else {
			printf("unknow prowider: %s\n",provider);
		}

        	free(htmlstring);

		//writing cashe
		CACHE = fopen(cashefile,"wb");
		flock(fileno(CACHE),LOCK_EX);
		fwrite(&nrOfPpcPages,sizeof(nrOfPpcPages),1,CACHE);
		for (i=0; i<nrOfPpcPages;i++) {
	        	fwrite(&ppcPages[i],sizeof(struct ppcPagesFormat),1,CACHE);
	        }
		fclose(CACHE);


	}//!hascashe



	//hvis vi har flere en vi trnger skal vi ikke bry oss mer med de
	if (((*nrOfPpcPages_global) + nrOfPpcPages) > 10) {
		nrOfPpcPages = 10 - (*nrOfPpcPages_global);
	}

	//kopierer inn sidene i den globale side arayen
	for (i=0; i<nrOfPpcPages;i++) {
		
		memcpy(&ppcPages_gloabal[(*nrOfPpcPages_global) +i],&ppcPages[i],sizeof(struct ppcPagesFormat));
		
	}

	(*nrOfPpcPages_global) = (*nrOfPpcPages_global) + nrOfPpcPages;

}
