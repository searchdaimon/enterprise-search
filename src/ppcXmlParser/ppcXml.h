
#include "../common/define.h"

struct ppcPagesFormat {

        char title[70];
        char description[255];
        char uri[1024];
        char url[1024]; //ToDo: har vi økt alle andre også ??
        float bid;
	char user[21];
	char domain[65];
	char thumbnail[128];
	char thumbnailwidth[5];
	char thumbnailheight[5];
	int allrank;
	int keyword_id;
	unsigned int DocID;
};


struct nodenamesFormat {
        char rootNodeName[162];
	char startNodeName[162]; //start value. The fist in an new element
        char titleNodeName[162];
        char descriptionNodeName[162];
        char uriNodeName[162];
        char bidNodeName[162];
        char urlNodeName[162];
	char thumbnailNodeName[162];
        char thumbnailwidthNodeName[162];
        char thumbnailheightNodeName[162];
};



void getPpcAds(char provider[],struct ppcPagesFormat *ppcPages_gloabal, int *nrOfPpcPages_gloabal,struct queryNodeHederFormat *queryNodeHeder);
void parsPpcAds(char *docname,struct ppcPagesFormat *ppcPages, int *nrOfPpcPages,char rootNodeName[],
char listningNodeName[], char listningSubNodeName[],char titleNodeName[],char descriptionNodeName[], char uriNodeName[],
char bidNodeName[], char urlNodeName[]);

void parsPpcAmazon(char *docname,struct ppcPagesFormat *ppcPages, int *nrOfPpcPages, struct nodenamesFormat *NodeName);

