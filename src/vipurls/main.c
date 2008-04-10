#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/stat.h>
#include <sys/file.h>

#include "../3pLibs/keyValueHash/hashtable.h"

#include "../common/poprank.h"
#include "../common/define.h"

static unsigned int fileshashfromkey(void *ky)
{
    	char *k = (char *)ky;
	int i = 0;
	unsigned int val = 0;

	while (k[i] != '\0') {
		val += (unsigned int)k[i];
		++i;
	}
	//printf("hash: s \"%s\", h %u\n",k,val);
        return val;
}

static int filesequalkeys(void *k1, void *k2)
{
    return (0 == strcmp(k1,k2));
}


int main (int argc, char *argv[]) {


	FILE *UDFILE, *VIPTXT,*PLANEFILE,*VIPFILE;
	struct udfileFormat udfilePost;
	char line[512];
	char domain[512];

	char *filesKey;
        int *filesValue;
        struct hashtable *h;

        if (argc < 5) {
                printf("Dette programet spliter en ut fil i to nye. En for de som var i vip.txt filen, og en for andre\n\n\t./SortUdfile udfile vip.txt vipprefix planeprefix\n");
                exit(0);
        }

	char *udfile 		= argv[1];
	char *viptxt 		= argv[2];
	char *vipprefix 	= argv[3];
	char *planeprefix 	= argv[4];

	//laster vip
        if ((VIPTXT = fopen(viptxt,"rb")) == NULL) {
                printf("Cant read udfile ");
                perror(udfile);
                exit(1);
        }


	h = create_hashtable(200, fileshashfromkey, filesequalkeys);

	while (fgets(line,sizeof(line),VIPTXT) != NULL) {
		chomp(line);

		url_normalization(line);

		find_domain_no_subname(line,domain,sizeof(domain));

		#ifdef DEBUG
		printf("line: \"%s\", domain: \"%s\"\n",line,domain);
		#endif

		filesValue = malloc(sizeof(int));
                (*filesValue) = 1;
                filesKey = strdup(domain);

		if (! hashtable_insert(h,filesKey,filesValue) ) {
                        printf("cant insert\n");
                	//exit(-1);
                }
	}

	fclose(VIPTXT);	

	//åpner filer
        if ((UDFILE = fopen(udfile,"rb")) == NULL) {
                printf("Cant read udfile ");
                perror(udfile);
                exit(1);
        }
	flock(fileno(UDFILE),LOCK_EX);


        if ((VIPFILE = fopen(vipprefix,"wb")) == NULL) {
                perror(vipprefix);
                exit(1);
        }

        if ((PLANEFILE = fopen(planeprefix,"wb")) == NULL) {
                perror(planeprefix);
                exit(1);
        }



        while(!feof(UDFILE)) {


                fread(&udfilePost,sizeof(udfilePost),1,UDFILE);


		find_domain_no_subname(udfilePost.url,domain,sizeof(domain));


		if (NULL != (filesValue = hashtable_search(h,domain) )) {
			//vip
			//printf("vip: %s\n",udfilePost.url);
	                if (fwrite(&udfilePost,sizeof(udfilePost),1,VIPFILE) != 1) {
				perror("write");
				exit(1);
			}

			
			++(*filesValue);
		}
		else {
			//plane
	                if (fwrite(&udfilePost,sizeof(udfilePost),1,PLANEFILE) != 1) {
				perror("write");
				exit(1);
			}


		}

	}

        hashtable_destroy(h,1);

	fclose(PLANEFILE);
	fclose(VIPFILE);
	fclose(UDFILE);

}

