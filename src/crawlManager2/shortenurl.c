#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/bstr.h"
#include "../logger/logger.h"

#ifdef BLACK_BOKS
	#define TARGET_VISIBLE_URL_LEN 80
#else
	#define TARGET_VISIBLE_URL_LEN 60
#endif

#ifdef WITH_SHORTENURL_MAIN
int globalOptVerbose = 0;
#endif

void shortenurl(char *url,int urllen) {

  	char **Data;
	size_t *datalen, protolen;
  	int Count, TokCount;
	#ifdef BLACK_BOKS
		unsigned char newurl[128];
	#else
		unsigned char newurl[201];
	#endif

	char slash[2];
	int len;
#ifdef DEBUG
	bblog(DEBUG, "shortenurl: inn url %s", url);
#endif
	char *p;
	char proto[128];
	char origurl[urllen+1];
	
	newurl[0] = '\0';
	proto[0] = '\0';

	//tar bort proto:// f¯rst
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
	} else {
		len = strlen(url);
	}	
	strcpy(origurl, url);

	#ifdef DEBUG
		bblog(DEBUG, "shortenurl: after proto \"%s\"",url);
	#endif

	//hvis den er kort kan vi bare returnere
	if (len < TARGET_VISIBLE_URL_LEN) {
		#ifdef DEBUG
 			bblog(DEBUG, "shortenurl: url is short enough. Don't need to shorten");
		#endif

		snprintf(url, urllen, "%s%s", proto, origurl);
		return;
	}

  	if ((TokCount = split(url, "/", &Data)) > 1) {
		#ifdef DEBUG
		bblog(DEBUG, "seperator: /");
		#endif
		strcpy(slash,"/");
	}
	else if ((TokCount = split(url, "\\", &Data)) > 1) {
		#ifdef DEBUG
		bblog(DEBUG, "seperator: \\");
		#endif
		strcpy(slash,"\\");
	}
	else {
		bblog(ERROR, "can't split url: %s", origurl);
		snprintf(url, urllen, "%s%s", proto, origurl);
		return;
	}

	Count = 0;
	datalen = malloc(sizeof(size_t) * TokCount);
  	while( (Data[Count] != NULL) ) {
    		//printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
  		//printf("\n");
		datalen[Count] = strlen(Data[Count]);

		Count++;
  	}
	--TokCount; //split ser ut til Â begynner pÂ 1, ikke pÂ 0 

#ifdef DEBUG
  	bblog(DEBUG, "found %d token(s):",  TokCount);

	{
	int a;
	for (a = 0; Data[a] != NULL; a++) {
		bblog(DEBUG, "    %s (%d)",  Data[a], strlen(Data[a]));
	}
	}
#endif


	/* Minmum length of filename */
#define PRESERVE_SPACE 20 /* XXX: Find suitable number */
	protolen = strlen(proto);
	/* We really want the first part (alias, or ip), first directory and first part of file name */
	if (TokCount >= 3 && (protolen + datalen[0] + datalen[1] + datalen[TokCount-1]) > TARGET_VISIBLE_URL_LEN) {
		/* Shorten last element enough */
		if (datalen[TokCount-1] > PRESERVE_SPACE) {
			Data[TokCount-1][PRESERVE_SPACE] = '\0';
			Data[TokCount-1][PRESERVE_SPACE-3] = '\xE2';
			Data[TokCount-1][PRESERVE_SPACE-2] = '\x80';
			Data[TokCount-1][PRESERVE_SPACE-1] = '\xA6';
			datalen[TokCount-1] = PRESERVE_SPACE;
		}

		if (protolen + datalen[0] + datalen[1] + datalen[TokCount-1] <= TARGET_VISIBLE_URL_LEN) {
			snprintf(url, urllen, /*proto*/"%s" /*ip*/"%s" /*slash*/ "%s"
			    /*firstdir*/ "%s" /*slash*/"%s" /*dot*/"%s" /*slash*/"%s" /*file*/"%s",
			    proto, Data[0], slash, Data[1], slash, "\xE2\x80\xA6", slash, Data[TokCount-1]);
			goto shortenurllongdone;
		}
		if (datalen[1] > PRESERVE_SPACE) {
			Data[1][PRESERVE_SPACE] = '\0';
			Data[1][PRESERVE_SPACE-3] = '\xE2';
			Data[1][PRESERVE_SPACE-2] = '\x80';
			Data[1][PRESERVE_SPACE-1] = '\xA6';
			datalen[1] = PRESERVE_SPACE;
		}
		if (protolen + datalen[0] + datalen[1] + datalen[TokCount-1] <= TARGET_VISIBLE_URL_LEN) {
			snprintf(url, urllen, /*proto*/"%s" /*ip*/"%s" /*slash*/ "%s"
			    /*firstdir*/ "%s" /*slash*/"%s" /*dot*/"%s" /*slash*/"%s" /*file*/"%s",
			    proto, Data[0], slash, Data[1], slash, "\xE2\x80\xA6", slash, Data[TokCount-1]);
			goto shortenurllongdone;
		}
		if (datalen[0] > PRESERVE_SPACE) {
			Data[0][PRESERVE_SPACE] = '\0';
			Data[0][PRESERVE_SPACE-3] = '\xE2';
			Data[0][PRESERVE_SPACE-2] = '\x80';
			Data[0][PRESERVE_SPACE-1] = '\xA6';
			datalen[0] = PRESERVE_SPACE;
		}
		if (protolen + datalen[0] + datalen[1] + datalen[TokCount-1] <= TARGET_VISIBLE_URL_LEN) {
			snprintf(url, urllen, /*proto*/"%s" /*ip*/"%s" /*slash*/ "%s"
			    /*firstdir*/ "%s" /*slash*/"%s" /*dot*/"%s" /*slash*/"%s" /*file*/"%s",
			    proto, Data[0], slash, Data[1], slash, "\xE2\x80\xA6", slash, Data[TokCount-1]);
			goto shortenurllongdone;
		}

		snprintf(url, urllen, "%.*s", PRESERVE_SPACE, origurl);
 shortenurllongdone:
 		;
	} else {
		int n = 0;
		size_t totlen, rtotlen;
		size_t bp, fp;
		char *p;
		int cur;
		int lastdots;

		totlen = snprintf(url, urllen-totlen, "%s", proto);
		//printf("url: %s proto: %s %d\n", url, proto, totlen);
		totlen += snprintf(url+totlen, urllen-totlen, "%s%s", Data[0], slash);
		//printf("url: %s proto: %s %d\n", url, proto, totlen);
		bp = TokCount - 1;
		fp = 1;
		lastdots = 0;
		rtotlen = totlen;
		while (totlen+PRESERVE_SPACE < TARGET_VISIBLE_URL_LEN && bp >= fp && n < TokCount) {
			//printf("%d < %d && %d >=  %d && %d < %d\n", totlen+20, TARGET_VISIBLE_URL_LEN, bp, fp, n, TokCount);
			
			if ((n & 1) == 0) {
				if ((lastdots & 1) == 1) {
					n++;
					//printf("dot dot 1\n");
					continue;
				}
				cur = fp;
			} else {
				if ((lastdots & 2) == 2) {
					n++;
					//printf("dot dot 2\n");
					continue;
				}
				cur = bp;
			}
			//printf("Itr: %d fp: %d bp: %d cur: %d\n", n, fp, bp, cur);

			//printf("trying: %s %d\n", Data[cur], strlen(Data[cur]));
			//printf("%d + %d + %d < %d\n", totlen, PRESERVE_SPACE, snprintf(NULL, 0, "%s%s", Data[cur], slash), TARGET_VISIBLE_URL_LEN);
			if (totlen+PRESERVE_SPACE+snprintf(NULL, 0, "%s%s", Data[cur], slash) < TARGET_VISIBLE_URL_LEN) {
				//printf("Wanted: %d\n", cur);
				totlen += snprintf(NULL, 0, "%s%s", Data[cur], slash);
				if ((n & 1) == 0) 
					fp++;
				else
					bp--;

			} else {
				lastdots = 1;
				if (lastdots == 0)
					totlen += snprintf(NULL, 0, "...%s", slash);
				if ((n & 1) == 0) {
					lastdots |= 1;
					//printf("setting dot 1...\n");
				} else {
					lastdots |= 2;
					//printf("setting dot 2...\n");
				}
			}
			
			n++;
		}
		//printf("Total length: %d\n", totlen);
		//printf("bp: %d, fp: %d\n", bp, fp);
		/* Get fp */
		for (cur = 1; cur < fp; cur++) {
			//printf("cur: %d totlen: %d urllen: %d\n", cur, rtotlen, urllen);
			//printf("before: %s %s %d\n", url, Data[cur], urllen-rtotlen);
			rtotlen += snprintf(url+rtotlen, urllen-rtotlen, "%s%s", Data[cur], slash);
			//printf("after: %s %s\n", url);
		}
		if (bp >= fp) {
			rtotlen += snprintf(url+rtotlen, urllen-rtotlen, "%s%s", "\xE2\x80\xA6", slash);
			//printf("%s\n", url);
		}
		for (cur = bp+1; cur < TokCount; cur++) {
			//printf("2 cur: %d totlen: %d\n", cur, rtotlen);
			rtotlen += snprintf(url+rtotlen, urllen-rtotlen, "%s%s", Data[cur], slash);
			//printf("%s %s\n", url, Data[cur]);
		}

		//printf("%d %s\n", strlen(Data[TokCount]), Data[TokCount]);

		if (strlen(Data[TokCount]) > TARGET_VISIBLE_URL_LEN-rtotlen) {
			size_t endlen;
			char *end = strrchr(Data[TokCount], '.');
			if (end == NULL)
				endlen = 0;
			else
				endlen = strlen(end);
			
			//printf("File will be %d long\n", TARGET_VISIBLE_URL_LEN-rtotlen-endlen-3);
			{
				int i = TARGET_VISIBLE_URL_LEN-rtotlen-endlen-2;
				Data[TokCount][i] = '\0';
				i--;
				/* Don't cut in the middle of a utf-8 character */
				while (Data[TokCount][i] & 0x80) {
					Data[TokCount][i] = '\0';
					i--;
				}
			}
			rtotlen += snprintf(url+rtotlen, urllen-rtotlen, "%s\xE2\x80\xA6%s", Data[TokCount], end);
		} else {
			rtotlen += snprintf(url+rtotlen, urllen-rtotlen, "%s", Data[TokCount]);
		}

	}

	FreeSplitList(Data);
}

#ifdef WITH_SHORTENURL_MAIN

int
main(int argc, char **argv)
{
	//char *url = strdup("file:\\\\192.168.22.25\\filer\\Kunder_Norge_Sverige\\Ringnes\\2002\\Brus\\Pepsi\\Gammelt\\Tester\\Pepsi sp√∏rreund..ppt");
	//char *url = strdup("[Administration]\\Diverse fra Runar\\googlexlsx\\OptimizationModel_for_Business_Productivity_Solutions.xlsx");
	//char *url = strdup("[Administration]\progs\SuperOffice 6.1 SR1 CD\STDReportFiles\FI Report Files\Kalenteri (1 p‰iv‰).txt");
	//char *url = strdup("http://sp2007-01/sites/crawltest/_catalogs/masterpage/Preview%20Images/searchresults.gif");
	char *url = strdup("file://213.179.58.125/Administration/Diverse%20fra%20Runar/dyp%20mappe/eksempel/2008/okober/16/raporrter/Lorem%20ipsum%20dolor%20sit%20amet,%20consectetur%20adipisicing%20elit/do%20eiusmod%20tempor%20incididunt%20ut%20labore%20et%20dolore%20magna%20aliqua/boitho%20lang%20dyp%20test.txt");

	globalOptVerbose = 1;

	shortenurl(url,255);

	printf("url %s\n",url);

	return 0;
}

#endif
