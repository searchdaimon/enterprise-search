#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/poprank.h"
#include "../common/reposetory.h"

#include "../parser/html_parser.h"

void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf );

int main (int argc, char *argv[]) {

        struct DocumentIndexFormat DocumentIndexPost;
	unsigned int DocID;
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

	printf("%i\n",argc);

	int optShowhtml = 0;
	int optShowWords = 0;
        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"hw"))!=-1) {
                switch (c) {
                        case 'h':
                                optShowhtml = 1;
                                break;
                        case 'w':
				optShowWords = 1;
                                break;
                        default:
                                          exit(1);
                }
        }
        --optind;

        printf("argc %i, optind %i\n",argc,optind);


        if ((argc - optind)!= 3) {
		printf("Dette programet gir info om en DocID\n\n\tUsage PageInfo DocID\n");
	}

	DocID = atol(argv[1 +optind]);
	char *subname = argv[2 +optind];

	printf("subname \"%s\", DocID %u\n",subname,DocID);

	//int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID)

	printf("Lot %i\n",rLotForDOCid(DocID));

	if (DIRead(&DocumentIndexPost,DocID,subname)) {

		printf("Url: %s\nSprok: %s\nOffensive_code: %hu\nDokumenttype: %s\nCrawleDato: %u\nAntallFeiledeCrawl: %hu\nAdultWeight: %hu\nResourceSize: %u\nIPAddress: %u\nresponse: %hu\nhtmlSize: %i\nimageSize: %i\nuserID: %i\nclientVersion: %f\nRepositoryPointer: %u\nhtmlSize %i\n\n",DocumentIndexPost.Url, DocumentIndexPost.Sprok, DocumentIndexPost.Offensive_code, DocumentIndexPost.Dokumenttype,  DocumentIndexPost.CrawleDato, DocumentIndexPost.AntallFeiledeCrawl, DocumentIndexPost.AdultWeight, DocumentIndexPost.ResourceSize, DocumentIndexPost.IPAddress, DocumentIndexPost.response, DocumentIndexPost.htmlSize, DocumentIndexPost.imageSize,DocumentIndexPost.userID, DocumentIndexPost.clientVersion,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize);


		printf("time %s (%u)\n",ctime((time_t *)&DocumentIndexPost.CrawleDato),DocumentIndexPost.CrawleDato);
		printf("crc32 %u\n",DocumentIndexPost.crc32);

		printf("SummaryPointer %u\nSummarySize %hu\n\n",DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize);

		///////////////
		char *metadesc, *title, *body;

		if (rReadSummary(&DocID,&metadesc, &title, &body,DocumentIndexPost.SummaryPointer,DocumentIndexPost.SummarySize,subname)) {
			printf("title %s\nmetadesc %s\n",title,metadesc);
			//printf("body\n%s\n\n",body);
		}
		else {
			printf("dont have summery\n");
		}


		///////////////


		if ((optShowhtml) || (optShowWords)) {
			htmlBufferSize = 300000;
			htmlBuffer = malloc(htmlBufferSize);
			char *aclbuffer = NULL;
			struct ReposetoryHeaderFormat ReposetoryHeader;

			rReadHtml(htmlBuffer,&htmlBufferSize,DocumentIndexPost.RepositoryPointer,DocumentIndexPost.htmlSize,DocID,subname,&ReposetoryHeader,&aclbuffer);
		}
		if (optShowhtml) {

			printf("html uncompresed size %i\n",htmlBufferSize);
			printf("html buff:\n*******************************\n%s\n*******************************\n\n",htmlBuffer);
		}
		if (optShowWords) {
			printf("words:\n");
			run_html_parser( DocumentIndexPost.Url, htmlBuffer, htmlBufferSize, fn );
		}
	}
	else {
		printf("Cant read post\n");
	}


		popopen (&popextern,"/home/boitho/config/popextern");
        	popopen (&popintern,"/home/boitho/config/popintern");
        	popopen (&popnoc,"/home/boitho/config/popnoc");
		popopen (&popindex,"/home/boitho/config/popindex");

		PopRankextern =  popRankForDocID(&popextern,DocID);
		PopRankintern =  popRankForDocID(&popintern,DocID);
		PopRanknoc =  popRankForDocID(&popnoc,DocID);
		PopRanindex = popRankForDocID(&popindex,DocID);		

		popclose(&popextern);
        	popclose(&popintern);
        	popclose(&popnoc);
		popclose(&popindex);

		printf("PopRankextern: %i\nPopRankintern %i\nPopRanknoc %i\npopindex %i\n",PopRankextern,PopRankintern,PopRanknoc,PopRanindex);



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
}






void fn( char* word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf )
{


        #ifdef DEBUG
                printf("\t%s (%i) ", word, pos);
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

                                printf("[word] is now %s ",word);


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



