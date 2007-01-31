#include "../common/poprank.h"

#define Rank_intern_max 10
#define Rank_noc_max 10

int main (int argc, char *argv[]) {

        struct popl popindex;
	FILE *IPDB;
	unsigned long int IPAddress;
	unsigned int DocID;

	if ((IPDB = fopen("/home/boitho/config/ipdb","r")) == NULL) {
		perror("");
		exit(1);
	}

        popopen (&popindex,"/home/boitho/config/popindex");
	DocID = 0;

	while(!feof(IPDB)) {

		fread(&IPAddress,sizeof(IPAddress),1,IPDB);


		if (IPAddress == 2419066176) {
			printf("%lu: %lu\n",DocID,IPAddress);
			popset(&popindex,DocID,0);
		}
		++DocID;
	}


	popclose(&popindex);
	fclose(IPDB);
}
