#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "../common/define.h"
#include "../common/lot.h"


int main (int argc, char *argv[]) {

	FILE *fh;
	struct stat inode;      // lager en struktur for fstat å returnere.
	char lotPath[512];
	char revindexPath[512];
	int i;

	if (argc != 3) {
		printf("usage: ./removeUnnecessaryRevindex lotNr subname\n");
		exit(1);
	}

	int lotNr = atoi(argv[1]);
	char *subname = argv[2];

	GetFilPathForLot(lotPath,lotNr,subname);

	//FILE *lotOpenFileNoCasheByLotNr(int LotNr,char resource[],char type[], char lock,char subname[]);
	if ((fh = lotOpenFileNoCasheByLotNr(lotNr,"reposetory","rb",'r',subname)) == NULL) {
		printf("dont have a reposetory\n");
		exit(1);
	}

	fstat(fileno(fh),&inode);
	//2147483648 = 2gb
	if (inode.st_size < 2147483647) {
		printf("reposetory to smal\n");
		exit(1);
	}	

	if ((fh = lotOpenFileNoCasheByLotNr(lotNr,"reposetory","rb",'r',subname)) == NULL) {
		printf("dont have a reposetory\n");
		exit(1);
	}

	//ToDo. Her BØR vi sjekke om det finnes noen gyldig iindex for lot'en
	for (i=0;i<64;i++) {
		sprintf(revindexPath,"%srevindex/Main/%i.txt",lotPath,i);
		printf("revindexPath %s\n",revindexPath);
		unlink(revindexPath);
	}
}
