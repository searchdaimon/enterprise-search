#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "ppcXml.h"



//compile: -I/usr/include/libxml2 -L/usr/lib -lxml2 -lz -lm
void parsListnings(xmlNodePtr cur,struct ppcPagesFormat *ppcPages, int *nrOfPpcPages,char rootNodeName[],
char listningNodeName[],char titleNodeName[],char descriptionNodeName[], char uriNodeName[],
char bidNodeName[], char urlNodeName[]) {

	xmlNodePtr cur_sub;
	xmlDocPtr doc;
        xmlChar *key;
	int i;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) listningNodeName))){


	        	cur_sub = cur->xmlChildrenNode;
        		while (cur_sub != NULL) {
	

            			if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) titleNodeName))) {
                    			key = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
					cleanString((char *)key);
					#ifdef DEBUG
                    			printf("titleNodeName: %s\n", (char *)key);
					#endif

					strncpy(ppcPages[(*nrOfPpcPages)].title,(char *)key,sizeof(ppcPages[(*nrOfPpcPages)].title) -1);

                    			xmlFree(key);
            			}
            			else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) descriptionNodeName))) {
                    			key = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
					cleanString((char *)key);

					#ifdef DEBUG
                    			printf("descriptionNodeName: %s\n", (char *)key);
					#endif

					strncpy(ppcPages[(*nrOfPpcPages)].description,(char *)key,sizeof(ppcPages[(*nrOfPpcPages)].description) -1);

                    			xmlFree(key);
            			}
            			else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) uriNodeName))) {
                    			key = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
					cleanString((char *)key);

                    			//printf("uriNodeName: %s\n", (char *)key);
					
					strncpy(ppcPages[(*nrOfPpcPages)].uri,(char *)key,sizeof(ppcPages[(*nrOfPpcPages)].uri) -1);

                    			xmlFree(key);
            			}
            			else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) urlNodeName))) {
                    			key = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
					cleanString((char *)key);

					#ifdef DEBUG
                    			printf("urlNodeName: %s\n", key);
					#endif

					strncpy(ppcPages[(*nrOfPpcPages)].url,(char *)key,sizeof(ppcPages[(*nrOfPpcPages)].url) -1);


                    			xmlFree(key);
            			}
            			else if ((!xmlStrcmp(cur_sub->name, (const xmlChar *) bidNodeName))) {
                    			key = xmlNodeListGetString(doc, cur_sub->xmlChildrenNode, 1);
					cleanString((char *)key);

					#ifdef DEBUG
                    			printf("bidNodeName: %s\n", (char *)key);
					#endif
					ppcPages[(*nrOfPpcPages)].bid = atof((char *)key);

                    			xmlFree(key);
            			}
            			else {
					#ifdef DEBUG
                    			fprintf(stderr,"wrong type %s\n",cur_sub->name);
					#endif
            			}



        			cur_sub = cur_sub->next;
        		}

				++(*nrOfPpcPages);				

		}
		else {
			#ifdef DEBUG
				printf("not listningNodeName, but \"%s\"\n",cur->name);
			#endif
		}

		 
		cur = cur->next;
	}

}

void parsPpcAds(char *docname,struct ppcPagesFormat *ppcPages, int *nrOfPpcPages,char rootNodeName[],
char listningNodeName[], char listningSubNodeName[],char titleNodeName[],char descriptionNodeName[], char uriNodeName[], 
char bidNodeName[], char urlNodeName[]) {

	xmlDocPtr doc;
	xmlNodePtr cur;

	xmlNodePtr cur_sub;
        xmlChar *key;


	//doc = xmlParseFile("revenuepilot.xml");
	doc = xmlParseDoc((unsigned char *)docname);
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

	//vi kan enten bare ha to løv i treet, eller tre. Hvis vi har tre må vi ha navnet i listningSubNodeName, 
	//hvis ikke skal dette være NULL
	if (listningSubNodeName == NULL) {
		parsListnings(cur,ppcPages, nrOfPpcPages,rootNodeName, listningNodeName, titleNodeName, descriptionNodeName, uriNodeName, bidNodeName, urlNodeName);
	}
	else {
	        while (cur != NULL) {
                	if ((!xmlStrcmp(cur->name, (const xmlChar *) listningSubNodeName))){

                        	cur_sub = cur->xmlChildrenNode;
				
				parsListnings(cur_sub,ppcPages, nrOfPpcPages,rootNodeName, listningNodeName, titleNodeName, descriptionNodeName, uriNodeName, bidNodeName, urlNodeName);
				
			}
			else {
				#ifdef DEBUG
                                	printf("not listningNodeName, but \"%s\"\n",cur->name);
                        	#endif
			}
			cur = cur->next;
		}
	}
	xmlFreeDoc(doc);
	return;
}

/*
int
main(int argc, char **argv) {

	char *docname;
	int i;
	
	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
		return(0);
	}

	struct ppcPagesFormat ppcPages[10];
	
	int nrOfppcPages = 0;

	docname = argv[1];
	getPpcAds (docname,ppcPages,&nrOfppcPages,"RESULTS","LISTING","TITLE","DESCRIPTION","LINK","BID","DOMAIN");

	printf("from ppcPages\n");
	for (i=0;i<nrOfppcPages;i++) {
		printf("i %i\n",i);
		printf("title %s\nbid %f\n",ppcPages[i].title,ppcPages[i].bid);

		//printf("uri: %s\n",ppcPages[i].uri);

		printf("uri len %i\n",strlen(ppcPages[i].uri));
		printf("url %s\n",ppcPages[i].url);
	}

	
}

*/
