#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "../base64/base64.h"
#include "../bbdocument/bbdocument.h"

#define rootNodeName "BBDOCUMENT"

struct xmldocumentFormat {
	char *TITLE;
	char *COLLECTION;
	char *URI;
	char *BODY;
	char *DOCUMENTFORMAT;
	int BODY_SIZE;
	unsigned int LASTMODIFIED;
	char *ACL;
	char *DOCUMENTTYPE;
};

int main(int argc, char *argv[]) {

	printf("Content-type: text/html\n\n");

	#ifdef DEBUG
		printf("is in debug mode. Will dump data to log.\n");

	        FILE *fh;

	        if ((fh = fopen("/tmp/bbdocumentWebAdd.log","ab")) == NULL) {
	                perror("logfile");
	        }

	        if (dup2(fileno(fh),fileno(stdout)) == -1) {
	                perror("dup2");
	        }

	#endif

	int postsize;
	char *xmldata;
	struct xmldocumentFormat xmldocument;

	//libxml
	xmlDocPtr doc;
        xmlNodePtr cur;
	xmlNodePtr cur_sub;
	xmlChar *value;

	//inaliserer:
	xmldocument.TITLE = NULL;
	xmldocument.COLLECTION = NULL;
	xmldocument.URI = NULL;
	xmldocument.BODY = NULL;
	xmldocument.DOCUMENTTYPE = NULL;

	//enten kalles vi fra komandolinjen, og skal ha en fil inn, eller så kalles vi via web, og 
	//skal ha post dtaa
	if (argc == 2) {
		printf("reading file %s\n",argv[1]);
		FILE *fp;
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
	}
	else if (getenv("CONTENT_LENGTH") != NULL) {


		//post data
		postsize = atoi(getenv("CONTENT_LENGTH"));

		xmldata = malloc(postsize +1);	
	
		//post data leses fra stdin, og er CONTENT_LENGTH lang
		fread(xmldata,1,postsize,stdin);

	}
	else {

		printf("Dident receive any data.\n");

		exit(1);		
	}

	bbdocument_init();

	//#ifdef DEBUG
		FILE *fhtmp;
		fhtmp = fopen("/tmp/posttest2.txt","wb");
		fprintf(fhtmp,"size: %i\nxmldata: %s\n\n\n",postsize,xmldata);
		fclose(fhtmp);
	//#endif

	printf("receiveed %ib ok.\n",postsize);

	//parsing xml
	//doc = xmlParseFile("revenuepilot.xml");
        doc = xmlParseDoc((unsigned char *)xmldata);

        if (doc == NULL ) {
                fprintf(stderr,"Document not parsed successfully. \n");
                return;
        }

        cur = xmlDocGetRootElement(doc);

        if (cur == NULL) {
                fprintf(stderr,"empty document\n");
                xmlFreeDoc(doc);
                return;
        }

        if (xmlStrcmp(cur->name, (const xmlChar *) rootNodeName)) {
                fprintf(stderr,"document of the wrong type, root node != %s, but %s\n",rootNodeName,cur->name);
                xmlFreeDoc(doc);
                return;
        }

        cur = cur->xmlChildrenNode;

	while (cur != NULL) {


		//printf("node cur->name %s\n",cur->name);

		if ((!xmlStrcmp(cur->name, (const xmlChar *) "text"))){

		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "DOCUMENTFORMAT"))){
			xmldocument.DOCUMENTFORMAT = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "DOCUMENTTYPE"))){
			xmldocument.DOCUMENTTYPE = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "KEY"))){


                                

                }
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "DOCUMENT"))){
			cur_sub = cur->xmlChildrenNode;

			while (cur_sub != NULL) {

				if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "text"))) {
					//do noting
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "TITLE"))) {
					xmldocument.TITLE = (char *)xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "LASTMODIFIED"))) {
					xmldocument.LASTMODIFIED = strtoul((char *)xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1), (char **)NULL, 10);
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "COLLECTION"))) {
					xmldocument.COLLECTION = (char *)xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "URI"))) {
					xmldocument.URI = (char *)xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "ACL"))) {
					xmldocument.ACL = (char *)xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
				}
				else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) "BODY_BASE64"))) {
					
					value = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);

					//printf("base64 vas \n\"%s\"\n\n",value);

					int sourcelen = strlen((char *)value);

					
					int maxlen = sourcelen * 3 / 4;
					xmldocument.BODY = (char*) malloc(maxlen);

					xmldocument.BODY_SIZE = base64_decode(xmldocument.BODY,(char *)value,maxlen);
					if (xmldocument.BODY_SIZE<0) {
						fprintf(stderr,"cant't bas64 decode. Error code %i\n",xmldocument.BODY_SIZE);
					}
					//printf("resultat %i b:\"%s\"\n",n,xmldocument.BODY);

					xmlFree(value);
					
				}
				else {
					printf("unknown xml subnode \"%s\".\n",cur_sub->name);
				}

				cur_sub = cur_sub->next;

			}
		}
              	else {
			printf("unknown xml node \"%s\"\n",cur->name);
		}
		cur = cur->next;

	}

	//add to Boitho
	//int xmldocument_exist(char subname[],char documenturi[],unsigned int lastmodified);
        //opne og les filen. Sende den så inn med xmldocument_add();
        bbdocument_add(xmldocument.COLLECTION,xmldocument.URI,xmldocument.DOCUMENTFORMAT,xmldocument.BODY,
		xmldocument.BODY_SIZE,xmldocument.LASTMODIFIED,xmldocument.ACL,xmldocument.TITLE,xmldocument.DOCUMENTTYPE);



	xmlFree(xmldocument.TITLE);
	xmlFree(xmldocument.COLLECTION);
	xmlFree(xmldocument.URI);
	xmlFree(xmldocument.ACL);
	xmlFree(xmldocument.DOCUMENTFORMAT);
	xmlFreeDoc(doc);
	free(xmldocument.BODY);
}
