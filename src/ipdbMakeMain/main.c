#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>

#include "../common/define.h"
#include "../common/reposetoryNET.h"


#define subname "www"

int main (int argc, char *argv[]) {

	int i;
	int lotStart,lotEnd;
	FILE *TMPTRANSFER;
	off_t offset;
	FILE *IPDBFH;
	unsigned int IPadress;
	unsigned int DocID;
	char command[128];

        if (argc < 4) {
                printf("Usage./ipdbMakeMain lotStart lotEnd ipdb\n\n");
               exit(0);
        }

        lotStart = atoi(argv[1]);
        lotEnd = atoi(argv[2]);

	TMPTRANSFER = tmpfile();

	if ((IPDBFH = fopen(argv[3],"wb")) == NULL) {
		perror(argv[3]);
		exit(1);
	}
	printf("opend ipdb %s\n",argv[3]);

	for(i=lotStart;i<(lotEnd +1);i++) {
		printf("get ipdb for %i\n",i);


		//ber om at ipdb skal lages
		sprintf(command,"/home/boitho/boithoTools/bin/ipdbBuildLotIndex %i",i);
		rComand(command,i,subname);

		//int rGetFileByOpenHandler(char source[],FILE *FILEHANDLER,int LotNr,char subname[]);
		rGetFileByOpenHandler("ipdb",TMPTRANSFER,i,subname);
		printf("rGetFileByOpenHandler end\n");
		//resetter
		fseek(TMPTRANSFER,0,SEEK_SET);
		

		//finer filofset
		int lottoffset = LotDocIDOfset(i);
		DocID = lottoffset;
	
		//if (lottoffset == 0) {
		//	offset = 0;
		//}
		//else {
			offset = ((lottoffset * sizeof(IPadress)));
		//}

		if (fseek(IPDBFH,offset,SEEK_SET) != 0) {
			perror("fseek");
			printf("cant seek to %"PRId64"\n",offset);
		}
		//printf("offset: %" PRId64 ", ftell: %ul\n",offset,ftell(IPDBFH));
		
		//kopierer over ipadresser
		while(!feof(TMPTRANSFER)) {

			fread(&IPadress,sizeof(IPadress),1,TMPTRANSFER);

			//printf("%u %u\n",DocID,IPadress);

			if (fwrite(&IPadress,sizeof(IPadress),1,IPDBFH) != 1) {
				perror("write");
			}

			++DocID;
		}

	}

	//printf("end write, ftel %ul\n",ftell(IPDBFH));

	fclose(IPDBFH);


	fclose(TMPTRANSFER);
}
