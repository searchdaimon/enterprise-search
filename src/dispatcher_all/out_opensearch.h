#ifndef _H_OUT_OPENSEARCH
#define _H_OUT_OPENSEARCH
#include "../common/define.h"
//void disp_out_opensearch(struct QueryDataForamt *query, int total_res, struct SiderFormat *results, struct queryNodeHederFormat *queryNodeHeder, int num_servers);
void disp_out_opensearch(int total_res, struct SiderFormat *results, struct queryNodeHederFormat *queryNodeHeder, int num_servers, int start, int res_per_page, char *query_escaped);
#endif
