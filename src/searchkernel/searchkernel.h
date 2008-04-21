#include "../query/query_parser.h"
#include "../common/define.h"
#include "../common/integerindex.h"

struct searchd_configFORMAT {
        int     newsockfd;
	int cmc_port;
};


int dosearch(char query[], int queryLen, struct SiderFormat **Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames,
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[],
char search_user[],struct filtersFormat *filters,
	struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen, struct iintegerMemArrayFormat *DomainIDs, char *, char *);

#define RANK_TYPE_FIND	1
#define RANK_TYPE_SUM	2

int dorank(char query[], int queryLen, struct SiderFormat **Sider, struct SiderHederFormat *SiderHeder,
char *hiliteQuery, char servername[], struct subnamesFormat subnames[], int nrOfSubnames,
int MaxsHits, int start, int filterOn, char languageFilter[],char orderby[],int dates[],
char search_user[],struct filtersFormat *filters,
	struct searchd_configFORMAT *searchd_config, char *errorstr,int *errorLen, struct iintegerMemArrayFormat *DomainIDs,
	int, unsigned int, int *);
