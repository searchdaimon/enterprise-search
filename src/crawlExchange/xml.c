/*
 * Exchange crawler
 *
 * June, 2007
 */

#define _XOPEN_SOURCE
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlerror.h>

#include <string.h>
#include <time.h>
#include <err.h>

#include "xml.h"
#include "webdav.h"
#include "excrawler.h"

#include "../dictionarywordsLot/set.h"
#include "../crawl/crawl.h"
#include "../base64/base64.h"

/*
 * XXX:
 * Will most probably segfault on a malformed xml structure
 */

time_t
ex_parsetime(char *time)
{
	struct tm tm;

	/* XXX: Does not handle time zones */
	if (strptime(time, "%Y-%m-%dT%H:%M:%S.", &tm) == NULL)
		warn("strptime");
	
	return mktime(&tm);
}

#if 0
void
dumptree(xmlNodePtr n, int indent)
{
	int i;
        xmlNodePtr cur;

        for (i = 0; i < indent; i++)
                printf("  ");

        printf("%s\n", n->name);

        for (cur = n->xmlChildrenNode; cur; cur = cur->next) {

                if (cur->xmlChildrenNode)
                        dumptree(cur, indent+1);
        }

}
#endif

xmlNodePtr
xml_find_child(xmlNodePtr parent, char *name)
{
	xmlNodePtr cur;

	for (cur = parent->xmlChildrenNode; cur; cur = cur->next) {
		if (strcmp((char *)cur->name, name) == 0)
			return cur;
	}

	return NULL;
}

void
handle_acllist(const xmlDocPtr doc, xmlNodePtr acls, set *acl_allow, set *acl_deny)
{
	xmlNodePtr cur;

	/* Find owner */
	if ((cur = xml_find_child(acls, "owner"))) {
		if ((cur = xml_find_child(cur, "sid"))) {
			if ((cur = xml_find_child(cur, "string_sid"))) {
				char *sid = (char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				printf("Found owner: %s\n", sid);
				set_add(acl_allow, sid);
			}
		}
	}
}

void
handle_response(const xmlDocPtr doc, xmlNodePtr response, struct crawlinfo *ci, char *parent, set *acl_allow, set *acl_deny)
{
	xmlNodePtr href, propstat, cur;
	char *url;
	size_t hreflen;
	set *acl_allow2, *acl_deny2;
	char *newxml;

	if (!ci->documentContinue(ci->collection))
		return;

	if (!(href = xml_find_child(response, "href"))) {
		printf("Unable to find href...\n");
		return;
	}
	if (!(propstat = xml_find_child(response, "propstat"))) {
		printf("Unable to find propstat...\n");
		return;
	}

	url = (char *)xmlNodeListGetString(doc, href->xmlChildrenNode, 1);
	if (strcmp(url, parent) == 0) {
		/* Update acl lists */
		if ((cur = xml_find_child(propstat, "prop"))) {
			if ((cur = xml_find_child(cur, "descriptor"))) {
				if ((cur = xml_find_child(cur, "security_descriptor"))) {
					handle_acllist(doc, cur, acl_allow, acl_deny);
				}
			}
		}
		printf("Found parent, skiping...\n");
		free(url);
		return;
	}
	hreflen = strlen(url);

	acl_allow2 = set_clone(acl_allow);
	acl_deny2 = set_clone(acl_deny);

	/* XXX: Getting people that do not have access to the mailbox */
#if 0
	/* Update acl lists */
	if ((cur = xml_find_child(propstat, "prop"))) {
		if ((cur = xml_find_child(cur, "descriptor"))) {
			if ((cur = xml_find_child(cur, "security_descriptor"))) {
				handle_acllist(doc, cur, acl_allow2, acl_deny2);
			}
		}
	}
#endif

	/* Directory perhaps? */
	printf("Pathname: %s\n", url);
	if (url[hreflen-1] == '/') {
		newxml = ex_getContent(url, ci->collection->user, ci->collection->password);
		grabContent(newxml, (char *)url, ci, acl_allow2, acl_deny2);
		free(newxml);
	} else {
		char *sid = NULL;
		time_t lastmodified = 0;
		size_t contentlen = 0;
		xmlNodePtr props;

		if ((props = xml_find_child(propstat, "prop"))) {
			if ((cur = xml_find_child(props, "x1A001E"))) { /* item type */
				char *type;

				type = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				/*
				 * In exchange 2003 an IPM.Note seems to be a mail, and a IPM.Stickynote a note.
				 *
				 * - eirik
				 */
				if (strcmp(type, "IPM.Note") == 0) {
					free(type);
				} else {
					fprintf(stderr, "Found item of type: '%s', not grabing\n", type);
					free(type);
					goto err1;
				}
			}
			if ((cur = xml_find_child(props, "getlastmodified"))) {
				char *str;

				str = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				lastmodified = ex_parsetime(str);
				free(str);
			}
			if ((cur = xml_find_child(props, "getcontentlength"))) {
				char *str;

				str = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				contentlen = atoi(str);
				free(str);
			}
			if ((cur = xml_find_child(props, "xfff0102"))) { /* entryid */
				sid = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			}
		}

		if (sid == NULL || lastmodified == 0) {
			printf("Missing email information, skiping.\n");
		} else {
			grab_email(ci, acl_allow2, acl_deny2, url, sid, contentlen, lastmodified);
		}
		free(sid);
	}

 err1:
	set_free_all(acl_allow2);
	free(acl_allow2);
	set_free_all(acl_deny2);
	free(acl_deny2);
	free(url);
}

int
getEmailUrls(const char *data, struct crawlinfo *ci, char *parent, set *acl_allow, set *acl_deny)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	//printf("aaaaa \n########################%s\n########################\n",data);
	doc = xmlParseMemory(data, strlen(data));
	if (!doc) {
		fprintf(stderr, "Parse error!\n");
		return 0;
	}
	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		fprintf(stderr, "Empty document\n");
		xmlFreeDoc(doc);
		return 0;
	}
	//printf("Root node: %s\n", cur->name);
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (xmlStrcmp(cur->name, (const xmlChar *)"response") == 0) {
			handle_response(doc, cur, ci, parent, acl_allow, acl_deny);
		}
	}
	xmlFreeDoc(doc);
	return 1;
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

