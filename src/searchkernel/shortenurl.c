#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "verbose.h"

#include "../common/bstr.h"
#include "../logger/logger.h"

#ifdef BLACK_BOX
	#define TARGET_VISIBLE_URL_LEN 80
#else
	#define TARGET_VISIBLE_URL_LEN 60
#endif

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
	#ifdef BLACK_BOX
		unsigned char newurl[128];
	#else
		unsigned char newurl[201];
	#endif

	int added, suburllen;
	int i;
	char slash[2];
	int len;
	char *p;
	char proto[128];
	char origurl[urllen+1];

	bblog(DEBUGINFO, "shortenurl: inn url %s", url);
	
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
		bblog(DEBUGINFO, "shortenurl: after proto \"%s\"", url);
	#endif

	//hvis den er kort kan vi bare returnere
	if (len < TARGET_VISIBLE_URL_LEN) {
		#ifdef DEBUG
 			bblog(DEBUGINFO, "shortenurl: url is short enough. Don't need to shorten");
		#endif

		snprintf(url, urllen, "%s%s", proto, origurl);
		return;
	}

  	if ((TokCount = split(url, "/", &Data)) > 1) {
		#ifdef DEBUG
		bblog(DEBUGINFO, "seperator: / ");
		#endif
		strcpy(slash,"/");
	}
	else if ((TokCount = split(url, "\\", &Data)) > 1) {
		#ifdef DEBUG
		bblog(DEBUGINFO, "seperator: \\ ");
		#endif
		strcpy(slash,"\\");
	}
	else {
		bblog(ERROR, "can't split url");
		snprintf(url, urllen, "%s%s", proto, origurl);
		return;
	}

	/*
	// Debug:
	Count = 0;
  	while( (Data[Count] != NULL) ) {
    		printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
  		printf("\n");

		Count++;
  	}
	*/
	--TokCount; //split ser ut til å begynner på 1, ikke på 0 

	#ifdef DEBUG
  	bblog(DEBUGINFO, "\tfound %d token(s):", TokCount);
	#endif

  	Count = 0;

	added = 0;
	suburllen = 0;
	while( (Data[Count] != NULL) ) {
		bblog(DEBUGINFO, "a: \t\t%d\t\"%s\"", Count, Data[Count]);
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

	strlcat(newurl, "...", sizeof(newurl));

	Count = TokCount;
	added = 0;
	suburllen = 0;
	while( (Count > 0) ) {
		bblog(DEBUGINFO, "b: \t\t%d\t\"%s\"", Count, Data[Count]);

		suburllen = strlen(Data[Count]);
		if ((added + suburllen) < (TARGET_VISIBLE_URL_LEN * 0.7)) {
			#ifdef DEBUG
			bblog(DEBUGINFO, "candidate %s",Data[Count]);
			#endif
		}
		else {
			break;
		}

		added += suburllen;
		--Count;
	}

	bblog(DEBUGINFO, "TokCount %i, count %i",TokCount,Count);

	//hvis også siste navn er for langt, hånterer vi det spesifikt.
	if (TokCount == Count) {
		bblog(DEBUGINFO, "bb");
		strlcat(newurl,slash,sizeof(newurl));
		strlcat(newurl,Data[Count],sizeof(newurl));		

	}
	else {
		for (i=Count+1;i<TokCount+1;i++) {
			bblog(DEBUGINFO, "c: \t\t%d\t\"%s\"", i, Data[i]);

			bblog(DEBUGINFO, "newurl: len %i, \"%s\"",strlen((char*)newurl),newurl);

                	strlcat(newurl,slash,sizeof(newurl));
			strlcat(newurl,Data[i],sizeof(newurl));
		}
	}

	bblog(DEBUGINFO, "shortenurl 1: newurl \"%s\"",newurl);
	//runarb 27 mai
	//Hvis den har et utf 8 tegn der vi slutter å kopierer for vi med bare halve tegnet, og bryter da xml'en
	bblog(DEBUGINFO, "strlen %i, size %i",strlen((char*)newurl),sizeof(newurl));
	i = strlen((char*)newurl) -1;
	while(i!=0 && newurl[i] > 127) {
		bblog(DEBUGINFO, "removing char %c",newurl[i]);
		newurl[i] = '\0';
		--i;
	}
	
	bblog(DEBUGINFO, "shortenurl 2: newurl \"%s\"",newurl);

	FreeSplitList(Data);
	
	snprintf(url, urllen, "%s%s", proto, newurl);
}

#ifdef WITH_SHORTENURL_MAIN

int
main(int argc, char **argv)
{
	char *url = strdup("file:\\\\192.168.22.25\\filer\\Kunder_Norge_Sverige\\Ringnes\\2002\\Brus\\Pepsi\\Gammelt\\Tester\\Pepsi spÃ¸rreund..ppt");

	globalOptVerbose = 1;

	shortenurl(url,255);

	printf("url %s\n",url);
}

#endif
