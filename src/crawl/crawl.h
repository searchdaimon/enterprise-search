#ifndef _CRAWL__H_
#define _CRAWL__H_

#include <limits.h>

#include "../common/define.h"
#include "../key/key.h"

void crawlWarn(const char *fmt, ...);
char *strcrawlWarn();

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
	unsigned int usersystem;
	struct crawlLibInfoFormat *crawlLibInfo;
	char errormsg[512];
	unsigned int rate;
        struct hashtable * params;
	// Nr of documents yet to crawl.
	// Set to -1 to crawl the entire collection.
	int docsRemaining;
	void *timeusage;
	char *alias;
	char systemkey[KEY_STR_LEN];
	int docsCount;
	unsigned int pid;
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
	char *acl_allow;
	char *acl_denied;
	char *title;
	char *doctype;
	char *attributes;
	char *image;
	int image_size;
};

//argumenter til crawlfirst()
#define crawlfirst_args struct collectionFormat *collection, \
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), \
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd), \
		int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...), \
		int (*documentContinue)(struct collectionFormat *collection)
//argumenter ter crawlupdate()
#define crawlupdate_args struct collectionFormat *collection, \
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), \
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd), \
		int (*documentError)(struct collectionFormat *collection,int level, const char *fmt, ...), \
		int (*documentContinue)(struct collectionFormat *collection)


struct crawlLibInfoFormat {
	int (*crawlinit)();
	int (*crawlfirst)( crawlfirst_args );
	int (*crawlupdate)( crawlupdate_args );
	int (*crawlcanconect)(struct collectionFormat *collection,int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...));
	int (*crawlpatAcces)(char resource[], char username[], char password[],int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...), struct collectionFormat *collection);
	int (*scan)(int (*scan_found_share)(char share[]),char host[],char username[], char password[], int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...));
	int (*rewrite_url)(struct collectionFormat *, char *, char *, char *, size_t, enum platform_type, enum browser_type);

	int crawl_security;
	char shortname[50];
	char resourcepath[PATH_MAX];

};


struct cargsF {

        struct collectionFormat *collection;
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist);
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd);
        int (*documentError)(struct collectionFormat *collection, int level, const char *fmt, ...);
        int (*documentContinue)(struct collectionFormat *collection);

};

void *collectionReset (struct collectionFormat *collection);

char *params_get_char_value (struct hashtable *params, char value[]);

#endif // _CRAWL__H_
