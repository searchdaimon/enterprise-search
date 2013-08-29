#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "define.h"
#include "debug.h"
#include "bstr.h"
#include "url.h"
#include "strlcat.h"
#include "utf8-strings.h"

//domener som har Country code second-level domain ccSLD domenesystem, slik som co.uk i name.co.uk
char *SecondLevelDomain[] = {"uk","au","nz",NULL};

int strchrcount(char s[], char c) {

	char *cpnt = s;
	int count = 0;

	while ((cpnt = strchr(cpnt,c)) != NULL) {
		++cpnt;
		++count;
	}

	return count;
}

/**********************************************************************************************

Rutine for å finne urler som er rekursive av natur, noe sombetyr at de har to like mapper, for eksempel:

http://www.swindon.gov.uk/atozlistingsbyletterdisplay/socialcare/signpost/socialcare/signpost/community/roadstransport/feedbackcomments.htm

**********************************************************************************************/
int urlMayBeRecursive(const char url[]) {

	#define _MAX_DIRS 10
	#define _MAX_DIR_LEN 50

	const char *p, *last;
	char directories[_MAX_DIRS ][_MAX_DIR_LEN];
	int i, y, z, len;

	#ifdef DEBUG	
		printf("urlMayBeRecursive: url: \"%s\"\n",url);
	#endif

	p = url;
	
	//søker os til protokoll
	if ((p = strstr(p,"//")) == NULL) {
		printf("urlMayBeRecursive: bad formated url \"%s\".\n",url);
		return 1;
	}
	++p;++p;

	//søker oss til slutten av domene
	if ((p = strchr(p,'/')) == NULL) {
		printf("urlMayBeRecursive: bad formated url \"%s\".\n",url);
		return 1;
	}
	++p;

	i = 0;	
	last = p;
	while ( ((p = strchr(p,'/')) != NULL) && (i < _MAX_DIRS) ) {

		len = p - last;
		#ifdef DEBUG
			printf("urlMayBeRecursive: p \"%s\", len %i\n",last, len );
		#endif

		if (len > _MAX_DIR_LEN) {
			len = _MAX_DIR_LEN;
		}

		strncpy(directories[i],last,len);
		directories[i][len] = '\0';

		++p;
		++i;
		last = p;

	}	

	for (y=0;y<i;y++) {
		#ifdef DEBUG
			printf("urlMayBeRecursive: dirs \"%s\"\n",directories[y]);
		#endif
		//samenligner med alle andre
		for (z=0;z<i;z++) {
			//samenligner ikke med seg selv
			if (y != z) {
				if (strcmp(directories[y],directories[z]) == 0) {
					printf("urlMayBeRecursive: url \"%s\" have recursive folder \"%s\", nr %i == \"%s\", nr %i\n",url,directories[y],y,directories[z],z);
					return 1;
				}
			}
		}
	}

	return 0;
}



int url_depth(char url[]) {

	int len;

	if (strncmp(url,"http://",7) != 0) {
		return -1;
	}

	len = strchrcount(url +7, '/');

	//teller ikke med den siste /. slik at  http://www.boitho.com/div/file.html og http://www.boitho.com/div/mappe/ blir det samme
	if (url[strlen(url) -1] == '/') {
		#ifdef DEBUG
		printf("url_depth: Url \"%s\" har / as last char\n",url);
		#endif
		--len;


	}
		
	return len;
}

int find_domain_path_len (char url[]) {


        char *cpnt;

        //søker oss til / og kapper
        if ((cpnt = strchr(url + 7,'/')) == NULL) {
		#ifdef DEBUG
			printf("bad url\n");
		#endif
		
                return 0;
        }
        else {
                #ifdef DEBUG
			printf("aa %s, len %i\n",cpnt,strlen(cpnt));
                #endif

                return strlen(cpnt);
        }
}


int isWikiUrl(char url[]) {

	if (strstr(url,"/wiki/") != NULL) {
		#ifdef DEBUG
		printf("is wiki: \"%s\"\n",url);
		#endif
		return 1;
	}
	else {
		return 0;
	}

}

//div intresange teknikker er listet på http://en.wikipedia.org/wiki/URL_normalization
//
// det finnes et program src/urltest/ som unit tester dette
int url_normalization (char url[], int urlsize) {

	char domain[urlsize];
	char *cpnt;
	int subdomains;


	if (url[0] == 0) {
		fprintf(stderr,"url_normalization: Got emty url \"%s\"\n",url);
		return 0;

	}

	if (strnlen(url,urlsize) < 10) {
		fprintf(stderr,"url_normalization: bad url \"%s\"\n",url);
		return 0;
	}

	/*
		Tar bort # og alt etter i urlen. Eks http://www.boitho.com/side.html#august blir til http://www.boitho.com/side.html
	*/
        if ( (cpnt = strchr((url +7),'#')) != NULL) {
		cpnt[0] = '\0';
	}

	/*
	Removing the default port. 
	The default port (port 80 for the .http. scheme) may be removed from (or added to) a URL. 

	Example: 
	http://www.example.com:80/bar.html . http://www.example.com/bar.html 
	*/
	if ((cpnt = strstr((url +7),":80/")) != NULL) {

		strcasesandr (url, urlsize, ":80/", "/");
	}

	


	/*
	Adding trailing / Directories are indicated with a trailing slash and should be included in links. 

	Example: 
	http://www.example.com . http://www.example.com/ 
	*/
        //søker oss til / , hvis vi ikke har har vi nokk en domene skrevet som www.boitho.com.
	//vi ønsker da og legge på / for og få www.boitho.com/
	// kan også være www.boitho.com?query=chat. Så hvis det er ? i den kan vi ikke bare apende en /
        if ( ((cpnt = strchr((url +7),'/')) == NULL) && ((cpnt = strchr((url +7),'?')) == NULL)) {
		#ifdef DEBUG
			printf("no trailing slash. Url \"%s\"\n",url);
		#endif
		
		strlcat(url,"/",urlsize);
		
		#ifdef DEBUG
			printf("no trailing slash. revriten to Url \"%s\"\n",url);
		#endif
        }

	/*
	//har ikke /, men har ?, så det må være http://www.boitho.com?a=b
	13% av redirect urlene har http://www.boitho.com?a=b type url, alstå uten / etter .com 
	*/

        if ( ((cpnt = strchr((url +7),'/')) == NULL) && ((cpnt = strchr((url +7),'?')) != NULL)) {

		strcasesandr (url, urlsize, "?", "/?");
		
        }

	//nå når vi har et fungerende domene kan vi prøve å finne domene
	if (!find_domain(url,domain,urlsize) ) {
		debug("gyldig_url: can't find domain. Url \"%s\"\n",url);
		return 0;
	}

	//teller subdomener
	subdomains = strchrcount(domain,'.');

	/*
	legger til www. foran en url. Dette da vi kan være rimelig sikker på at http://boitho.com og http://www.boitho.com
	er samme side
	*/
	/*
	dette brekker rankering for pi urler.
	if (subdomains == 1) {
		strcasesandr (url, urlsize, "http://", "http://www.");		
	}
	*/
	/*
	lovercaser domenet og protokoll
	
	Converting the scheme and host to lower case. The scheme and host components of the URL are 
	case-insensitive. Most normalizers will convert them to lowercase. 
	
	Example: 
	HTTP://www.Example.com/ . http://www.example.com/
	*/
	if ((cpnt = strchr((url +7),'/')) != NULL) {

		convert_to_lowercase_n((unsigned char *)url,(cpnt-url));
		
	}

	return 1;
}


int NotDuplicateUrl(char word[]) {

		if (strstr(word,"/default") != NULL) {
			debug("hav /default",word);
			return 0;
		}
		else if (strstr(word,"/index") != NULL) {
			debug("hav /index",word);
			return 0;
		}
		else if (strstr(word,"/www1") != NULL) {
			debug("hav /www1",word);
			return 0;
		}
		else if (strstr(word,"/www2") != NULL) {
			debug("hav /www2",word);
			return 0;
		}
		else if (strstr(word,"/www3") != NULL) {
			debug("hav /www3",word);
			return 0;
		}
		else if (strstr(word,"/www4") != NULL) {
			debug("hav /www4",word);
			return 0;
		}


	return 1;
}

int legalUrl(char word[]) {

                static char lasturl[201] = {""};
                int lengt;
                char *cpnt;
                char *filetype;
		int querylength;
		struct updateFormat updatePost;
		char domain[128];
		int subdomains;
		int domainNrOfLines;

                lengt = strlen(word);

		if (!find_domain(word,domain,sizeof(domain)) ) {
			debug("gyldig_url: can't find domain. Url \"%s\"\n",word);
			return 0;
		}

		//teller subdomener
		subdomains = strchrcount(domain,'.');
		

		domainNrOfLines = strchrcount(domain,'-');

		
                //finner siste bokstav
                if (word[lengt -1] == '/') {
                        filetype = &word[lengt -1];
                }
                else {
                        if ((filetype = strrchr(word,'.')) == NULL){
                                filetype = NULL;
                        }
                }

		//finner lengde på query. Altså det som er etter ?
		querylength = 0;
		if ((cpnt = strstr(word,"?")) != NULL) {
			querylength = strlen(cpnt +1); //+1 da vi ikke skal ha med ?
		}


                //temp: for nå tar vi bare korte urler
                if (lengt > 150) {
			debug("To long. Was %i bytes",lengt);
			return 0;
                }
		else if (querylength > 21) {
			debug("To long query. Was %i bytes",querylength);
			return 0;
		}
		else if (subdomains == 1) {
			//if not will we index boitho.com and www.boitho.com. Now are we only takint he www.boitho"
			debug("To few subdomains. Most have one. Url: \"%s\"\n",word);
			return 0;
		}
		else if (domainNrOfLines >= 3) {
			debug("To many lines in domain. Was %i lines. Url: \"%s\"\n",domainNrOfLines,word);
			return 0;
		}
		else if (word[lengt -1] == '\n') {
			debug("url has \\n as last char");
			return 0;
		}
                else if (filetype == NULL) {
                        debug("bad url. filetype is NULL");
			return 0;
                }
                else if (
                        !(
                        (strcmp(filetype,"/") == 0)
                        || (strstr(filetype,"?") != NULL)
                        || (strcmp(filetype,".cfm") == 0)
                        || (strcmp(filetype,".htm") == 0)
                        || (strcmp(filetype,".html") == 0)
                        || (strcmp(filetype,".shtml") == 0)
                        || (strcmp(filetype,".shtm") == 0)
                        || (strcmp(filetype,".php") == 0)
                        || (strcmp(filetype,".asp") == 0)
                        || (strcmp(filetype,".stm") == 0)
                        || (strcmp(filetype,".ece") == 0)
                        || (strcmp(filetype,".aspx") == 0)
                        || (strcmp(filetype,".do") == 0)
                        || (strcmp(filetype,".jsp") == 0)
                        || (strcmp(filetype,".jhtml") == 0)
			|| isWikiUrl(word)
                        )
                ) {
                        //tar bar de filendelsene vi vil ha
                        debug("bad ending \"%s\"",filetype);
			return 0;
                }
                else if (strchr(word,'#') != NULL) {
                        debug("has #: %s\n",word);
			return 0;
                }
                else if (strchr(word,'@') != NULL) {
                        debug("has @: %s\n",word);
			return 0;
                }
                else if (strchr(word,'%') != NULL) {
                        debug("has %: %s\n",word);
			return 0;
                }
                else if (strcmp(word,lasturl) == 0) {
                        debug("last and ne the same %s == %s",word,lasturl);
			return 0;
                }
		else if (strstr(word,"..") != NULL) {
			debug("hav ..");
			return 0;
		}
		else if (strstr(word,"/./") != NULL) {
			debug("hav /./");
			return 0;
		}
		else if (strstr((word + 7),"http:/") != NULL) {
			//for å hindre http://go.kvasir.sol.no/fs/http:/www.nettfinn.no/
			debug("hav http:/");
			return 0;
		}
		else if (strstr((word + 7),"http://") != NULL) {
			debug("hav multiple http://");
			return 0;
		}
		else if (strstr((word + 7),"https://") != NULL) {
			debug("hav https://");
			return 0;
		}
		else if (strstr((word + 7),"ftp://") != NULL) {
			debug("hav ftp://");
			return 0;
		}
		else if (strstr((word + 7),"//") != NULL) {
			debug("hav //");
			return 0;
		}
		else if (strstr(word,"mailto:") != NULL) {
			debug("hav mailto:",word);
			return 0;
		}
		else if ((char *)strcasestr(word,"javascript:") != NULL) {
			debug("hav mailto:",word);
			return 0;
		}
		else if (strstr(word,"//:") != NULL) {
			debug("hav //:",word);
			return 0;
		}
                else if (lengt > sizeof(updatePost.url)) {
                        debug("url to long at %i char",strlen(word));
			return 0;
                }
                else {
                        debug("link: %s\n",word);

			return 1;
		}

		printf("bug: did go to end in gyldig_url(). Isnt suposet to happend\n");
		exit(1);
}


int gyldig_url(char word[]) {

	if (!legalUrl(word)) {
		return 0;
	}
	else if(!NotDuplicateUrl(word)) {
		return 0;
	}
	else if (urlMayBeRecursive(word)) {
		return 0;
	}
	return 1;
}

int url_havpri1(char word[]) {

	if (
		(strstr((word + 7),".no/") != NULL)
		|| (strstr((word + 7),".gov/") != NULL)
		|| (strstr((word + 7),".mil/") != NULL)
		|| (strstr((word + 7),".edu/") != NULL)
		) {
 
        	return 1;
        }
	else {
		return 0;
	}
}

int url_havpri2(char word[]) {

	if (
		(strstr((word + 7),".se/") != NULL)
		|| (strstr((word + 7),".dk/") != NULL)
		|| (strstr((word + 7),".uo.uk/") != NULL)
		|| (strstr((word + 7),".com/") != NULL)
		) {
        	return 1;
        }
	else {
		return 0;
	}
}

int isOkTttl(char word[]) {

//Barry
//> 2) Initially we can start with .com, .net, .org., .gov, . mil, .edu,.biz.,
//> .info, .us as well as 
//> .uk, .eu, .au and .nz from your existing index


	if (
		(strstr((word + 7),".com/") != NULL)
		|| (strstr((word + 7),".net/") != NULL)
		|| (strstr((word + 7),".org/") != NULL)
		|| (strstr((word + 7),".gov/") != NULL)
		|| (strstr((word + 7),".mil/") != NULL)
		|| (strstr((word + 7),".edu/") != NULL)
		|| (strstr((word + 7),".biz/") != NULL)
		|| (strstr((word + 7),".info/") != NULL)
		|| (strstr((word + 7),".us/") != NULL)
		|| (strstr((word + 7),".uk/") != NULL)
		|| (strstr((word + 7),".eu/") != NULL)
		|| (strstr((word + 7),".au/") != NULL)
		|| (strstr((word + 7),".nz/") != NULL)
		|| (strstr((word + 7),".fm/") != NULL)  // brukes av mange netradioer, som di.fm
		) {
        	return 1;
        }
	else {
		return 0;
	}

}

int url_isttl(char word[],char ttl[]) {
    char ttlfull[6];

    snprintf(ttlfull,sizeof(ttlfull),".%s/",ttl);

    if (strstr((word + 7),ttlfull) == NULL) {
            return 0;
    }
    else {
            return 1;
    }
}


int isSecondLevelDomain(char ttl[]) {

	int i = 0;
	
	while(SecondLevelDomain[i] != NULL) {

		#ifdef DEBUG
			printf("i %i, SecondLevelDomain %s ?= ttl %s\n",i,SecondLevelDomain[i],ttl);
		#endif

		if (strcmp(SecondLevelDomain[i],ttl) == 0) {
			return 1;
		}

		++i;
	}

	return 0;

}

int find_domains_subname(const char url[],char domain[], int sizeofdomain) {

 	char **Data;

	int TokCount;
	int i;
	int nrToGet;

	if (!find_domain(url,domain,sizeofdomain)) {
		return 0;
	}

  	TokCount = split(domain, ".", &Data);

	#ifdef DEBUG
	  	int Count;

		printf("url %s\n",url);

		printf("TokCount %i\n",TokCount);
  		Count = 0;
  		while( (Data[Count] != NULL) ) {
    			printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
			Count++;
		}
  		printf("\n");

		printf("nr %i\n",TokCount);

	#endif

	domain[0] = '\0';

	#ifdef DEBUG
		printf("isSecondLevelDomain domain \"%s\", i %i\n",Data[TokCount -1],isSecondLevelDomain(Data[TokCount -1]));
	#endif

	if (isSecondLevelDomain(Data[TokCount -1])) {
		nrToGet = 3;
	}
	else {
		nrToGet = 2;
	}

	if (TokCount < nrToGet) {
		printf("bad domain url: \"%s\", nr %i\n",url, TokCount);

		return 0;
	}


	
	for (i=0;i<(TokCount -nrToGet);i++) {
		strlcat(domain,Data[i],sizeofdomain);
		strlcat(domain,".",sizeofdomain);
	}


	domain[strlen(domain) -1] = '\0';

	#ifdef DEBUG
		printf("domain: \"%s\"\n",domain);
	#endif


  	FreeSplitList(Data);

  	return 1;

}

int find_domain_no_subname (const char url[],char domain[], int sizeofdomain) {

 	char **Data;
  	int TokCount;
	int i;
	int nrToGet;

	if (!find_domain_no_www(url,domain,sizeofdomain)) {
		return 0;
	}

  	TokCount = split(domain, ".", &Data);

	#ifdef DEBUG
		int Count;

		printf("url %s\n",url);

		printf("TokCount %i\n",TokCount);
  		Count = 0;
  		while( (Data[Count] != NULL) )
    			printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
  		printf("\n");

		printf("nr %i\n",TokCount);


	#endif

	domain[0] = '\0';

	#ifdef DEBUG
		printf("isSecondLevelDomain domain \"%s\", i %i\n",Data[TokCount -1],isSecondLevelDomain(Data[TokCount -1]));
	#endif

	if (isSecondLevelDomain(Data[TokCount -1])) {
		nrToGet = 3;
	}
	else {
		nrToGet = 2;
	}

	if (TokCount < nrToGet) {
		printf("bad domain url: \"%s\", nr %i\n",url, TokCount);
		return 0;
	}


	
	for (i=nrToGet;i>0;i--) {
		strlcat(domain,Data[TokCount -i],sizeofdomain);
		strlcat(domain,".",sizeofdomain);
	}


	domain[strlen(domain) -1] = '\0';


	#ifdef DEBUG
		printf("domain: \"%s\"\n",domain);
	#endif
	
	FreeSplitList(Data);

	return 1;

}
/*
//fjerner også www. i begyndelsen av en url, slik at det blir lettere å samenligne www.vg.no == vg.no
int find_domain_no_subname (const char url[],char domain[], int sizeofdomain) {
//ToDo har lagt til sizeofdomain, for å ungå buffer owerflow

	#define maxsubdomains 10

        char *cpnt;
        char buff[64];
        char darray[10][64];
        int i,y;
        //kutter av http:// (de 7 første tegnene)
        strscpy(domain,url + 7,sizeofdomain -1);




        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                printf("bad url. Dont have any / after protokoll, or domain (or subdomain) is to long Url \"%s\"\n",url);
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';

                strscpy(buff,domain,sizeof(buff));
                i = 0;
                while (((cpnt = strrchr(buff,'.'))) != NULL && (i<maxsubdomains)) {
                        strscpy(darray[i],cpnt,sizeof(darray[i]));
                        cpnt[0] = '\0';
                        ++i;
                }
                strscpy(darray[i],buff,sizeof(darray[i]));

                if (strlen(darray[1]) > 3) {
                        domain[0] = '\0';

                        snprintf(domain,sizeofdomain,"%s%s",darray[1],darray[0]);

                        if (strncmp(domain,"www",3) == 0) {
                                strcpy(domain,domain +3);
                        }

                        //fjerner . på begyndelsen
                        strcpy(domain,domain +1);
                }
                else {
                        //returnerer domanet slik vi fant det lengere opp
                        if (strncmp(domain,"www.",4) == 0) {
                                strscpy(domain,domain +4,sizeofdomain);
                        }

                }

		//18 mai 2007. Forsikrer os at vi altid slutter på \0
		//underlig at det er nædvendig. Men ser ut til at det kan oppstå feil som gir oss domene uten \0
		domain[sizeofdomain] = '\0';

                return 1;
        }
}
*/


//fjerner også www. i begyndelsen av en url, slik at det blir lettere å samenligne www.vg.no == vg.no
int find_domain_no_www (const char url[],char domain[], int sizeofdomain) {

        char *cpnt;


        //kutter av http:// (de 7 første tegnene)
        strscpy(domain,url + 7,sizeofdomain);

	if (strncmp(domain,"www.",4) == 0) {
		strscpy(domain,domain +4,sizeofdomain);
	}

        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
		#ifdef DEBUG
			printf("bad url\n");
		#endif
		
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}


int find_TLD(char domain[], char TLD[],int TLDsize) {

      char *cpoint;

      if ((cpoint = strrchr(domain,'.')) == NULL) {
                printf("bad domain\n");
                return 0;
        }
      //h<E5>pper over . i domene. Skal ha "com", ikke ".com"
      ++cpoint;

      strscpy(TLD,cpoint,TLDsize);
      return 1;
}



int find_domain (const char url[],char domain[],const int domainsize) {

        char *cpnt;


        //kutter av http:// (de 7 første tegnene)
        strscpy(domain,url + 7,domainsize);

        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
		#ifdef DEBUG
			printf("bad url\n");
		#endif
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}

int url_nrOfSubDomains(char url[]) {

	int subdomains;
	char domain[512];

	if (!find_domains_subname(url,domain,sizeof(domain)) ) {
		debug("gyldig_url: can't find domain. Url \"%s\"\n",url);
		return -1;
	}

	//hvis vi ikke har noen subdomene
	if (domain[0] == '\0') {
		return 0;
	}

	//teller subdomener
	subdomains = strchrcount(domain,'.');

	//test.ntnu er 2, ikke 1
	++subdomains;

	return subdomains;
}


