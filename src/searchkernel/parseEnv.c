//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "searchkernel.h"


//#include "parseEnv.h"

//#include "../common/define.h"

//#include "grepparseEnv.h"

#include <stdlib.h>


#include "cgi-util.h"

//#include "parseEnv.h"

parseTheEnv (struct QueryDataForamt *QueryData) {

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
		perror("Did'n receive any query.");
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
