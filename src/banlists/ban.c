#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "../common/boithohome.h"
#include "../common/bstr.h"
#include "../common/stdlib.h"


#include "../3pLibs/keyValueHash/hashtable.h"


#define ipbanfile "data/ip_bann_list.txt"
#define domainbanfile "data/domain_bann_list.txt"

static struct hashtable *ip_h;
static struct hashtable *domain_h;

static unsigned int ip_hashfromkey(void *ky)
{
	unsigned int k = (*(int *)ky);	
	#ifdef DEBUG
	printf("ip_hashfromkey: k %u\n",k);
	#endif
	return k;
}

static int ip_equalkeys(void *k1, void *k2)
{

	
        if ((*(unsigned int *)k1) == (*(unsigned int *)k2)) {
                return 1;
        }
        else {
		return 0;
        }
}

unsigned int ippac(char s[]) {

        struct in_addr addr;

        inet_aton(s,&addr);

        return addr.s_addr;
}


static unsigned int domain_hashfromkey(void *ky)
{
	unsigned int k = (*(int *)ky);	
	#ifdef DEBUG
	printf("domain_hashfromkey: k %u\n",k);
	#endif
	return k;
}

static int domain_equalkeys(void *k1, void *k2)
{

	return (0 == strcmp(k1,k2));
	
}


unsigned int domainpac(char s[]) {


	//runarb: weak this is
	return (s[0] * s[1] * s[2]);

}

int anyLoad(char file[],struct hashtable *h, unsigned int (*pac)(char s[])) {

	FILE *fh;
	char line[512];
	unsigned int *key;
	char *value;
	int linenr;

	if ((fh = fopen(bfile(file),"r")) == NULL) {
		perror(file);
		return 0;
	}

	linenr = 0;	
        while (fgets(line,sizeof(line),fh) != NULL) {
                chomp(line);
		++linenr;

		if (line[0] == '#' || line[0] == '\0') {
			continue;
		}

		if (strlen(line) < 4) {
			fprintf(stderr,"Error: %s line %i. Value \"%s\" isent a legal value\n",file,linenr,line);
			exit(-1);
		}

		if ((key =  intudup(pac(line))) == NULL) {
			perror("intudup");
			exit(-1);
		}
		
		#ifdef DEBUG
		printf("line: \"%s\": %u , %u\n",line,(*key),pac(line));
		#endif

                if (NULL == hashtable_search(h,key) ) {
                        //printf("not found!. Vil insert first");
			if ((value = strdup(line)) == NULL) {
				perror("strdup");
				exit(-1);
			}

                        if (! hashtable_insert(h,key,value) ) {
                                printf("cant insert!\n");
                                exit(-1);
                        }
                }
                else {
			#ifdef DEBUG
			printf("dublicat element in banlist. Elemenet line: \"%s\": %u\n",line,*key);
			#endif
			free(key);
                }
		
	}


	fclose(fh);

	return 1;
}

int isAnyBan(struct hashtable *h, unsigned int key) {


        if (hashtable_search(h,&key) == NULL) {
		#ifdef DEBUG
		printf("ip %u is NOT in bann lsit\n",key);
		#endif
		return 0;
	}
	else {
		#ifdef DEBUG
		printf("ip %u is in bann lsit\n",key);
		#endif
		return 1;
	}
}

// ip
int isIpBan(unsigned int ip) {
	return isAnyBan(ip_h, ip);
}

int ipbanLoad() {

	ip_h = create_hashtable(200, ip_hashfromkey, ip_equalkeys);

	return anyLoad(ipbanfile,ip_h,ippac);
}

void ipbanEnd() {
	hashtable_destroy(ip_h,1);
}



// domain
int isDomainBan(char domain[]) {
	return isAnyBan(domain_h, domainpac(domain));
}

int domainLoad() {

	domain_h = create_hashtable(200, domain_hashfromkey, domain_equalkeys);

	return anyLoad(domainbanfile,domain_h,domainpac);
}

void domainbanEnd() {
	hashtable_destroy(domain_h,1);
}

