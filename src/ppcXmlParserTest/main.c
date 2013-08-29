
#include <string.h>
#include <stdio.h>
#include "../ppcXmlParser/ppcXml.h"

int main () {

	int i, showabal;

        struct ppcPagesFormat ppcPages[10];
        int nrOfppcPages = 0;
	
	struct queryNodeHederFormat queryNodeHeder;

	strcpy(queryNodeHeder.query,"chat");
	strcpy(queryNodeHeder.userip,"64.236.16.20");

	queryNodeHeder.AmazonAssociateTag[0] = '\0';
	queryNodeHeder.AmazonSubscriptionId[0] = '\0';

	getPpcAds("amazon",ppcPages,&nrOfppcPages,&queryNodeHeder);


        for (i=0;i<nrOfppcPages;i++) {

			printf("%i:\n",i);
                        
			printf("-%s-\n-%s-\n%s\n\n",ppcPages[i].url,ppcPages[i].title,ppcPages[i].description);

                ++showabal;
        }
	printf("nrOfppcPages %i\n",nrOfppcPages);
}

