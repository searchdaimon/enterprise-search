#include <stdlib.h>

#include "../common/define.h"
#include "cgi-util.h"


parseEnv (struct QueryDataForamt *QueryData) {

	int res;

	//inaliserer
	QueryData->query[0] = '\0';
	

	// Initialize the CGI lib
	res = cgi_init();
  
	// Was there an error initializing the CGI???
	if (res != CGIERR_NONE) {
      		printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
      		exit(0);
    	}

	if (cgi_getentrystr("query") == NULL) {
		gerror("Did'n receive any query.");
	}
	else {
		//printf("query=%s<p>\n", cgi_getentrystr("query"));
		strncat(QueryData->query,cgi_getentrystr("query"),sizeof(QueryData->query));
	}

	// debugg
	//setter query
	//strncat(QueryData->query,"sex",3);
	///debug
}
