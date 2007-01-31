#include "../common/define.h"

void findDomain(char url[], char domain[]);


int filterDocumentIndex(struct DocumentIndexFormat *DocumentIndex) {

	char domain[65]; //domane kan maks være på 64 bokstaver, + \0
	char *cpoint;
	int hyphenCount;

	printf("url: %s\n",(*DocumentIndex).Url);

	findDomain((*DocumentIndex).Url,domain);

	//teller antal forekomster av -
	hyphenCount = 0;
	cpoint = domain;
	while(cpoint = strchr(cpoint,'-')) {
		//går vidre
		++cpoint;
		++hyphenCount;
	}	

	printf("domain: %s\nhyphenCount %i\n",domain,hyphenCount);

}

/*******************************************************
Finner doemene for en url. Denne må bare brukes på velformede urler, 
som begynner på http://
********************************************************/
void findDomain(char url[], char domain[]) {

	char *cpoint;

	// +7 fjerner http://, som er de første 7 bokstavene i urlen
	strcpy(domain,url +7);
	printf("domain: %s\n",domain);
	//begynner med å fjerne http://
	cpoint = strchr(domain,'/');
	//vil ikke ha med / på slutten
	

	domain[cpoint - domain] = '\0';


}
