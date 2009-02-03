/*
 * Exchange crawler
 *
 * June, 2007
 */


#include <libxml/parser.h>
#include <libxml/xinclude.h>

#include "excrawler.h"
#include "webdav.h"
#include "../dictionarywordsLot/set.h"

typedef struct stringListElement {
	xmlChar *str;
	unsigned int modified;
	int contentlen;
	char id[1024];
	char ownerSid[1024];
	struct stringListElement *next;
} stringListElement;

xmlChar * getHref(const xmlDocPtr doc, xmlNodePtr cur);
void freeStringList(stringListElement * head);
int getEmailUrls(const char *data, struct crawlinfo *ci, char *parent, set *acl_allow, set *acl_deny, char *usersid, CURL **curl, struct loginInfoFormat *login);

