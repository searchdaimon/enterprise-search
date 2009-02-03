#ifndef _EXCRAWLER_H_
#define _EXCRAWLER_H_

#include "../dictionarywordsLot/set.h"
#include "../crawl/crawl.h"
#include "webdav.h"

struct crawlinfo {
	int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *);
	int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *);
	int (*documentError)(struct collectionFormat *, int, const char *, ...);
	int (*documentContinue)(struct collectionFormat *);
	struct collectionFormat *collection;
	unsigned int timefilter;
};

int grabContent(char *xml, char *url, struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *usersid, CURL **curl, struct loginInfoFormat *login);
void grab_email(struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *url, char *sid, size_t contentlen, time_t lastmodified, char *usersi, CURL **curl);



#endif /* _EXCRAWLER_H_ */
