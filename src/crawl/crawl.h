#ifndef _CRAWL__H_
#define _CRAWL__H_

#include <limits.h>

#include "../common/define.h"

void crawlperror(const char *fmt, ...);
void crawlWarn(const char *fmt, ...);
char *strcrawlWarn();
char *strcrawlError();

#define crawl_security_acl 1
#define crawl_security_none 1


struct crawlLibInfoFormat;

struct collectionFormat {
	char *resource;
	char *connector;
	char *password;
	char *query1;
	char *query2;
	char *collection_name; 
	char *user;
	int socketha;
	unsigned int lastCrawl;
	char *host;
	int auth_id;
	unsigned int id;
	char *userprefix;
	char **users;
	char *extra;
	char *test_file_prefix;
	struct crawlLibInfoFormat *crawlLibInfo;
};

struct crawldocumentExistFormat {
	char *documenturi;
	int dokument_size;
	unsigned int lastmodified;
};

struct crawldocumentAddFormat {
	char *documenturi;
	char *documenttype;
	char *document;
        int dokument_size;
	unsigned int lastmodified;
	//char *acl;
	char *acl_allow;
	char *acl_denied;
	char *title;
	char *doctype;
};

//argumenter til crawlfirst()
#define crawlfirst_args struct collectionFormat *collection, \
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), \
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd), \
		int (*documentError)(int level, const char *fmt, ...), \
		int (*documentContinue)(struct collectionFormat *collection)
//argumenter ter crawlupdate()
#define crawlupdate_args struct collectionFormat *collection, \
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), \
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd), \
		int (*documentError)(int level, const char *fmt, ...), \
		int (*documentContinue)(struct collectionFormat *collection)


struct crawlLibInfoFormat {
	int (*crawlinit)();
	// a pointer to a crawlfirst rutine
	int (*crawlfirst)( crawlfirst_args );

	int (*crawlupdate)( crawlupdate_args );

	int (*crawlcanconect)(struct collectionFormat *collection,int (*documentError)(int level, const char *fmt, ...));

	int (*crawlpatAcces)(char resource[], char username[], char password[],int (*documentError)(int level, const char *fmt, ...));

	int (*scan)(int (*scan_found_share)(char share[]),char host[],char username[], char password[],int (*documentError)(int level, const char *fmt, ...));
	int (*rewrite_url)(char *, enum platform_type, enum browser_type);

	int crawl_security;
	//char *shortname;
	//22 mai 2008;
	char shortname[50];
	char *(*strcrawlError)();

	char resourcepath[PATH_MAX];
};


struct cargsF {

        struct collectionFormat *collection;
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist);
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd);
        int (*documentError)(int level, const char *fmt, ...);
        int (*documentContinue)(struct collectionFormat *collection);

};

void *collectionReset (struct collectionFormat *collection);
#endif // _CRAWL__H_
