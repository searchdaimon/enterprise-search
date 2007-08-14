/*
 * Exchange crawler
 *
 * June, 2007
 */


#include <libxml/parser.h>
#include <libxml/xinclude.h>

typedef struct stringListElement {
	xmlChar *str;
	unsigned int modified;
	int contentlen;
	char id[1024];
	struct stringListElement *next;
} stringListElement;

xmlChar * getHref(const xmlDocPtr doc, xmlNodePtr cur);
stringListElement * getEmailUrls(const char * const data);
void freeStringList(stringListElement * head);

