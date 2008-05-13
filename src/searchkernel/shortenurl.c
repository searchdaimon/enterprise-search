#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "verbose.h"

#include "../common/bstr.h"

#define TARGET_VISIBLE_URL_LEN 80

#ifdef WITH_SHORTENURL_MAIN
int globalOptVerbose = 0;
#endif

void
shortenurl_outlook(char *url, int urllen)
{
#if 0
	if (urllen > 43) {
		url[40] = '.';
		url[41] = '.';
		url[42] = '.';
		url[43] = '\0';
	}
#else
	strcpy(url, "Exchange");
#endif
}

void shortenurl(char *url,int urllen) {

  	char **Data;
  	int Count, TokCount;
	char newurl[201];
	int added, suburllen;
	int i;
	char slash[2];
	int len;
	vboprintf("shortenurl: inn url %s\n", url);
	char *p;
	char proto[128];
	char origurl[urllen+1];
	
	newurl[0] = '\0';
	proto[0] = '\0';

	//tar bort proto:// først
	p = strstr(url, "://");
	if (p != NULL && p > url) {
		p += 3; /* Skip past :// */
		strncpy(proto, url, p-url);
		proto[p-url] = '\0';
		while (*p == '/')
			p++;
		len = strlen(p);
		memmove(url, p, len);
		url[len] = '\0';
	} else if (strncmp(url, "outlook:", 8) == 0) {
		shortenurl_outlook(url, urllen);
		return;
	} else {
		len = strlen(url);
	}	
	strcpy(origurl, url);

	#ifdef DEBUG
		printf("shortenurl: after proto \"%s\"\n",url);
	#endif

	//hvis den er kort kan vi bare returnere
	if (len < TARGET_VISIBLE_URL_LEN) {
		#ifdef DEBUG
 			printf("shortenurl: url is short enough. Don't need to shorten\n");
		#endif

		snprintf(url, urllen, "%s%s", proto, origurl);
		return;
	}

  	if ((TokCount = split(url, "/", &Data)) > 1) {
		#ifdef DEBUG
		printf("seperator: / \n");
		#endif
		strcpy(slash,"/");
	}
	else if ((TokCount = split(url, "\\", &Data)) > 1) {
		#ifdef DEBUG
		printf("seperator: \\ \n");
		#endif
		strcpy(slash,"\\");
	}
	else {
		printf("can't split\n");
		snprintf(url, urllen, "%s%s", proto, origurl);
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

	#ifdef DEBUG
  	printf("\tfound %d token(s):\n", TokCount);
	#endif

  	Count = 0;

	added = 0;
	suburllen = 0;
	while( (Data[Count] != NULL) ) {
		printf("a: \t\t%d\t\"%s\"\n", Count, Data[Count]);
		suburllen = strlen(Data[Count]);

		if ((added + suburllen) < (TARGET_VISIBLE_URL_LEN * 0.3)) {
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
		printf("b: \t\t%d\t\"%s\"\n", Count, Data[Count]);

		suburllen = strlen(Data[Count]);
		if ((added + suburllen) < (TARGET_VISIBLE_URL_LEN * 0.7)) {
			#ifdef DEBUG
			printf("candidate %s\n",Data[Count]);
			#endif
		}
		else {
			break;
		}

		added += suburllen;
		--Count;
	}

	printf("TokCount %i, count %i\n",TokCount,Count);

	//hvis også siste navn er for langt, hånterer vi det spesifikt.
	if (TokCount == Count) {
		strlcat(newurl,slash,sizeof(newurl));
		strlcat(newurl,Data[Count],sizeof(newurl));		
	}
	else {
		//printf("addint last part:\n");
		for (i=Count+1;i<TokCount+1;i++) {
			printf("c: \t\t%d\t\"%s\"\n", i, Data[i]);

                	strlcat(newurl,slash,sizeof(newurl));
			strlcat(newurl,Data[i],sizeof(newurl));

		}
	}

	printf("shortenurl: newurl %s\n",newurl);

	FreeSplitList(Data);
	
	//strscpy(url,newurl,urllen);
	snprintf(url, urllen, "%s%s", proto, newurl);
}

#ifdef WITH_SHORTENURL_MAIN

int
main(int argc, char **argv)
{
	//char *url = strdup("file://G:/fellesdata/2%20NETTVERK%20OG%20INFORMASJON/Profilh%C3%A5ndbok%20og%20logoer/Visittkort%20Outlook/Astrid.htm");
	char *url = strdup("file://///192.168.22.25\\filer\\Ringnes\\2006\\Brus.Vann\\Pepsi 2006\\Pepsi Max Gold lansering\\Pepsi Max Gold Sommerturne - kjÃ¸replan.xls");

	shortenurl(url,255);

	printf("url %s\n",url);
}

#endif
