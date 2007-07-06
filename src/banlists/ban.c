#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "../common/boithohome.h"


#include "../3pLibs/keyValueHash/hashtable.h"


#define ipbanfile "data/ip_bann_list.txt"

static struct hashtable *ip_h;

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

int *intdup(int i) {
	int *ret;

	ret = malloc(sizeof(int));
	(*ret) = i;

	return ret;
}
unsigned int *intudup(int i) {
	unsigned int *ret;

	ret = malloc(sizeof(unsigned int));
	(*ret) = i;

	return ret;
}

int anyLoad(char file[],struct hashtable *h, unsigned int (*pac)(char s[])) {

	FILE *fh;
	char line[512];
	unsigned int *key;
	char *value;

	if ((fh = fopen(bfile(file),"r")) == NULL) {
		perror(file);
		return 0;
	}

	
        while (fgets(line,sizeof(line),fh) != NULL) {
                chomp(line);

		if (line[0] == '#' || line[0] == '\0') {
			continue;
		}

		key =  intudup(pac(line));
		
		printf("line: \"%s\": %u , %u\n",line,(*key),pac(line));

                if (NULL == hashtable_search(h,key) ) {
                        //printf("not found!. Vil insert first");
			value = strdup(line);

                        if (! hashtable_insert(h,key,value) ) {
                                printf("cant insert!\n");
                                exit(-1);
                        }
                }
                else {
			printf("dublicat element in banlist. Elemenet line: \"%s\": %u\n",line,key);
                }
		
	}


	fclose(fh);

	return 1;
}

int isAnyBan(struct hashtable *h, unsigned int *key) {

	

        if (hashtable_search(h,key) == NULL) {
		#ifdef DEBUG
		printf("ip %u is NOT in bann lsit\n",(*key));
		#endif
		return 0;
	}
	else {
		#ifdef DEBUG
		printf("ip %u is in bann lsit\n",(*key));
		#endif
		return 1;
	}
}

int isIpBan(unsigned int ip) {
	return isAnyBan(ip_h, &ip);
}

int ipbanLoad() {

	ip_h = create_hashtable(200, ip_hashfromkey, ip_equalkeys);

	return anyLoad(ipbanfile,ip_h,ippac);
}


