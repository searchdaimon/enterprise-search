#include "../common/define.h"

void findDomain(char url[], char domain[]);


int filterDocumentIndex(struct DocumentIndexFormat *DocumentIndex) {

	char domain[65]; //a domain can be at most 64 letters, + \0
	char *cpoint;
	int hyphenCount;

	printf("url: %s\n",(*DocumentIndex).Url);

	findDomain((*DocumentIndex).Url,domain);

	//count the number of occurrences of -
	hyphenCount = 0;
	cpoint = domain;
	while(cpoint = strchr(cpoint,'-')) {
		//continue
		++cpoint;
		++hyphenCount;
	}	

	printf("domain: %s\nhyphenCount %i\n",domain,hyphenCount);

}

/*******************************************************
Finds the domain for a url. This must only be used on well-formed urls
that start with http://
********************************************************/
void findDomain(char url[], char domain[]) {

	char *cpoint;

	// +7 removes http://, which is the first 7 letters of the url
	strcpy(domain,url +7);
	printf("domain: %s\n",domain);
	//start by removing http://
	cpoint = strchr(domain,'/');
	//we don't want the trailing /
	

	domain[cpoint - domain] = '\0';


}
