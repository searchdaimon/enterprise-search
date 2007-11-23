
#ifndef _CRAWLSMB_H_
#define _CRAWLSMB_H_

#include "../crawl/crawl.h"
#include <libsmbclient.h>

#include "../boitho-bbdn/bbdnclient.h"

char* smb_mkprefix( const char *username, const char *passwd );

int smb_recursive_get( char *prefix, char *dir_name,
	struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
	int (*documentError)(int level, const char *fmt, ...),
	unsigned int timefilter,
	int no_auth
	 );

int smb_test_open( char *prefix, char *dir_name , int (*documentError)(int level, const char *fmt, ...));
int smb_test_conect( char *prefix, char *dir_name ,int no_auth, int (*documentError)(int level, const char *fmt, ...));

#endif	// _CRAWLSMB_H_
