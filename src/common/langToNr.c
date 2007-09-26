#include <stdio.h>
#include <string.h>
#include <stdlib.h>

	char *languages[] = { "ENG", "NOR", "DAN", "DEU", "FIN", "FRA", "ITA", "NLD", "POR", "RUS", "SWE" };
	int nrOfLanguage = (sizeof(languages) / sizeof(char *));


int getLangNr(char testlang[]) {


	int i =0;

	while ( (i<nrOfLanguage) && (strcmp(testlang,languages[i]) != 0) ) {
	
		++i;
	}
	++i; //0 er ikke funnet, 1 er det vi skal retunere
	//printf("i %i,nrOfLanguage %i. testlang %s\n",i,nrOfLanguage,testlang);

	//exit(1);

	if (i == nrOfLanguage) {
		return 0;
	}
	else {
		return i;
	}
	
}

void getLangCode(char langcode[],int langnr) {

	if (langnr == 0) {
		//fprintf(stderr,"Got langnr 0\n");
		langcode[0] = '\0';
	}
	else if (langnr > nrOfLanguage) {
		//fprintf(stderr,"langnr %i larger then max (%i)",langnr,nrOfLanguage);
		langcode[0] = '\0';
	}
	else {
		strcpy(langcode,languages[langnr -1]);
	}
}

char *getLangCode2(int langnr) {

	if (langnr == 0) {
		//fprintf(stderr,"Got langnr 0\n");
		//langcode[0] = '\0';
		return NULL;
	}
	else if (langnr > nrOfLanguage) {
		//fprintf(stderr,"langnr %i larger then max (%i)",langnr,nrOfLanguage);
		//langcode[0] = '\0';
		return NULL;

	}
	else {
		//strcpy(langcode,languages[langnr -1]);
		return languages[langnr -1];
	}
}

