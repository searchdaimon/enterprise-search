#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/poprank.h"
#include "../common/reposetory.h"
#include "../common/search_automaton.h"

#include "../parser/html_parser.h"

/*
void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void* wordlist )
{
    //if (pos > 25) return;

    printf("\t%s (%i) ", word, pos);
    printf("pu %i ",pu);

    switch (pu)
        {
            case pu_word: printf("[word]"); break;
            case pu_linkword: printf("[linkword]"); break;
            case pu_link: printf("[link]"); break;
            case pu_baselink: printf("[baselink]"); break;
            case pu_meta_keywords: printf("[meta keywords]"); break;
            case pu_meta_description: printf("[meta description]"); break;
            case pu_meta_author: printf("[meta author]"); break;
            case pu_meta_redirect: printf("[meta redirect]"); break;
            default: printf("[...]");
        }

    switch (puf)
        {
            case puf_none: break;
            case puf_title: printf(" +title"); break;
            case puf_h1: printf(" +h1"); break;
            case puf_h2: printf(" +h2"); break;
            case puf_h3: printf(" +h3"); break;
            case puf_h4: printf(" +h4"); break;
            case puf_h5: printf(" +h5"); break;
            case puf_h6: printf(" +h6"); break;
        }

    printf("\n");
}
*/

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void* pagewords )
{


        //#ifdef DEBUG
                printf("\t%s (%i) ", word, pos);
		printf("type %i ",pu);
        //#endif
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

                                printf("[word] is now %s ",word);
				printf("crc32 %u",crc32boitho(word));                      


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
	unsigned int htmlBufferSize;
	char *htmlBuffer;
	char *acl_allowbuffer = NULL;
	char *acl_deniedbuffer = NULL;

	printf("%i\n",argc);

	int optShowhtml = 0;
	int optShowWords = 0;
	int optSummary = 0;
	int optAnchor = 0;
        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"hwsa"))!=-1) {
                switch (c) {
                        case 'h':
                                optShowhtml = 1;
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
                        default:
                                          exit(1);
                }
        }
        --optind;

        printf("argc %i, optind %i\n",argc,optind);


        if ((argc - optind)!= 3) {
		printf("Dette programet gir info om en DocID\n\n\tUsage PageInfo DocID collection\n");
		exit(1);
	}

	const unsigned int DocID = atol(argv[1 +optind]);
	char *subname = argv[2 +optind];

	html_parser_init();

	printf("subname \"%s\", DocID %u\n",subname,DocID);

	//int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID)

	printf("Lot %i\n",rLotForDOCid(DocID));

	if (DIRead(&DocumentIndexPost,DocID,subname)) {

		printf("Url: %s\nSprok: %s\nOffensive_code: %hu\nDokumenttype: %s\nCrawleDato: %u\nAntallFeiledeCrawl: %hu\nAdultWeight: %hu\nResourceSize: %u\nIPAddress: %u\nresponse: %hu\nhtmlSize: %i\nimageSize: %i\nuserID: %i\nclientVersion: %f\nRepositoryPointer: %u\nhtmlSize %i\n\n",DocumentIndexPost.Url, DocumentIndexPost.Sprok, DocumentIndexPost.Offensive_code, DocumentIndexPost.Dokumenttype,  DocumentIndexPost.CrawleDato, DocumentIndexPost.AntallFeiledeCrawl, DocumentIndexPost.AdultWeight, DocumentIndexPost.ResourceSize, DocumentIndexPost.IPAddress, DocumentIndexPost.response, DocumentIndexPost.htmlSize, DocumentIndexPost.imageSize,DocumentIndexPost.userID, DocumentIndexPost.clientVersion,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize);


		printf("time %s (%u)\n",ctime((time_t *)&DocumentIndexPost.CrawleDato),DocumentIndexPost.CrawleDato);
		printf("crc32 %u\n",DocumentIndexPost.crc32);

		printf("SummaryPointer %u\nSummarySize %hu\n\n",DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize);
		printf("nrOfOutLinks: %u\n",(unsigned int)DocumentIndexPost.nrOfOutLinks);
		///////////////
		char *metadesc, *title, *body;

		if (rReadSummary(DocID,&metadesc, &title, &body,DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize,subname)) {
			printf("title %s\nmetadesc %s\n",title,metadesc);
			if (optSummary) {
				printf("sumary body\n*******************\n%s\n*******************\n\n",body);
			}
		}
		else {
			printf("dont have summery\n");
		}


		///////////////


		if ((optShowhtml) || (optShowWords)) {
			htmlBufferSize = 300000;
			htmlBuffer = malloc(htmlBufferSize);
			struct ReposetoryHeaderFormat ReposetoryHeader;

			rReadHtml(htmlBuffer,&htmlBufferSize,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize,DocID,subname,&ReposetoryHeader,&acl_allowbuffer,&acl_deniedbuffer);

		}
		if (optShowhtml) {

			printf("html uncompresed size %i\n",htmlBufferSize);
			printf("html buff:\n*******************************\n%s\n*******************************\n\n",htmlBuffer);

			#ifdef BLACK_BOKS
				printf("acl_allowbuffer: \"%s\"\n",acl_allowbuffer);
				printf("acl_deniedbuffer: \"%s\"\n",acl_deniedbuffer);
			#endif

		}
		if (optShowWords) {
			printf("words:\n");
			//run_html_parser( DocumentIndexPost.Url, htmlBuffer, htmlBufferSize, fn );
			char *title, *body;
			html_parser_run(DocumentIndexPost.Url,htmlBuffer, htmlBufferSize,&title, &body,fn,NULL );
		}
	}
	else {
		printf("Cant read post\n");
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


		#ifndef BLACK_BOKS

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

		#endif


		printf("done\n");
}



