#ifndef _EXCRAWLER_H_
#define _EXCRAWLER_H_

#include "../dictionarywordsLot/set.h"
#include "../crawl/crawl.h"

struct crawlinfo {
	int (*documentExist)(struct collectionFormat *, struct crawldocumentExistFormat *);
	int (*documentAdd)(struct collectionFormat *, struct crawldocumentAddFormat *);
	int (*documentError)(int, const char *, ...);
	struct collectionFormat *collection;
	unsigned int timefilter;
};

int grabContent(char *xml, char *url, struct crawlinfo *ci, set *acl_allow, set *acl_deny);
void grab_email(struct crawlinfo *ci, set *acl_allow, set *acl_deny, char *url, char *sid, size_t contentlen, time_t lastmodified);



#endif /* _EXCRAWLER_H_ */
