#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "ppcXml.h"

#include "../common/bstr.h"

//compile: -I/usr/include/libxml2 -L/usr/lib -lxml2 -lz -lm
void parsAmazonListnings(char path[], char value[],struct ppcPagesFormat *ppcPages, 
int *nrOfPpcPages,struct nodenamesFormat *NodeName) {

	#ifdef DEBUG
	printf("path %s, value %s\n",path,value);
	#endif

	if (strcmp(path,(*NodeName).startNodeName) == 0) {
		#ifdef DEBUG
			printf("got start\n");
		#endif
		//temp: dorlig begrensing her
		if ((*nrOfPpcPages) < 9) {
			++(*nrOfPpcPages);
		}
	}


	if (strcmp(path,(*NodeName).titleNodeName) == 0) {
		#ifdef DEBUG
			printf("title for %i: %s\n",(*nrOfPpcPages),value);
		#endif

		strscpy(ppcPages[(*nrOfPpcPages) -1].title,value,sizeof((*ppcPages).title));
	}	
	else if (strcmp(path,(*NodeName).urlNodeName) == 0){
		#ifdef DEBUG
			printf("url for %i: %s\nlen: %i\n",(*nrOfPpcPages),value,strlen(value));
		#endif

		strscpy(ppcPages[(*nrOfPpcPages) -1].url,value,sizeof((*ppcPages).url));

	}
	else if (strcmp(path,(*NodeName).descriptionNodeName) == 0){
		#ifdef DEBUG
			printf("description for %i: %s\n",(*nrOfPpcPages),value);
		#endif
		//bare hvis vi ikke har noe fra før. Skal bare ta første element
		if (ppcPages[(*nrOfPpcPages) -1].description[0] == '\0') {
			strscpy(ppcPages[(*nrOfPpcPages) -1].description,value,sizeof((*ppcPages).description));
		}
	}
	else if (strcmp(path,(*NodeName).thumbnailNodeName) == 0){
		#ifdef DEBUG
			printf("thumbnail for %i: %s\n",(*nrOfPpcPages),value);
		#endif

		strscpy(ppcPages[(*nrOfPpcPages) -1].thumbnail,value,sizeof((*ppcPages).thumbnail));
		
	}
	else if (strcmp(path,(*NodeName).thumbnailwidthNodeName) == 0){
		#ifdef DEBUG
			printf("thumbnailwidth for %i: %s\n",(*nrOfPpcPages),value);
		#endif

		strscpy(ppcPages[(*nrOfPpcPages) -1].thumbnailwidth,value,sizeof((*ppcPages).thumbnailwidth));
		
	}
	else if (strcmp(path,(*NodeName).thumbnailheightNodeName) == 0){
		#ifdef DEBUG
			printf("thumbnailheight for %i: %s\n",(*nrOfPpcPages),value);
		#endif

		strscpy(ppcPages[(*nrOfPpcPages) -1].thumbnailheight,value,sizeof((*ppcPages).thumbnailheight));
		
		
	}

}

int recursiveparser(xmlNodePtr *cur_r, char path[],struct ppcPagesFormat *ppcPages, int *nrOfPpcPages,struct nodenamesFormat *NodeName) {

	xmlNodePtr cur_sub;
	char newpath[128];
	xmlChar *key;
	xmlDocPtr doc;

	while ((*cur_r) != NULL) {
		
		sprintf(newpath,"%s->%s",path,(*cur_r)->name);
		

		if ((*cur_r)->xmlChildrenNode == NULL) {
			#ifdef DEBUG
                        printf("leaf. %s\n",path);
			#endif

			if ((key = xmlNodeListGetString(doc, (*cur_r), 1))  != NULL) {

				parsAmazonListnings(path,(char *)key,ppcPages,nrOfPpcPages,NodeName);
				
				#ifdef DEBUG
				printf("key %s\n",key);
				#endif
				xmlFree(key);
			}
                }
		else {
			recursiveparser(&(*cur_r)->xmlChildrenNode,newpath,ppcPages, nrOfPpcPages,NodeName);
		}	
		#ifdef DEBUG
		printf("bb %s\n",(*cur_r)->name);
		#endif
		(*cur_r) = (*cur_r)->next;
	}
	return 0;


}

void parsPpcAmazon(char *docname,struct ppcPagesFormat *ppcPages, int *nrOfPpcPages, struct nodenamesFormat *NodeName) {

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
	
	if (xmlStrcmp(cur->name, (const xmlChar *) (*NodeName).rootNodeName)) {
		fprintf(stderr,"document of the wrong type, root node != %s, but %s\n",(*NodeName).rootNodeName,cur->name);
		xmlFreeDoc(doc);
		return;
	}
	
	cur = cur->xmlChildrenNode;

	
	recursiveparser(&cur,(char *)cur->name,ppcPages, nrOfPpcPages, NodeName);

	return;
}

