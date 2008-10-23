#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <libconfig.h>
#include <mysql.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "../base64/base64.h"
#include "../bbdocument/bbdocument.h"
#include "../boitho-bbdn/bbdnclient.h"
#include "../cgi-util/cgi-util.h"
#include "../maincfg/maincfg.h"
#include "../crawlManager/client.h"
#include "../common/define.h"

#define ROOT_NODE_NAME "sddocument"

/* MYSQL login information */
#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"

struct xmldocumentFormat {
	char *title;
	char *collection;
	char *uri;
	char *body;
	char *documentformat;
	char *documenttype;
	size_t bodysize;
	unsigned int lastmodified;
	char *aclallow;
	char *acldeny;
	char *attributes;
};

int version;

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
sd_add_one(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	struct xmldocumentFormat xmldoc;
	xmlNodePtr n;

#define getxmlnodestr(name) if ((n = xml_find_child(top, #name)) == NULL) { \
		fprintf(stderr, "Missing: %s\n", #name); \
		goto err; \
	} else { \
		xmldoc.name = (char*)xmlNodeListGetString(doc, n->xmlChildrenNode, 1); \
		if (xmldoc.name == NULL) {\
			xmldoc.name = strdup(""); \
			if (0) { \
				fprintf(stderr, "Couldn't fetch content: %s\n", #name); \
				goto err; \
			} \
		} \
	}

	memset(&xmldoc, '\0', sizeof(xmldoc));
	fprintf(stderr, "Going to add something! %s\n", top->name);

	getxmlnodestr(title);
	getxmlnodestr(uri);
	getxmlnodestr(documenttype);
	getxmlnodestr(documentformat);
	getxmlnodestr(collection);
	getxmlnodestr(aclallow);
	getxmlnodestr(acldeny);
	getxmlnodestr(attributes);

	//xmldoc.attributes = strdup("");


	if ((n = xml_find_child(top, "lastmodified")) == NULL) {
		fprintf(stderr, "Err 9\n");
		goto err;
	} else {
		char *p;
		p = (char*)xmlNodeListGetString(doc, n->xmlChildrenNode, 1);
		if (p == NULL) {
			fprintf(stderr, "Err 10\n");
			goto err;
		}
		xmldoc.lastmodified = atol(p);
	}

	if ((n = xml_find_child(top, "body")) == NULL) {
		fprintf(stderr, "Err 13\n");
		goto err;
	} else {
		char *p;
		char *body;
		xmlChar *encodetype;

		encodetype = xmlGetProp(n, (xmlChar*)"encoding");
		if (encodetype == NULL || xmlStrcmp(encodetype, (xmlChar*)"base64") != 0) {
			fprintf(stderr, "Err 16\n");
			goto err;
		}
		p = (char*)xmlNodeListGetString(doc, n->xmlChildrenNode, 1);
		if (p == NULL) {
			fprintf(stderr, "Err 17\n");
			goto err;
		}
		if ((xmldoc.body = malloc(strlen(p))) == NULL) {
			fprintf(stderr, "Err 18\n");
			goto err;
		}
		
		xmldoc.bodysize = base64_decode(xmldoc.body, p, strlen(p));
		body = malloc(xmldoc.bodysize+1);
		memcpy(body, xmldoc.body, xmldoc.bodysize);
		body[xmldoc.bodysize] = '\0';
		xmldoc.bodysize++;
		fprintf(stderr, "We are %d long\n", xmldoc.bodysize);
		free(xmldoc.body);
		xmldoc.body = body;
	}

	fprintf(stderr, "Adding: %s\n", xmldoc.body);

	bbdn_docadd(sock, xmldoc.collection, xmldoc.uri, xmldoc.documenttype, xmldoc.body, xmldoc.bodysize,
	    xmldoc.lastmodified, xmldoc.aclallow, xmldoc.acldeny, xmldoc.title, xmldoc.documentformat, xmldoc.attributes);

 err:
	free(xmldoc.title);
	free(xmldoc.collection);
	free(xmldoc.uri);
	free(xmldoc.documenttype);
	free(xmldoc.documentformat);
	free(xmldoc.body);
	free(xmldoc.aclallow);
	free(xmldoc.acldeny);
#undef getxmlnodestr
}


void
sd_add(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;

	if ((top = xml_find_child(top, "documents")) == NULL) {
		return;
	}
	for (n = top->xmlChildrenNode; n != NULL; n = n->next) {
		if (xmlStrcmp(n->name, (xmlChar*)"document") == 0)
			sd_add_one(sock, doc, n);
	}
}



void
sd_close(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;

	fprintf(stderr, "Going to close something!\n");

	for (n = top->xmlChildrenNode; n != NULL; n = n->next) {
		xmlChar *p;

		if (xmlStrcmp(n->name, (xmlChar*)"collection") != 0) {
			fprintf(stderr, "Unknown node(close) name: %s\n", (char *)n->name);
			continue;
		}
		if ((p = xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL) {
			fprintf(stderr, "Unable to get collection name to close.\n");
			continue;
		}

		bbdn_closecollection(sock, (char *)p);

		xmlFree(p);
	}
}

void
sd_users_user(MYSQL *db, unsigned int usersystem, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr cur;
	char *username;
	char user[512];
	
	username = (char *)xmlGetProp(top, (xmlChar *)"username");

	mysql_real_escape_string(db, user, username, strlen(username));
	free(username);

	for (cur = top->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (xmlStrcmp(cur->name, (xmlChar *)"group") == 0) {
			char *groupname;
			char query[1024];
			char group[512];
			size_t querylen;

			groupname = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			mysql_real_escape_string(db, group, groupname, strlen(groupname));
			free(groupname);
			querylen = snprintf(query, sizeof(query), "INSERT INTO foreignUserSystem (usersystem, username, groupname) "
			    "VALUES(%d, '%s', '%s')", usersystem, user, group);
			fprintf(stderr, "Query: %s\n", query);
			if (mysql_real_query(db, query, querylen)) {
				fprintf(stderr, "Failed to insert row, Error: %s\n", mysql_error(db));
				continue;
			}

		} else {
			// What???
		}
	}
}

void
sd_users(xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr cur;
	MYSQL db;
	unsigned int usersystem;

	if (mysql_init(&db) == NULL) {
		fprintf(stderr, "Unable to init mysql.\n");
		return;
	}
	if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)) {
		fprintf(stderr, "Unable to connect to database: %s\n", mysql_error(&db));
		return ;
	}

	// Get usersystem id
	usersystem = atoi((char *)xmlGetProp(top, (xmlChar *)"usersystem"));
	fprintf(stderr, "Got a usersystem: %d\n", usersystem);

	for (cur = top->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (xmlStrcmp(cur->name, (xmlChar *)"user") == 0) {
			sd_users_user(&db, usersystem, doc, cur);
		} else if (xmlStrcmp(cur->name, (xmlChar *)"dropusers") == 0) {
			char query[1024];
			size_t querylen;

			querylen = snprintf(query, sizeof(query), "DELETE FROM foreignUserSystem WHERE usersystem = %d", usersystem);
			fprintf(stderr, "Deleting old usersystem rows: %s\n", query);
			if (mysql_real_query(&db, query, querylen)) {
				fprintf(stderr, "Failed to remove rows, Error: %s\n", mysql_error(&db));
				continue;
			}
		} else {
			fprintf(stderr, "Unknown node in users: %s\n", (char *)cur->name);
			continue;
		}
	}

	mysql_close(&db);
}

int
main(int argc, char **argv)
{
	int postsize;
	char *xmldata;
	char *keystr;
	xmlDocPtr doc;
        xmlNodePtr cur, anode;
	int bbdnsock;
	int bbdnport;
	struct config_t maincfg;

	printf("Content-type: text/txt\n\n");
	/* Read in config file */
	maincfg = maincfgopen();
	bbdnport = maincfg_get_int(&maincfg, "BLDPORT");
	maincfgclose(&maincfg);

	if (!bbdn_conect(&bbdnsock, "", bbdnport))
		errx(1, "Unable to connect to document manager");

	/*
	 * Either called from command line, and then we want a file.
	 * Or we are handling a web request, and getting the data from stdin.
	 */
	if (argc == 2) {
		FILE *fp;

		fprintf(stderr, "reading file %s\n",argv[1]);
		if ((fp = fopen(argv[1],"rb")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		struct stat inode;      // lager en struktur for fstat å returnere.
		fstat(fileno(fp),&inode);		

		postsize = inode.st_size;

		xmldata = malloc(postsize +1);
		fread(xmldata,1,postsize,fp);

		fclose(fp);
	} else if (getenv("CONTENT_LENGTH") != NULL) {
		// Get data length
		postsize = atoi(getenv("CONTENT_LENGTH"));
		xmldata = malloc(postsize + 1);	
		// Read data
		fread(xmldata, 1, postsize, stdin);
		xmldata[postsize] = '\0';
	} else {
		errx(1, "Didn't receive any data.");
	}

	//fprintf(stderr, "Received %i bytes.\n", postsize);
	//fprintf(stderr, "Got document:\n%s\n", xmldata);

	//parsing xml
        doc = xmlParseDoc((xmlChar*)xmldata);

        if (doc == NULL)
		errx(1, "Unable to parse document");

        cur = xmlDocGetRootElement(doc);

        if (cur == NULL) {
                xmlFreeDoc(doc);
                errx(1, "empty document");
        }

	// Some document checking
        if (xmlStrcmp(cur->name, (const xmlChar *)ROOT_NODE_NAME)) {
                xmlFreeDoc(doc);
                errx(1, "document of the wrong type, root node != %s, but %s\n", ROOT_NODE_NAME, cur->name);
        }

	if ((anode = xml_find_child(cur, "key")) != NULL) {
		fprintf(stderr, "Got a key\n");
	} else {
		errx(1, "Did not receive a key");
	}
	if ((anode = xml_find_child(cur, "version")) != NULL) {
		xmlChar *p;

		p = xmlNodeListGetString(doc, anode->xmlChildrenNode, 1);
		version = atoi((char*)p);
		fprintf(stderr, "Got a version: %d\n", version);

		xmlFree(p);
	} else {
		errx(1, "Did not receive a version number");
	}

	for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "key"))){
			// Ignore
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "version"))){
			// Ignore
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "add"))){
			sd_add(bbdnsock, doc, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "close"))) {
			sd_close(bbdnsock, doc, cur);
		} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "users"))) {
			sd_users(doc, cur);
		} else if ((!xmlStrcmp(cur->name, (xmlChar*)"text"))) {
			//fprintf(stderr, "Got text: %s\n", xmlNodeListGetString(doc, cur, 1));
			// Ignore for now
		} else {
			warnx("Unknown xml node '%s'", cur->name);
		}
	}

	return 0;
}
