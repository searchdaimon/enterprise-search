#ifndef _H_DISPATCHER_CGIHANDLER
#define _H_DISPATCHER_CGIHANDLER
#include "../common/define.h"
#include <libconfig.h>

#define ACCESS_TYPE_NONE 0
#define ACCESS_TYPE_LIMITED 1
#define ACCESS_TYPE_FULL 2

void dispatcher_cgi_init(void);
int cgi_access_type(struct config_t *cfg, char *remoteaddr);
void cgi_set_defaults(struct QueryDataForamt *qdata);
void cgi_fetch_common(struct QueryDataForamt *qdata, int *noDocType);
void cgi_fetch_limited(struct QueryDataForamt *qdata, char *remoteaddr);
void cgi_fetch_full(struct QueryDataForamt *qdata);
#endif
