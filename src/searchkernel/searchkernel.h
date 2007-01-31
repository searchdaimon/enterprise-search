#include "../query/query_parser.h"
#include "../common/define.h"

void dosearch(char query[], int queryLen, struct SiderFormat *Sider, struct SiderHederFormat *SiderHeder, char *hiliteQuery, char servername[],struct subnamesFormat subnames[],int nrOfSubname, int MaxsHits, int start, int filterOn, char languageFilter[], char orderby[], int dates[], char search_user[]);
