#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "verbose.h"

#include "../common/bstr.h"

#ifdef BLACK_BOKS
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
	#ifdef BLACK_BOKS
		unsigned char newurl[128];
	#else
		unsigned char newurl[201];
	#endif

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
		vboprintf("a: \t\t%d\t\"%s\"\n", Count, Data[Count]);
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
		vboprintf("b: \t\t%d\t\"%s\"\n", Count, Data[Count]);

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

	vboprintf("TokCount %i, count %i\n",TokCount,Count);

	//hvis også siste navn er for langt, hånterer vi det spesifikt.
	if (TokCount == Count) {
		printf("bb\n");
		strlcat(newurl,slash,sizeof(newurl));
		strlcat(newurl,Data[Count],sizeof(newurl));		

	}
	else {
		printf("aa\n");
		//printf("addint last part:\n");
		for (i=Count+1;i<TokCount+1;i++) {
			vboprintf("c: \t\t%d\t\"%s\"\n", i, Data[i]);

			printf("newurl: len %i, \"%s\"\n",strlen((char*)newurl),newurl);

                	strlcat(newurl,slash,sizeof(newurl));
			strlcat(newurl,Data[i],sizeof(newurl));
		}
	}

	vboprintf("shortenurl 1: newurl \"%s\"\n",newurl);
	//runarb 27 mai
	//Hvis den har et utf 8 tegn der vi slutter å kopierer for vi med bare halve tegnet, og bryter da xml'en
	printf("strlen %i, size %i\n",strlen((char*)newurl),sizeof(newurl));
	i = strlen((char*)newurl) -1;
	while(i!=0 && newurl[i] > 127) {
		printf("removing char %c\n",newurl[i]);
		newurl[i] = '\0';
		--i;
	}
	//if ((strlen(newurl) == (sizeof(newurl) -1) ) 
	//	// && ( (newurl[sizeof(newurl) -1] > 127) || (newurl[sizeof(newurl) -1] < 10)) 
	//	) {
	//	newurl[sizeof(newurl) -1] = 'X';
	//}	
	vboprintf("shortenurl 2: newurl \"%s\"\n",newurl);

	FreeSplitList(Data);
	
	//strscpy(url,newurl,urllen);
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
