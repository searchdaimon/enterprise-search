#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/bstr.h"


void shortenurl(char *url,int urllen) {

  	char **Data;
  	int Count, TokCount;
	char newurl[201] = "";
	int added, suburllen;
	int i;
	char slash[2];
	int len;
	len = strlen(url);
	printf("inn url %s\n",url);

	//tar bort http:// først
	if (strncmp(url,"http://",7) == 0) {
		//strcpy(url,url +7);
		memmove(url, url +7, len);
	}

	//tar bort / sist
	len -= 7 ;

	if (url[len -1] == '/') {
		url[len -1] = '\0';
	}

	len -= 1;

	//hvis den er kort kan vi bare returnere
	if (len < 80) {
		return;
	}

  	if ((TokCount = split(url, "/", &Data)) > 1) {
		printf("seperator: / \n");
		strcpy(slash,"/");
	}
	else if ((TokCount = split(url, "\\", &Data)) > 1) {
		printf("seperator: \\ \n");
		strcpy(slash,"\\");
	}
	else {
		printf("can't split\n");
		return;
	}

	/*
	Count = 0;
  	while( (Data[Count] != NULL) ) {
    		printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
  		printf("\n");

		Count++;
  	}
	*/
	--TokCount; //split ser ut til å begynner på 1, ikke på 0 


  	printf("\tfound %d token(s):\n", TokCount);

  	Count = 0;

	added = 0;
	suburllen = 0;
	while( (Data[Count] != NULL) ) {
		//printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
		suburllen = strlen(Data[Count]);

		if ((added + suburllen) < 20) {
			strlcat(newurl,Data[Count],sizeof(newurl));
			strlcat(newurl,slash,sizeof(newurl));
			
		}
		else {
			break;
		}
		added += suburllen;
		++Count;
	}

	strlcat(newurl,"...",sizeof(newurl));

	//printf("rev:\n");
	Count = TokCount;
	added = 0;
	suburllen = 0;
	while( (Count > 0) ) {
		//printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);

		suburllen = strlen(Data[Count]);
		if ((added + suburllen) < 50) {
			printf("candidate %s\n",Data[Count]);
		}
		else {
			break;
		}

		added += suburllen;
		--Count;
	}

	//printf("TokCount %i, count %i\n",TokCount,Count);

	//printf("addint last part:\n");
	for (i=Count+1;i<TokCount+1;i++) {
		//printf("\t\t%d\t\"%s\"\n", i, Data[i]);

                strlcat(newurl,slash,sizeof(newurl));
		strlcat(newurl,Data[i],sizeof(newurl));

	}

	//printf("newurl %s\n",newurl);
	FreeSplitList(Data);
	
	strscpy(url,newurl,urllen);
}

/*
int main () {
	char *url = strdup("file://G:/fellesdata/2%20NETTVERK%20OG%20INFORMASJON/Profilh%C3%A5ndbok%20og%20logoer/Visittkort%20Outlook/Astrid.htm");

	shortenurl(&url,200);

	printf("url %s\n",url);
}
*/
