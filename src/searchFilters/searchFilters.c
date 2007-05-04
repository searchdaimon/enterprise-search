#include <stdio.h>
#include <string.h>

#include "searchFilters.h"

#include "../common/url.h"

int filterAdultWeight(char AdultWeight,int adultpages,int noadultpages) {

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


int filterResponse(int responscode) {

	//filtrerer hvis det ikke er lovlige responskoder
	if ((responscode == 200) || (responscode == 301) || (responscode == 302)) {
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
		if (Sider[i].DocumentIndex.crc32 == (*CurentSider).DocumentIndex.crc32) {
			//printf("crc32 is the same\n");

			return 1;

			++count;
		}		
	}

	return 0;

}

int filterSameUrl(int showabal,char url[], struct SiderFormat *Sider) {

	int i;

	for (i=0;i<showabal;i++) {
		if (strcmp(Sider[i].url,url) == 0) {
			//printf("Url is the same\n");
			return 1;
		}		
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
	char domainCuren[65];
	char domainOther[65];

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
				//printf("domain is the same. %s == %s\n",(*CurentSider).domain,Sider[i].domain);

				(*CurentSider).posisjon = Sider[i].posisjon;
			
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


