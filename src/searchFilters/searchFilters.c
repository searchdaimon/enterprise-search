#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>


#include "searchFilters.h"

#include "../common/url.h"
#include "../common/lot.h"
#include "../common/utf8-strings.h"
#include "../common/bstr.h"


int pi_switch(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

        int i;
        int count = 0;

        for (i=0;i<showabal;i++) {

                if (!Sider[i].deletet) {

                        if (
                                (CurentSider->subname.config.isPaidInclusion)
                                && ((Sider[i].iindex.DocID == CurentSider->iindex.DocID) || (strcmp(Sider[i].DocumentIndex.Url,CurentSider->DocumentIndex.Url) == 0))
                        ) {
                                #ifdef DEBUG
                                        printf("pi_switch: DocID or url is the same for Url \"%s\" DocID %u == \"%s\" DocID %u\n",
                                                Sider[i].DocumentIndex.Url,Sider[i].iindex.DocID,CurentSider->DocumentIndex.Url,CurentSider->iindex.DocID);
                                #endif
                                //bytter om slik at den beste blir pi side også, så vil vi filtrere ut denne vi har np
                                Sider[i].subname.config.isPaidInclusion = CurentSider->subname.config.isPaidInclusion;
				Sider[i].cache_params = CurentSider->cache_params;

                                return 1;

                                ++count;
                        }

                }
        }

        return 0;

}


void filtersTrapedReset(struct filtersTrapedFormat *filtersTraped) {

        filtersTraped->filterAdultWeight_bool	= 0;
        filtersTraped->filterAdultWeight_value	= 0;
        filtersTraped->filterSameCrc32_1	= 0;
        filtersTraped->filterSameUrl		= 0;
        filtersTraped->find_domain_no_subname	= 0;
        filtersTraped->filterSameDomain		= 0;
        filtersTraped->filterTLDs		= 0;
        filtersTraped->filterResponse		= 0;
        filtersTraped->cantpopResult		= 0;
        filtersTraped->cmc_pathaccess		= 0;
        filtersTraped->filterSameCrc32_2	= 0;
        filtersTraped->cantDIRead		= 0;
        filtersTraped->getingDomainID 		= 0;
        filtersTraped->sameDomainID 		= 0;
        filtersTraped->filterNoUrl 		= 0;

}

int filterSummery(char summery[]) {
	if (summery[0] == '\0') {
		printf("summery \"%s\" is emty\n",summery);
		return 1;
	}
	else if (strnlen(summery,200) < 10) {
		printf("summery \"%s\" is to short\n",summery);
		return 1;
	}

	return 0;
}
int filterTitle(char title[]) {

	if (title[0] == '\0') {
		printf("title \"%s\" is emty\n",title);
		return 1;
	}
	else if (strnlen(title,200) < 2) {
		printf("title \"%s\" is to short\n",title);
		return 1;
	}
	else if (detect_no_uppercase((unsigned char *)title)) {
		printf("title \"%s\" is all lovercase\n",title);
		return 1;
	}

	return 0;
}

int filterAdultWeight_bool(char AdultWeight,int adultpages,int noadultpages) {

	if (noadultpages > 10) {
		//hvis vi har mange nokk sider, viser vi ikke en side hvis den er adult
		if (AdultWeight) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		//hvis vi har fårfå sider så filtrerer vi ikke
		return 0;
	}

}

int filterAdultWeight_value(int AdultWeight,int adultpages,int noadultpages) {

	if (noadultpages > 10) {
		//hvis vi har mange nokk sider, viser vi ikke en side hvis den er adult
		if (AdultWeight >= AdultWeightForXXX) {
			printf("filterAdultWeight_value: filtered %i\n",AdultWeight);
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		//hvis vi har fårfå sider så filtrerer vi ikke
		return 0;
	}

}


int filterResponse(int responscode) {

	//filtrerer hvis det ikke er lovlige responskoder
	//runarb 25.10.2007: Jayde vil ha bare ok sider
	//if ((responscode == 200) || (responscode == 301) || (responscode == 302) || (responscode == 0)) {
	if ((responscode == 200) || (responscode == 203 )) {
		return 0;
	}
	else {
		return 1;
	}

}


int filterSameCrc32(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

	int i;
	int count = 0;

	for (i=0;i<showabal;i++) {

		if (!Sider[i].deletet) {

			if (Sider[i].DocumentIndex.crc32 == (*CurentSider).DocumentIndex.crc32) {
				#ifdef DEBUG
				printf("crc32 is the same for Url \"%s\" == \"%s\"\n",Sider[i].DocumentIndex.Url,(*CurentSider).DocumentIndex.Url);
				printf("crc32: %u == %u\n",Sider[i].DocumentIndex.crc32,(*CurentSider).DocumentIndex.crc32);
				#endif
				return 1;

				++count;
			}		

		}
	}

	return 0;

}

int filterSameDomainID(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

	int i;
	int count = 0;

	for (i=0;i<showabal;i++) {

		if (!Sider[i].deletet) {

			#ifdef DEBUG
			printf("filterSameDomainID: showabal %i, count %iSider %hu, CurentSider %hu\n",showabal,count,Sider[i].DomainID,(*CurentSider).DomainID);
			#endif

			if (Sider[i].DomainID == (*CurentSider).DomainID) {
				#ifdef DEBUG
				printf("DomainID is the same\n");
				#endif
		

				++count;
			}		

		}
	}

	if (count < 2) {
		return 0;
	}
	else {
		return 1;
	}


}

int filterSameUrl(int showabal,char url[], struct SiderFormat *Sider) {

	int i;

	for (i=0;i<showabal;i++) {

		if (!Sider[i].deletet) {

			if (strcmp(Sider[i].url,url) == 0) {
				//printf("Url is the same\n");
				return 1;
			}		

		}
	}
	
	return 0;
}



int filterResponseCode(struct SiderFormat *CurentSider) {
	if (CurentSider->DocumentIndex.response > 204) {
	//if (CurentSider->DocumentIndex.response > 302) {
		#ifdef DEBUG
		printf("filterResponseCode:page har bad respons code %i\n",CurentSider->DocumentIndex.response);
		#endif

		return 1;
	}

	return 0;
}

//fjerner sider med samme IPadresse
int filterSameIp(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

	int i;
	int count = 0;

	for (i=0;i<showabal;i++) {
		if (Sider[i].DocumentIndex.IPAddress == (*CurentSider).DocumentIndex.IPAddress) {
			printf("IP is the same\n");

			++count;
		}		
	}

	if (count < 2) {
		return 0;
	}
	else {
		return 1;
	}
}

//fjerner sider som har samme beskrivelse
int filterDescription(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

        int i;

        for (i=0;i<showabal;i++) {
                if ((strlen((*CurentSider).description) > 50) && (strcmp(Sider[i].description,(*CurentSider).description) == 0)) {
                        //printf("Description is the same\n");

                        return 1;
                }
        }

	return 0;
}


//fjerner sider med samme domene
int filterSameDomain(int showabal,struct SiderFormat *CurentSider, struct SiderFormat *Sider) {

	int i;
	int count = 0;

	//filtrerer ikke sider vi ikke har noe domne for. Typisk ppc anonser som peger til out.cfi side på samme domene
	if ((*CurentSider).domain[0] == '\0') {
		return 0;
		#ifdef DEBUG
			printf("Warn: domain is blank, wont try to filter it.\n");
		#endif
	}

	for (i=0;i<showabal;i++) {

		if (!Sider[i].deletet) {

			if (strcmp((*CurentSider).domain,Sider[i].domain) == 0) {
				#ifdef DEBUG
				if (count < 2) {
				printf("domain is the same. Urls Url \"%s\" (domain \"%s\", DociD %u-%i, DomainID %ho) == \"%s\" (domain \"%s\", DocID %u-%i, DomainID %ho)\n",
					Sider[i].DocumentIndex.Url,Sider[i].domain,Sider[i].iindex.DocID,rLotForDOCid(Sider[i].iindex.DocID),Sider[i].DomainID,
					(*CurentSider).DocumentIndex.Url,(*CurentSider).domain,(*CurentSider).iindex.DocID,rLotForDOCid((*CurentSider).iindex.DocID),(*CurentSider).DomainID);
				}
				#endif
				//printf("domain is the same. %s == %s\n",(*CurentSider).domain,Sider[i].domain);

				//runarb: 14.11.2007: hva gjør linjen nedenfor???
				//(*CurentSider).posisjon = Sider[i].posisjon;
			
				++count;
			}	
		}

	}

	#ifdef DEBUG
	printf("have a total of %i from this domain\n",count);
	#endif

	if (count < 2) {
		return 0;
	}
	else {
		return 1;
	}
}

int filterTLDs(char domain[]) {

        char *cpoint;

        char *bannedDomains[] = {"ch","ru","be","cz","hr","it","pl","de"};
        int nrOfbannedDomains = (sizeof(bannedDomains) / sizeof(char *));
        int i;

        if ((cpoint = strchr(domain,'.')) == NULL) {
                printf("bad domain\n");
                return 1;
        }
        ++cpoint;

        //printf("%s\n",cpoint);

        for (i=0;i<nrOfbannedDomains;i++) {
                //printf("%i: %s\n",i,bannedDomains[i]);

                if (strcmp(bannedDomains[i],cpoint) == 0) {
                        return 1;
                }
        }

        return 0;

}

int filterDomainNrOfLines(char domain[]) {

      #define maxdomainlines 2

      int n=0;
      char *cpnt;


      cpnt = domain;
      while ((cpnt = strchr(cpnt,'-')) != NULL) {
              ++n;
              ++cpnt;
      }

      if (n > maxdomainlines) {
              return 1;
      }
      else {
                return 0;
        }

}


int filterDomainLength(char domain[]) {

      #define maxdomainlen 30;

      if (strlen(domain) > 30) {
              return 1;
      }
      else {
              return 0;
      }

}


