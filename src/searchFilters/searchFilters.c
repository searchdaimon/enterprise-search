#include <stdio.h>
#include <string.h>

#include "searchFilters.h"

//fjerner også www. i begyndelsen av en url, slik at det blir lettere å samenligne www.vg.no == vg.no
int find_domain_no_subname (char url[],char domain[], int sizeofdomain) {
//ToDo har lagt til sizeofdomain, for å ungå buffer owerflow
        char *cpnt;
        char buff[64];
        char darray[10][64];
        int i,y;
        //kutter av http:// (de 7 første tegnene)
        strncpy(domain,url + 7,sizeofdomain -1);




        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                printf("bad url\n");
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';

                strncpy(buff,domain,sizeof(buff));
                i = 0;
                while ((cpnt = strrchr(buff,'.')) != NULL) {
                        //++cpnt;
                        //printf("el %s\n",cpnt);
                        strncpy(darray[i],cpnt,sizeof(darray[i]));
                        cpnt[0] = '\0';
                        ++i;
                }
                strncpy(darray[i],buff,sizeof(darray[i]));

                if (strlen(darray[1]) > 3) {
                        //printf("%s > 3\n",darray[1]);
                        domain[0] = '\0';

                        sprintf(domain,"%s%s",darray[1],darray[0]);

                        if (strncmp(domain,"www",3) == 0) {
                                strcpy(domain,domain +3);
                        }

                        //fjerner . på begyndelsen
                        strcpy(domain,domain +1);
                }
                else {
                        //returnerer domanet slik vi fant det lengere opp
                        if (strncmp(domain,"www.",4) == 0) {
                                strcpy(domain,domain +4);
                        }

                }

                return 1;
        }
}



//fjerner også www. i begyndelsen av en url, slik at det blir lettere å samenligne www.vg.no == vg.no
int find_domain_no_www (char url[],char domain[], int sizeofdomain) {

        char *cpnt;


        //kutter av http:// (de 7 første tegnene)
        strncpy(domain,url + 7,sizeofdomain -1);

	if (strncmp(domain,"www.",4) == 0) {
		strcpy(domain,domain +4);
	}

        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                //printf("bad url\n");
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}

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
