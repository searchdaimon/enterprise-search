#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/poprank.h"
#include "../common/reposetory.h"
#include "../common/langToNr.h"
#include "../common/search_automaton.h"
#include "../common/bstr.h"

#include "../cgi-util/cgi-util.h"
#include "../parser2/html_parser.h"
#include "../IndexerRes/IndexerRes.h"

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void* pagewords )
{

        #ifdef DEBUG
                printf("\t%s (%i) ", word, pos);
		printf("type %i ",pu);
        #endif
        switch (pu)
        {
            case pu_word:


                                switch (puf)
                                {
                                        case puf_none: printf(" none"); break;
                                        case puf_title: printf(" +title"); break;
                                        case puf_h1: printf(" +h1"); break;
                                        case puf_h2: printf(" +h2"); break;
                                        case puf_h3: printf(" +h3"); break;
                                        case puf_h4: printf(" +h4"); break;
                                        case puf_h5: printf(" +h5"); break;
                                        case puf_h6: printf(" +h6"); break;
                                }

				convert_to_lowercase(word);

                                printf("[word] is now %s (crc32 %u, pos %i)", word, crc32boitho(word), pos);


                break;
            case pu_linkword:
                                printf("[linkword]");
                break;
            case pu_link:
                                printf("[link]");
                break;
            case pu_baselink:
                                printf("[baselink]");
                break;
            case pu_meta_keywords:
                                printf("[meta keywords]");
                        break;
            case pu_meta_description:
                                printf("[meta description]");
                        break;
            case pu_meta_author:
                                printf("[meta author]");
                        break;
            default: printf("[...]");
        }

        printf("\n");

}

#ifdef BLACK_BOX
char *aclResolvEl(char value[]) {

	static char user[64];


        if (boithoad_sidToGroup(value, user)) {

	}	
        else {
		strcpy(user,value);
	}

	return user;

}

char *aclResolv(char acl[]) {
	char **Data;
	int Count;
	static char ret[1024];

	ret[0] = '\0';

	if (split(acl, ",", &Data) == 0) {
		return acl;
	}

        Count = 0;
        while( (Data[Count] != NULL) ) {

		if(Data[Count][0] == '\0') {
			++Count;
			continue;
		}
		if(Count != 0) {
			strcat(ret,",");
		}
		strcat(ret,aclResolvEl(Data[Count]));

		++Count;
	}

	return ret;

}
#endif

int main (int argc, char *argv[]) {

        struct DocumentIndexFormat DocumentIndexPost;
	int PopRankextern;
	int PopRankintern;
	int PopRanknoc;
	int PopRanindex;
	char ShortRank;
	FILE *FH;
        struct popl popextern;
        struct popl popintern;
        struct popl popnoc;
	struct popl popindex;	
	uLong htmlBufferSize = 0;
	char *htmlBuffer = NULL;
	char *acl_allowbuffer = NULL;
	char *acl_deniedbuffer = NULL;

	char timebuf[26];

	int optShowhtml = 0;
	int optShowWords = 0;
	int optSummary = 0;
	int optAnchor = 0;
	int optResource = 0;
	int optPopRank = 0;
	int optDelete = 0;
	int optAdult = 0;

	unsigned int DocID;
	char *subname;

	if (getenv("QUERY_STRING") == NULL) {

	        extern char *optarg;
	        extern int optind, opterr, optopt;
	        char c;
	        while ((c=getopt(argc,argv,"hwsarpdu"))!=-1) {
	                switch (c) {
	                        case 'h':
	                                optShowhtml = 1;
	                                break;
	                        case 'u':
	                                optAdult = 1;
	                                break;
	                        case 'w':
					optShowWords = 1;
	                                break;
	                        case 's':
					optSummary = 1;
	                                break;
	                        case 'a':
					optAnchor = 1;
	                                break;
				case 'p':
					optPopRank = 1;
					break;
				case 'r':
					optResource = 1;
					break;
				case 'd':
					optDelete = 1;
					break;
	                        default:
	                                          exit(1);
	                }
	        }
	        --optind;

		#ifdef DEBUG
	        printf("argc %i, optind %i\n",argc,optind);
		#endif

	        if ((argc - optind)!= 3) {
			printf("Dette programet gir info om en DocID\n\n\tUsage PageInfo DocID collection\n");
			exit(1);
		}

		
		DocID = atol(argv[1 +optind]);
		subname = argv[2 +optind];


	}
	else {
		printf("Content-type: text/plain\n\n");
		int res;
	        // Initialize the CGI lib
	        res = cgi_init();

	        // Was there an error initializing the CGI???
	        if (res != CGIERR_NONE) {
	                printf("Error # %d: %s<p>\n", res, cgilib_strerror(res));
	                fprintf(stderr,"Cgi-lib error.");
			return -1;
	        }

	        if (cgi_getentrystr("subname") == NULL) {
	                fprintf(stderr,"Didn't recieve any subname.");
			return -1;
	        }
	        else {
	                subname = cgi_getentrystr("subname");
	        }

	        if (cgi_getentrystr("DocID") == NULL) {
	                fprintf(stderr,"Didn't recieve any DocID.");
			return -1;
	        }
	        else {
	                DocID = atol( cgi_getentrystr("DocID") );
	        }

	}

	html_parser_init();

	printf("Showing data for Collection \"%s\", DocID %u\n\n",subname,DocID);


	printf("Lot: %i\n",rLotForDOCid(DocID));

	if (optDelete) {
		memset(&DocumentIndexPost,'\0',sizeof(DocumentIndexPost));
		DIWrite(&DocumentIndexPost,DocID,subname,NULL);
		
		return 0;
	}

	if (DIRead_fmode(&DocumentIndexPost,DocID,subname,'s')) {

		printf("Url: \"%s\"\nLanguage: %s (id: %s)\nOffensive code: %hu\nDocument type: %s\nTime tested sins last good crawl: %hu\nAdult weight: %hu\nResource size: %u\nIP Address: %u\nHtml size: %i\nImage size: %i\nUser ID: %i\nCrawler version: %f\nRepository pointer: %u\n",

			DocumentIndexPost.Url, 
			getLangCode2(atoi(DocumentIndexPost.Sprok)), 
			DocumentIndexPost.Sprok,
			DocumentIndexPost.Offensive_code, 
			DocumentIndexPost.Dokumenttype, 
			DocumentIndexPost.AntallFeiledeCrawl, 
			DocumentIndexPost.AdultWeight, 
			DocumentIndexPost.ResourceSize, 
			DocumentIndexPost.IPAddress, 
			DocumentIndexPost.htmlSize2, 
			DocumentIndexPost.imageSize,
			DocumentIndexPost.userID, 
			DocumentIndexPost.clientVersion,
			DocumentIndexPost.RepositoryPointer);

		if (DocumentIndexPost.response == 200) {
			printf("HTTP response: %hu\n",DocumentIndexPost.response);
		}
		else {
			printf("HTTP response: \033[1;31m%hu\033[0m\n",DocumentIndexPost.response);

		}


		ctime_r((time_t *)&DocumentIndexPost.CrawleDato,timebuf);
		timebuf[24] = '\0';


		printf("Last crawled time: %u\n",DocumentIndexPost.CrawleDato);
		printf("Last crawled time ISO: %s\n",timebuf);
	
		printf("crc32: %u\n",DocumentIndexPost.crc32);

#ifdef BLACK_BOX
		printf("Last seen Unix: %u\n",DocumentIndexPost.lastSeen);
		printf("Last seen ISO: %s", ctime(&DocumentIndexPost.lastSeen));
#endif

		printf("Nr of out links: %u\n",(unsigned int)DocumentIndexPost.nrOfOutLinks);


		#ifndef BLACK_BOX
		char *metadesc, *title, *body;
		if (DocumentIndexPost.SummarySize == 0) {
			printf("Summary: Don't have pre-parsed summery (summary size is 0)\n");

		}
		else if (rReadSummary(DocID,&metadesc, &title, &body,DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize,subname)) {
			printf("\nSummary:\n");
			printf("\tSummary pointer: %u\n\tSummary size: %hu\n",DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize);

			printf("\tTitle from summary:  \"%s\"\n\tMeta description from summary: \"%s\"\n",title,metadesc);
			if (optSummary) {
				printf("Summary body\n*******************\n%s\n*******************\n\n",body);
			}
		}
		else {
			printf("Don't have pre-parsed summery\n");
		}
		#endif



		struct ReposetoryHeaderFormat ReposetoryHeader;
		char *url, *attributes;

		if (!rReadHtml(&htmlBuffer,&htmlBufferSize,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize2,DocID,subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer,DocumentIndexPost.imageSize, &url, &attributes)) {
			printf("rReadHtml: did not returne true!\n");
			return;
		}
		printf("Entire url: %s\n", url);

		#ifdef BLACK_BOX
			printf("acl allow raw: \"%s\"\n",acl_allowbuffer);
			printf("acl denied raw: \"%s\"\n",acl_deniedbuffer);

			printf("acl allow resolved: \"%s\"\n",aclResolv(acl_allowbuffer));
			printf("acl denied resolved: \"%s\"\n",aclResolv(acl_deniedbuffer));

			printf("PopRank: %d\n", ReposetoryHeader.PopRank);
		#endif

		if (optShowhtml) {

			printf("html uncompresed size %i\n",htmlBufferSize);
			printf("html buff:\n*******************************\n");
			fwrite(htmlBuffer,htmlBufferSize,1,stdout);
			printf("\n*******************************\n\n");


		}
		if (optShowWords) {
			printf("words:\n");
			//run_html_parser( DocumentIndexPost.Url, htmlBuffer, htmlBufferSize, fn );
			char *title, *body;
			html_parser_run(url,htmlBuffer, htmlBufferSize,&title, &body,fn,NULL );
		}
		if (optResource) {
			char buf[500000];
			printf("Resource:\n");
			printf("Ptr: 0x%x Len: %x\n", DocumentIndexPost.ResourcePointer, DocumentIndexPost.ResourceSize);
			if (getResource(rLotForDOCid(DocID), subname, DocID, buf, sizeof(buf)) == 0) {
				printf("\tDid not get any resource\n");
				warn("");
			} else {
				printf("%s\n", buf);
			}
		}

		printf("attributes:\"%s\"\n", attributes);

		free(url);
		free(attributes);
		free(acl_allowbuffer);
		free(acl_deniedbuffer);
	}
	else {
		printf("Cant read post\n");
	}

	#ifndef BLACK_BOX

		if (optAdult) {
			int httpResponsCodes[nrOfHttpResponsCodes];
	        	//char *title;
	        	//char *body;
			struct adultFormat *adult;
			struct pagewordsFormat *pagewords = malloc(sizeof(struct pagewordsFormat));
			int AdultWeight;
			unsigned char langnr;
        		if ((adult = malloc(sizeof(struct adultFormat))) == NULL) {
        		        perror("malloc argstruct.adult");
        		        exit(1);
	        	}

			wordsInit(pagewords);
      			langdetectInit();
		        adultLoad(adult);

			AdultWeight -1;

			handelPage(pagewords,&ReposetoryHeader,htmlBuffer,htmlBufferSize,&title,&body);

			wordsMakeRevIndex(pagewords,adult,&AdultWeight,&langnr);

			printf("adult %i\n",AdultWeight);
		}

		if (optAnchor) {
			int anchorBufferSize;
			char *anchorBuffer;
	
			anchorBufferSize = anchorRead(rLotForDOCid(DocID),subname,DocID,NULL,-1);
			anchorBufferSize += 1;
			anchorBuffer = malloc(anchorBufferSize);
			anchorRead(rLotForDOCid(DocID),subname,DocID,anchorBuffer,anchorBufferSize);

			printf("#######################################\nanchors:\n%s\n#######################################\n",anchorBuffer);

			free(anchorBuffer);
		}
	


		if (optPopRank) {
			popopen (&popindex,"/home/boitho/config/popindex");
			PopRanindex = popRankForDocID(&popindex,DocID);		
			popclose(&popindex);
			printf("popindex %i\n",PopRanindex);

			if (popopen (&popextern,"/home/boitho/config/popextern")) {
				PopRankextern =  popRankForDocID(&popextern,DocID);
				printf("PopRankextern: %i\n",PopRankextern);
				popclose(&popextern);
			}
        		if (popopen (&popintern,"/home/boitho/config/popintern")) {
				PopRankintern =  popRankForDocID(&popintern,DocID);
				printf("PopRankintern %i\n",PopRankintern);
				popclose(&popintern);
			}
        		if (popopen (&popnoc,"/home/boitho/config/popnoc")) {
				PopRanknoc =  popRankForDocID(&popnoc,DocID);
				printf("PopRanknoc %i\n",PopRanknoc);
				popclose(&popnoc);
			}
			if (popopen (&popindex,"/home/boitho/config/popindex")) {
				PopRanindex = popRankForDocID(&popindex,DocID);		
				printf("popindex %i\n",PopRanindex);
				popclose(&popindex);
			}



			printf("PopRankextern: %i\nPopRankintern %i\nPopRanknoc %i\n",PopRankextern,PopRankintern,PopRanknoc);
		

			int brank;
			popopenMemArray_oneLot(subname,rLotForDOCid(DocID));
			brank = popRankForDocIDMemArray(DocID);
			printf("brank %i\n",brank);
			//short rank
			if ( (FH = fopen(SHORTPOPFILE,"rb")) == NULL ) {
                		perror("open");
        		}
			else {
				if ((fseek(FH,DocID* sizeof(ShortRank),SEEK_SET) == 0) && (fread(&ShortRank,sizeof(ShortRank),1,FH) != 0)){
	
					printf("Short rank %u\n",(unsigned char)ShortRank);
				}
				else {
					printf("no hort rank avalibal\n");
				};
		
				fclose(FH);
			}
		} // if optPopRank
	#endif


}



