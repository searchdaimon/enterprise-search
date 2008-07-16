#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <libconfig.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "../base64/base64.h"
#include "../bbdocument/bbdocument.h"
#include "../boitho-bbdn/bbdnclient.h"
#include "../cgi-util/cgi-util.h"
#include "../maincfg/maincfg.h"

#define ROOT_NODE_NAME "sddocument"

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
	//getxmlnodestr(attributes);

	xmldoc.attributes = strdup("hei=ho");


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
	free(xmldoc.title);
#undef getxmlnodestr
}


void
sd_add(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;

	printf("name: %s\n", top->name);
	if ((top = xml_find_child(top, "documents")) == NULL) {
		return;
	}
	printf("name: %s\n", top->name);
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

		if (xmlStrcmp(n->name, (xmlChar*)"collection") != 0)
			continue;
		if ((p = xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL)
			continue;

		bbdn_closecollection(sock, (char *)p);

		xmlFree(p);
	}
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
		printf("Content-type: text/xml\n\n");
		// Get data length
		postsize = atoi(getenv("CONTENT_LENGTH"));
		xmldata = malloc(postsize + 1);	
		// Read data
		fread(xmldata, 1, postsize, stdin);
	} else {
		errx(1, "Didn't receive any data.");
	}

#ifdef DEBUG
	FILE *fhtmp;
	fhtmp = fopen("/tmp/posttest2.txt","wb");
	fprintf(fhtmp,"size: %i\nxmldata: %s\n\n\n",postsize,xmldata);
	fclose(fhtmp);
#endif

	fprintf(stderr, "Received %i bytes.\n", postsize);

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
		} else if ((!xmlStrcmp(cur->name, (xmlChar*)"text"))) {
			//fprintf(stderr, "Got text: %s\n", xmlNodeListGetString(doc, cur, 1));
			// Ignore for now
		} else {
			warnx("Unknown xml node '%s'", cur->name);
		}
	}

#if 0
	xmlFree(xmldocument.TITLE);
	xmlFree(xmldocument.COLLECTION);
	xmlFree(xmldocument.URI);
	xmlFree(xmldocument.ACL);
	xmlFree(xmldocument.DOCUMENTFORMAT);
	xmlFreeDoc(doc);
	free(xmldocument.BODY);
#endif

	return 0;
}
