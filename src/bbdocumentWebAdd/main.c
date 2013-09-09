#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <libconfig.h>
#include <mysql.h>
#include <ctype.h>
#include <time.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "../base64/base64.h"
#include "../bbdocument/bbdocument.h"
#include "../boitho-bbdn/bbdnclient.h"
#include "../cgi-util/cgi-util.h"
#include "../maincfg/maincfg.h"
#include "../crawlManager2/client.h"
#include "../common/define.h"
#include "../key/key.h"
#include "../common/error.h"

char systemkey[KEY_STR_LEN];


#define _GNU_SOURCE
#include "../cgi-util/cgi-util.h"


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
xml_add_one(int sock, xmlDocPtr doc, xmlNodePtr top)
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
	#ifdef DEBUG
		fprintf(stderr, "Going to add something! %s\n", top->name);
	#endif

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

		#ifdef DEBUG
			fprintf(stderr, "We are %d long\n", xmldoc.bodysize);
		#endif

		free(xmldoc.body);
		xmldoc.body = body;
	}

	//fprintf(stderr, "Adding: %s\n", xmldoc.body);

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
xml_add(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;

	if ((top = xml_find_child(top, "documents")) == NULL) {
		return;
	}
	for (n = top->xmlChildrenNode; n != NULL; n = n->next) {
		if (xmlStrcmp(n->name, (xmlChar*)"document") == 0)
			xml_add_one(sock, doc, n);
	}
}

void
xml_delete(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;
	xmlChar *coll;

	fprintf(stderr, "Going to delete something!\n");

	if ((n = xml_find_child(top, "collection")) == NULL) { 
		fprintf(stderr, "Unable to find collection to delete from\n");
		return;
	}

	if ((coll = xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL) {
		fprintf(stderr, "Unable to get collection name from delete\n");
		return;
	}


	for (n = top->xmlChildrenNode; n != NULL; n = n->next) {
		xmlChar *p;

		if (xmlStrcmp(n->name, (xmlChar*)"uri") != 0) {
			fprintf(stderr, "Unknown node(delete) name: %s\n", (char *)n->name);
			continue;
		}
		if ((p = xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL) {
			fprintf(stderr, "Unable to get uri to delete.\n");
			continue;
		}

		bbdn_deleteuri(sock, (char *)coll, (char *)p);

		xmlFree(p);
	}

	xmlFree(coll);
}


void sd_close(int bbdnsock, char *coll) {

        char *query;
        MYSQL db;
	MYSQL_RES *mysqlres;
	MYSQL_ROW row;
	int systemcont;

	bbdn_closecollection(bbdnsock, coll);

        // creat it in the sql db
        if (mysql_init(&db) == NULL) {
	        fprintf(stderr, "Unable to init mysql.\n");
                return;
        }
        if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)) {
        	fprintf(stderr, "Unable to connect to database: %s\n", mysql_error(&db));
                return ;
        }

	// see if we have a user system
        asprintf(&query, "SELECT COUNT(*) FROM system");

        if(mysql_real_query(&db, query, strlen(query))){ /* Make query */
                fprintf(stderr, "%s", mysql_error(&db));
                fprintf(stderr, "MySQL Error: \"%s\".",mysql_error(&db));
                return;
        }
        mysqlres=mysql_store_result(&db); /* Download result from server */

        row = mysql_fetch_row(mysqlres);
        systemcont = (row == NULL) ? -1 : atoi(row[0]);

fprintf(stderr, "systemcont=%i\n",systemcont);
	free(query);
	

        // create the collections if it don't exist
        asprintf(&query, "INSERT IGNORE INTO shares VALUES (NULL,'',14,1,NULL,0,NOW(),0,'','','','','',NULL,'Pushing has started.','%s',NULL,NULL,NULL,NULL,'%s',NULL)", coll, systemcont == 0 ? "anonymous" : "acl" );

        if (mysql_real_query(&db, query, strlen(query))) {
        	fprintf(stderr, "Failed to insert row, Error: %s\n", mysql_error(&db));
                return;
        }

        // Update status
        asprintf(&query, "UPDATE shares set crawler_success=1,crawler_message=\"OK.\",last=now() WHERE collection_name=\"%s\"", coll);

        if (mysql_real_query(&db, query, strlen(query))) {
        	fprintf(stderr, "Failed to update row, Error: %s\n", mysql_error(&db));
                return;
        }

        free(query);
        mysql_close(&db);

}

int sd_deletecollection(int bbdnsock, char *coll) {


        char *query;
        MYSQL db;


        // delete in the db
        if (mysql_init(&db) == NULL) {
                fprintf(stderr, "Unable to init mysql.\n");
	        return;
        }

        if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)) {
        	fprintf(stderr, "Unable to connect to database: %s\n", mysql_error(&db));
                return ;
        }

        asprintf(&query, "UPDATE shares set crawler_success=1,crawler_message=\"Deleting.\",last=now() WHERE collection_name=\"%s\"", coll);

	if (mysql_real_query(&db, query, strlen(query))) {
        	fprintf(stderr, "Failed to update row, Error: %s\n", mysql_error(&db));
                return;
	}


	if (bbdn_deletecollection(bbdnsock, coll) != 1) {
		return 0;
	}


        asprintf(&query, "DELETE from shares WHERE collection_name=\"%s\"", coll);

        if (mysql_real_query(&db, query, strlen(query))) {
        	fprintf(stderr, "Failed to update row, Error: %s\n", mysql_error(&db));
                return;
        }


        free(query);
        mysql_close(&db);


	return 1;
}


void
xml_close(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;
	char *query;
	MYSQL db;

	#ifdef DEBUG
		fprintf(stderr, "Going to close something!\n");
	#endif

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

		sd_close(sock, (char *)p);


		xmlFree(p);
	}
}

void
xml_create(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr n;
	char *query;
	MYSQL db;

	#ifdef DEBUG
		fprintf(stderr, "Going to create something!\n");
	#endif

	for (n = top->xmlChildrenNode; n != NULL; n = n->next) {
		xmlChar *p;

		if (xmlStrcmp(n->name, (xmlChar*)"collection") != 0) {
			fprintf(stderr, "Unknown node(create) name: %s\n", (char *)n->name);
			continue;
		}
		if ((p = xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL) {
			fprintf(stderr, "Unable to get collection name to create.\n");
			continue;
		}


		// creat it in the sql db
	        if (mysql_init(&db) == NULL) {
	                fprintf(stderr, "Unable to init mysql.\n");
	                return;
	        }
	        if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)) {
	                fprintf(stderr, "Unable to connect to database: %s\n", mysql_error(&db));
	                return ;
	        }

		asprintf(&query, "INSERT IGNORE INTO shares VALUES (NULL,'',14,1,NULL,0,NOW(),0,'','','','','',NULL,'Pushing has started.','%s',NULL,NULL,NULL,NULL,'acl',NULL)", (char *)p);

		if (mysql_real_query(&db, query, strlen(query))) {
                      fprintf(stderr, "Failed to insert rows, Error: %s\n", mysql_error(&db));
                      continue;
  		}

		free(query);
		mysql_close(&db);


		xmlFree(p);
	}
}

void
xml_users_user(MYSQL *db, unsigned int usersystem, xmlDocPtr doc, xmlNodePtr top)
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
			#ifdef DEBUG
				fprintf(stderr, "Query: %s\n", query);
			#endif
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
xml_gcwhispers(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	xmlNodePtr cur, n;
	char *collection;
	whisper_t w;
	
	if ((n = xml_find_child(top, "collection")) == NULL) {
		fprintf(stderr, "Unable to find collection to add gcwhisper to\n");
		return;
	}
	if ((collection = (char *)xmlNodeListGetString(doc, n->xmlChildrenNode, 1)) == NULL) {
		warnx("Unable to find collection name");
		return ;
	}

	w = 0;
	for (cur = top->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (xmlStrcmp(cur->name, (xmlChar *)"whisper") == 0) {
			char *str, *p;

			str = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (str == NULL)
				continue;

			if (strcmp(str, "notold") == 0) {
				w |= GCWHISPER_NOTOLD;
			} else {
				warn("Unknown gcwhisper string: '%s'", str);
			}
			free(str);
		}
	}
	bbdn_addwhisper(sock, collection, w);
	free(collection);
}

void
xml_errormsg(int sock, xmlDocPtr doc, xmlNodePtr top)
{
	char *msg;
	char path[2048];
	char *home;
	FILE *fp;
	time_t now;
	char stime[128];

	if ((home =  getenv("BOITHOHOME")) == NULL) {
		warn("Unable to get BOITHOHOME");
		return;
	}
	if ((msg = (char *)xmlNodeListGetString(doc, top->xmlChildrenNode, 1)) == NULL) {
		warnx("Unable to find error message");
		return;
	}
	snprintf(path, sizeof(path), "%s/logs/bbdocumentWebAdd.log", home);
	if ((fp = fopen(path, "a")) == NULL) {
		warn("Unable to append to: %s: %s", path, msg);
		free(msg);
		return;
	}
	now = time(NULL);
	strlcpy(stime, ctime(&now));
	stime[strlen(stime)-1] = '\0';
	fprintf(fp, "%s: %s\n", stime, msg);
	fclose(fp);

	free(msg);
}


void
xml_users(xmlDocPtr doc, xmlNodePtr top)
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
	#ifdef DEBUG
	fprintf(stderr, "Got a usersystem: %d\n", usersystem);
	#endif

	for (cur = top->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (xmlStrcmp(cur->name, (xmlChar *)"user") == 0) {
			xml_users_user(&db, usersystem, doc, cur);
		} else if (xmlStrcmp(cur->name, (xmlChar *)"dropusers") == 0) {
			char query[1024];
			size_t querylen;

			querylen = snprintf(query, sizeof(query), "DELETE FROM foreignUserSystem WHERE usersystem = %d", usersystem);
			#ifdef DEBUG
			fprintf(stderr, "Deleting old usersystem rows: %s\n", query);
			#endif
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

int main(int argc, char **argv, char **envp)
{
	int postsize;
	char *xmldata, *data;
	xmlDocPtr doc;
        xmlNodePtr cur, anode;
	int bbdnsock;
	int bbdnport;
	char *status = NULL;
	struct config_t maincfg;


	/* Read in config file */
	maincfg = maincfgopen();
	bbdnport = maincfg_get_int(&maincfg, "BLDPORT");
	maincfgclose(&maincfg);
	key_get(systemkey);
	char *request_method;

	if (!bbdn_conect(&bbdnsock, "", bbdnport))
		cgi_error(500, bstrerror());


	request_method = strdup(getenv("REQUEST_METHOD"));
	// We will handel the post stuf our self. Set REQUEST_METHOD to GET so cgi-util ignores it.
	setenv("REQUEST_METHOD", "GET", 1);

	if (cgi_init() != CGIERR_NONE) {
		cgi_error(500, "Can't init cgi-util");		
	}


	/*
	 * Either called from command line, and then we want a file.
	 * or a http get/put
	 * Or we are handling a web request, and getting the data from stdin.
	 */
	if ((cgi_getentrystr("method") != NULL) && (strcmp(cgi_getentrystr("method"),"rest") == 0)) {

		char api[100], coll[100], url[512];
		char *requrle;

		if (getenv("REQUEST_URI") == NULL) {
			cgi_error(500, "Can't read REQUEST_URI");
		}

		requrle = strdup(getenv("REQUEST_URI"));
		unescape_url(requrle);
		sscanf(requrle,"/%[a-z]/%[a-zA-Z0-9_-]/%[^?]", api, coll, url);

		#ifdef DEBUG
			printf("api: \"%s\"\n",api);
			printf("coll: \"%s\"\n",coll);
			printf("url: \"%s\"\n",url);
			printf("request_method: \"%s\"\n",request_method);
			printf("reques url \"%s\"\n",getenv("REQUEST_URI"));
			printf("reques url unescaped \"%s\"\n",requrle);
		#endif

		free(requrle);

		if (strcmp(request_method,"POST") == 0 || strcmp(request_method,"ADDDELAYED") == 0 || strcmp(request_method,"PUT") == 0) {

			if (getenv("CONTENT_LENGTH") == NULL) {
				cgi_error(500, "Can't read CONTENT_LENGTH");
			}

			
	               // Get data length
	                postsize = atoi(getenv("CONTENT_LENGTH"));
	                data = malloc(postsize + 1);
			if (data == NULL) {
				cgi_error(500, "Can't allocate data.");
			}
	                // Read data
	                fread(data, 1, postsize, stdin);
	                data[postsize] = '\0';

			// add in to repo
	        	if (bbdn_docadd(bbdnsock, coll, url, "", data, postsize, 0, "Everyone", "", "", "", cgi_getentrystr("attributes")) != 1) {
				cgi_error(500, "Can't add document");
			}

			
			if (strcmp(request_method,"ADDDELAYED") != 0) {
	                	// close it
	                	sd_close(bbdnsock, coll);
			}

			asprintf(&status,"Added %s to %s\n",url,coll);

		}
		else if (strcmp(request_method,"DELETE") == 0) {

			if (url[0] == '\0') {

				if (sd_deletecollection(bbdnsock, coll) != 1) {
					cgi_error(500, "Can't delete collection");
				}

				asprintf(&status,"Deleted collection %s\n",coll);
			}
			else {
				if (bbdn_deleteuri(bbdnsock, coll, url) != 1) {
					cgi_error(500, "Can't delete document");
				}

				asprintf(&status,"Deleted url %s in %s\n",url,coll);
			}

		}
		else if (strcmp(request_method,"CLOSE") == 0) {
			sd_close(bbdnsock, coll);

			asprintf(&status,"Closed %s\n",coll);

		}
		else {
			cgi_error(500, "Unknown request method \"%s\"", request_method );
		}

		#ifdef DEBUG
			// Print the envirement so we can better see what is going on.
	                char** env;
	                for (env = envp; *env != 0; env++) {
	                    char* thisEnv = *env;
	                    fprintf(stderr, "%s\n", thisEnv);
	                }
		#endif


	}
	else if ((cgi_getentrystr("do") != NULL) && (strcmp(cgi_getentrystr("do"),"add") == 0)) {

		char *url, *coll;
		char *data;
		int datasize;
		int n;

		url = getenv("HTTP_X_FILENAME") ? getenv("HTTP_X_FILENAME") : cgi_getentrystr("url");
		coll = cgi_getentrystr("collection");

		if (url == NULL) {
			cgi_error(500, "No url specified. Either set http header HTTP_X_FILENAME or get parameter 'url'.\n");
		}
		if (coll == NULL) {
			cgi_error(500, "No collection specified\n");
		}


		char *tmpname;
		FILE *fh;
		asprintf(&tmpname,"/tmp/%s",url);
	
		fh = fopen(tmpname,"wb");
		if (fh == NULL) {
			cgi_error(500, "Can't open file %s",tmpname);
		}

		if ((data = malloc( atoi(getenv("CONTENT_LENGTH")) )) == NULL) {
			cgi_error(500, "Can't malloc data");
		}

		datasize = 0;
		while ((n = fread ((unsigned char *)(data + datasize),1,1024,stdin)) > 0) {
			datasize += n;
		}

		fwrite(data,1,datasize,fh);

		fclose(fh);
		free(tmpname);


		//        bbdn_docadd(bbdnsock, xmldoc.collection, uri, xmldoc.documenttype, xmldoc.body, xmldoc.bodysize,
		//            xmldoc.lastmodified, xmldoc.aclallow, xmldoc.acldeny, xmldoc.title, xmldoc.documentformat, xmldoc.attributes);
        	bbdn_docadd(bbdnsock, coll, url, "", data, datasize,
      		    0, "Everyone", "", "omp1", "", "");


		// close it
		sd_close(bbdnsock, coll);
	}
	else if ((cgi_getentrystr("do") != NULL) && (strcmp(cgi_getentrystr("do"),"delete") == 0)) {


		char *url, *coll;

		url = getenv("HTTP_X_FILENAME") ? getenv("HTTP_X_FILENAME") : cgi_getentrystr("url");
		coll = cgi_getentrystr("collection");

		if (url == NULL) {
			cgi_error(500, "No url specified. Either set http header HTTP_X_FILENAME or get parameter 'url'.\n");
		}
		if (coll == NULL) {
			cgi_error(500, "No collection specified\n");
		}

		bbdn_deleteuri(bbdnsock, coll, url);

		asprintf(&status,"%s deleted.\n", url);
	}
	else if (getenv("CONTENT_LENGTH") != NULL) {
		// Get data length
		postsize = atoi(getenv("CONTENT_LENGTH"));
		xmldata = malloc(postsize + 1);	
		// Read data
		fread(xmldata, 1, postsize, stdin);
		xmldata[postsize] = '\0';


		//fprintf(stderr, "Received %i bytes.\n", postsize);
		//fprintf(stderr, "Got document:\n%s\n", xmldata);

		//parsing xml
        	doc = xmlParseDoc((xmlChar*)xmldata);

        	if (doc == NULL)
		cgi_error(500, "Unable to parse document");

        	cur = xmlDocGetRootElement(doc);

        	if (cur == NULL) {
        	        xmlFreeDoc(doc);
        	        cgi_error(500, "empty document");
        	}

		// Some document checking
        	if (xmlStrcmp(cur->name, (const xmlChar *)ROOT_NODE_NAME)) {
        	        xmlFreeDoc(doc);
        	        cgi_error(500, "document of the wrong type, root node != %s, but %s\n", ROOT_NODE_NAME, cur->name);
        	}

		if ((anode = xml_find_child(cur, "key")) != NULL) {
			char *p;
		
			p = (char *)xmlNodeListGetString(doc, anode->xmlChildrenNode, 1);
			if (p == NULL)
				cgi_error(500, "No key data");

			if ((systemkey[0] != '\0') && (!key_equal(systemkey, p))) {
				cgi_error(500, "Keys does not match:  Got \"%s\" but wanted \"%s\"\n",p,systemkey);
			}
		} else {
			cgi_error(500, "Did not receive a key");
		}
		if ((anode = xml_find_child(cur, "version")) != NULL) {
			xmlChar *p;

			p = xmlNodeListGetString(doc, anode->xmlChildrenNode, 1);
			version = atoi((char*)p);

			xmlFree(p);
		} else {
			cgi_error(500, "Did not receive a version number");
		}

		for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
			if ((!xmlStrcmp(cur->name, (const xmlChar *) "key"))){
				// Ignore
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "version"))){
				// Ignore
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "add"))){
				xml_add(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "delete"))){
				xml_delete(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "close"))) {
				xml_close(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "create"))) {
				xml_create(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "users"))) {
				xml_users(doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "gcwhispers"))) {
				xml_gcwhispers(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (const xmlChar *) "error"))) {
				xml_errormsg(bbdnsock, doc, cur);
			} else if ((!xmlStrcmp(cur->name, (xmlChar*)"text"))) {
				//fprintf(stderr, "Got text: %s\n", xmlNodeListGetString(doc, cur, 1));
				// Ignore for now
			} else {
				warnx("Unknown xml node '%s'", cur->name);
			}
		}

	} else {
		cgi_error(500, "Didn't receive any command or data.");
	}

	if (status != NULL) {
		printf("Content-type: text/plain\n\n");
		printf(status);
	}
	else {
		cgi_error(500, "Reached end of program without status.");
	}

	return 0;
}

/******************************************/
/******************************************/
/******************************************/
/******************************************/
/******************************************/
/******************************************/

/*
int main(int argc, char **argv, char** envp) {


        printf("Content-type: text/html\n");
        printf("Pragma: no-cache\n\n");

        int bbdnsock;
        int bbdnport;
        struct config_t maincfg;



        maincfg = maincfgopen();
        bbdnport = maincfg_get_int(&maincfg, "BLDPORT");
        maincfgclose(&maincfg);
        key_get(systemkey);

	// Connect to backend
        if (!bbdn_conect(&bbdnsock, "", bbdnport)) {
                errx(1, "Unable to connect to document manager");
	}


	return EXIT_SUCCESS;
}
*/
