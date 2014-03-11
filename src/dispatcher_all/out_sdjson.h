#ifndef _H_OUT_SDJSON_
#define _H_OUT_SDJSON_

#include "library.h"

#include "../common/define.h"



void disp_out_sd_json(
	struct SiderHederFormat FinalSiderHeder,
	struct QueryDataForamt QueryData,
	int noDoctype,
	struct SiderHederFormat *SiderHeder,
	int hascashe,
	int nrRespondedServers,
	int num_servers,
	int nrOfAddServers,
	struct filtersTrapedFormat dispatcherfiltersTraped,
	int *sockfd,
	int *addsockfd,
	struct SiderHederFormat *AddSiderHeder,
	struct errorhaFormat errorha,
	struct SiderFormat *Sider,
	struct queryNodeHederFormat queryNodeHeder,
	time_t etime
);


#endif
