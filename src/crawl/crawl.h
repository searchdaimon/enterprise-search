#ifndef _CRAWL__H_
#define _CRAWL__H_

void crawlperror(const char *fmt, ...);
void crawlWarn(const char *fmt, ...);
char *strcrawlWarn();
char *strcrawlError();

#define crawl_security_acl 1
#define crawl_security_none 1

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
	char *acl;
	char *title;
	char *doctype;
};

struct crawlLibInfoFormat {
	int (*crawlinit)();
	// a pointer to a crawlfirst rutine
	int (*crawlfirst)(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
		int (*documentError)(int level, const char *fmt, ...)
	);

	int (*crawlupdate)(struct collectionFormat *collection,
		int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
	        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
		int (*documentError)(int level, const char *fmt, ...)
	);

	int (*crawlcanconect)(struct collectionFormat *collection,int (*documentError)(int level, const char *fmt, ...));

	int (*crawlpatAcces)(char resource[], char username[], char password[],int (*documentError)(int level, const char *fmt, ...));

	int (*scan)(int (*scan_found_share)(char share[]),char host[],char username[], char password[],int (*documentError)(int level, const char *fmt, ...));

	int crawl_security;
	char *shortname;
	char *(*strcrawlError)();
};


#endif // _CRAWL__H_
