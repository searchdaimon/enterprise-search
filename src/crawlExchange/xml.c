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

#define MASK_READ 0x00120089
#define MASK_DENIED_READ 0x00020089

/*
 * XXX:
 * Will most probably segfault on a malformed xml structure
 */

time_t
ex_parsetime(char *time)
{
	struct tm tm;

	bzero(&tm, sizeof(tm));
	printf("ex_parsetime(time=\"%s\")\n",time);

	/* XXX: Does not handle time zones */
	if (strptime(time, "%Y-%m-%dT%H:%M:%S.", &tm) == NULL)
		warn("strptime");

	return mktime(&tm);
}

#if 1
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
handle_acllist_dacl_2(const xmlDocPtr doc, xmlNodePtr acls, set *acl_allow, set *acl_deny, int rev)
{
	xmlNodePtr cur, child, mask, sid;

	if ((cur = xml_find_child(acls, "effective_aces"))) {
		for (child = cur->xmlChildrenNode; child; child = child->next) {
			set *acl;
			char *sidstr, *maskstr, *nt4name;
			unsigned int masknum;

			if (strcmp((char *)child->name, "access_allowed_ace") == 0) {
				acl = acl_allow;
			} else if (strcmp((char *)child->name, "access_denied_ace") == 0) {
				acl = acl_deny;
			} else {
				//warnx("unknown entry a_a_[ad]: %s", (char *)child->name);
				continue;
			}
			
			if (!(mask = xml_find_child(child, "access_mask"))) {
				warnx("no access mask");
				continue;
			}
			if ((sid = xml_find_child(child, "sid"))) {
				if (!(sid = xml_find_child(sid, "string_sid"))) {
					warnx("No string sid");
					continue;
				}
			} else {
				warnx("no sid");
				continue;
			}
			sidstr = (char *)xmlNodeListGetString(doc, sid->xmlChildrenNode, 1);
			maskstr = (char *)xmlNodeListGetString(doc, mask->xmlChildrenNode, 1);
			masknum = strtol(maskstr, NULL, 16);
			free(maskstr);

			//printf("Sid: %s\nMask: %x\n", sidstr, masknum);

			if (acl == acl_allow && (MASK_READ & masknum) == MASK_READ) {
				//warnx("Adding to allow list: %s\n", sidstr);
				set_add(acl, sidstr);
			} else if (acl == acl_deny && (MASK_DENIED_READ & masknum) == MASK_DENIED_READ) {
				//warnx("Adding to deny list: %s\n", sidstr);
				set_add(acl, sidstr);
			} else {
				//printf("something else... %X\n", masknum);
				free(sidstr);
			}
		}
	} else {
		warnx("No effective aces");
	}
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

	/* dacl */
	if ((cur = xml_find_child(acls, "dacl"))) {
		xmlNodePtr rev;
		int revision;


		#ifdef DEBUG
		printf("dacl tree\n");
		dumptree(cur, 0);
		#endif

		if ((rev = xml_find_child(cur, "revision"))) {
			char *r = (char*)xmlNodeListGetString(doc, rev->xmlChildrenNode, 1);
			revision = atoi(r);
			free(r);
			printf("We have a revision: %d\n", revision);
		} else {
			warnx("Unable to find dacl revision");
			return;
		}

		if (revision == 2) {
			handle_acllist_dacl_2(doc, cur, acl_allow, acl_deny, revision);
		} else {
			warnx("Unknown revision: %d", revision);
		}
	}
}

void
handle_response(const xmlDocPtr doc, xmlNodePtr response, struct crawlinfo *ci, char *parent, set *acl_allow, set *acl_deny, char *usersid, CURL **curl, struct loginInfoFormat *login)
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
		if ((newxml = ex_getContent(url, curl, login)) == NULL) {
			printf("can't ex_getContent() for url %s. skipping\n",url);
			goto err1;
		}
		grabContent(newxml, (char *)url, ci, acl_allow2, acl_deny2, usersid, curl, login);
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
				} 
				#ifdef WITH_PUBLIC_FOLDERS
				// Runarb: 12 des 2008:
				// ser ut til at mail i public fiolders konverteres til IPM.Post ... Tør ikke å bare grabbe
				// IPM.Post for alle mail, da det kansje kan være noe annet i vanlig Exchange?
				else if (strcmp(type, "IPM.Post") == 0) {
					free(type);
				}
				#endif 
				else {
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
			grab_email(ci, acl_allow2, acl_deny2, url, sid, contentlen, lastmodified, usersid, curl);
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
getEmailUrls(const char *data, struct crawlinfo *ci, char *parent, set *acl_allow, set *acl_deny, char *usersid, CURL **curl, struct loginInfoFormat *login)
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
			handle_response(doc, cur, ci, parent, acl_allow, acl_deny, usersid, curl, login);
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

