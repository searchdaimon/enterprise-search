#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/stat.h>
#include <sys/file.h>


#include "../common/poprank.h"
#include "../common/define.h"



int main (int argc, char *argv[]) {


	FILE *UDFILE, *VIPTXT,*PLANEFILE,*VIPFILE;
	struct udfileFormat udfilePost;
	char line[512];
	char domain[512];
	int urldepth;
	int urllen;
	int urlsubnr;

        if (argc < 7) {
                printf("Dette programet spliter en ut fil i to nye. En for de som var i vip.txt filen, og en for andre\n\n\t./SortUdfile udfile shortprefix longprefix maxdepth maxlen maxsubnames\n\n");
		printf("eks: bin/shortUrls indexes/udfile /tmp/udfile_short /tmp/udfile_long 2 50 1\n\n");
                exit(0);
        }

	char *udfile 		= argv[1];
	char *vipprefix 	= argv[2];
	char *planeprefix 	= argv[3];

	int maxdepth		= atoi(argv[4]);
	int maxlen		= atoi(argv[5]);
	int maxsubname		= atoi(argv[6]);



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



        while(fread(&udfilePost,sizeof(udfilePost),1,UDFILE) > 0) {


		urldepth = url_depth(udfilePost.url);
		urllen = strlen((char *)udfilePost.url);
		urlsubnr = url_nrOfSubDomains(udfilePost.url);
		find_domain_no_subname(udfilePost.url,domain,sizeof(domain));

		if (urldepth > maxdepth) {
			#ifdef DEBUG
			printf("BAD: maxdepth: url: \"%s\", domain \"%s\", depth %i, len %i, nrOfSubDomains %i\n",udfilePost.url,domain,urldepth,urllen,urlsubnr);
			#endif

                        if (fwrite(&udfilePost,sizeof(udfilePost),1,PLANEFILE) != 1) {
                                perror("write");
                                exit(1);
                        }

		}
		else if (urllen > maxlen) {
			#ifdef DEBUG
			printf("BAD: maxlen: url: \"%s\", domain \"%s\", depth %i, len %i, nrOfSubDomains %i\n",udfilePost.url,domain,urldepth,urllen,urlsubnr);
			#endif

                        if (fwrite(&udfilePost,sizeof(udfilePost),1,PLANEFILE) != 1) {
                                perror("write");
                                exit(1);
                        }

		}
		else if (urlsubnr > maxsubname) {
			#ifdef DEBUG
			printf("BAD: maxsubname: url: \"%s\", domain \"%s\", depth %i, len %i, nrOfSubDomains %i\n",udfilePost.url,domain,urldepth,urllen,urlsubnr);
			#endif

                        if (fwrite(&udfilePost,sizeof(udfilePost),1,PLANEFILE) != 1) {
                                perror("write");
                                exit(1);
                        }

		}
		else if (strncmp((char *)udfilePost.url,"http://www.",11) != 0) {
			#ifdef DEBUG
			printf("BAD: not www: url: \"%s\", domain \"%s\", depth %i, len %i, nrOfSubDomains %i\n",udfilePost.url,domain,urldepth,urllen,urlsubnr);
			#endif

                        if (fwrite(&udfilePost,sizeof(udfilePost),1,PLANEFILE) != 1) {
                                perror("write");
                                exit(1);
                        }
		}
		else {
			#ifdef DEBUG
			printf("GOD: url: \"%s\", domain \"%s\", depth %i, len %i, nrOfSubDomains %i\n",udfilePost.url,domain,urldepth,urllen,urlsubnr);
			#endif
                        if (fwrite(&udfilePost,sizeof(udfilePost),1,VIPFILE) != 1) {
                                perror("write");
                                exit(1);
                        }
		}

	}


	fclose(PLANEFILE);
	fclose(VIPFILE);
	fclose(UDFILE);

}

