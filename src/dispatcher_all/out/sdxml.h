#ifndef _H_OUT_SDXML_
#define _H_OUT_SDXML
#include "../../common/define.h"
#include "../library.h"

void disp_out_sd_v2_0(
	struct SiderHederFormat FinalSiderHeder,
        struct QueryDataForamt QueryData,
	int noDoctype,
        struct SiderHederFormat *SiderHeder,
	int hascashe,
	int hasprequery,
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

void disp_out_sd_v2_1(
	struct SiderHederFormat FinalSiderHeder,
        struct QueryDataForamt QueryData,
	int noDoctype,
        struct SiderHederFormat *SiderHeder,
	int hascashe,
	int hasprequery,
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
