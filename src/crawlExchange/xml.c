/*
 * Exchange crawler
 *
 * June, 2007
 */

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlerror.h>

#include <string.h>
#include <time.h>

#include "xml.h"

/*
 * XXX:
 * Will most probably segfault on a malformed xml structure
 */

unsigned int
ex_parsetime(xmlChar *time)
{
	struct tm tm;

	/* XXX: Does not handle time zones */
	strptime((char *)time, "%Y-%m-%dT%H:%M:%S.", &tm);
	
	return mktime(&tm);
}

xmlChar *
getInfo(const xmlDocPtr doc, xmlNodePtr cur, unsigned int *modified, int *size, char *id)
{
	xmlChar *ahref = NULL;

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (xmlStrcmp(cur->name, (const xmlChar *)"href") == 0) {
			ahref = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		} else if (xmlStrcmp(cur->name, (const xmlChar *)"propstat") == 0) {
			xmlNodePtr props;

			for (props = cur->xmlChildrenNode; props; props = props->next) {
				xmlNodePtr props2;
				if (xmlStrcmp(props->name, (xmlChar *)"prop") == 0) {
					for (props2 = props->xmlChildrenNode; props2; props2 = props2->next) {
						if (xmlStrcmp(props2->name, (xmlChar *)"getlastmodified") == 0)
							*modified = ex_parsetime(xmlNodeListGetString(doc, props2->xmlChildrenNode, 1));
						if (xmlStrcmp(props2->name, (xmlChar *)"getcontentlength") == 0)
							*size = atoi((char *)xmlNodeListGetString(doc, props2->xmlChildrenNode, 1));
						if (xmlStrcmp(props2->name, (xmlChar *)"xfff0102") == 0)
							strcpy(id, (char *)xmlNodeListGetString(doc, props2->xmlChildrenNode, 1));

					}
				}
			}
		}
	}
	return ahref;
}

stringListElement *
getEmailUrls(const char *data)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar * str;
	stringListElement * head = NULL, * tail = NULL;
	doc = xmlParseMemory(data, strlen(data));
	if (!doc) {
		fprintf(stderr, "Parse error!\n");
		return NULL;
	}
	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		fprintf(stderr, "Empty document\n");
		xmlFreeDoc(doc);
		return NULL;
	}
	//printf("Root node: %s\n", cur->name);
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (xmlStrcmp(cur->name, (const xmlChar *)"response") == 0) {
			unsigned int modified;
			int size;
			char id[1024];

			str = getInfo(doc, cur, &modified, &size, id);
			if (str) {
				if (tail) {
					tail->next = malloc(sizeof(stringListElement));
					tail = tail->next;
					tail->next = NULL;
					tail->str = str;
					tail->modified = modified;
					tail->contentlen = size;
					strcpy(tail->id, id);
				}
				else {
					head = malloc(sizeof(stringListElement));
					head->next = NULL;
					head->str = str;
					head->modified = modified;
					head->contentlen = size;
					strcpy(head->id, id);
					tail = head;
				}
			}
		}
	}
	xmlFreeDoc(doc);
	return head;
}

void
freeStringList(stringListElement * head)
{
	stringListElement *next;

	while (head) {
		xmlFree(head->str);
		next = head->next;
		free(head);
		head = next;
	}
}

