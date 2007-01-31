#include <stdio.h>
#include <math.h>

int main (int argc, char *argv[]) {

	int LongRank;
	unsigned char ShortRank;
	unsigned int convertetrank;
	unsigned int DocID;
	FILE *LONGFILE;
	FILE *SHORTFILE;

 	if (argc < 3) {
                printf("Dette programet konverterer en 4 byte til 1 byte rank file\nBruk\n\n\tConvertRankToShortRankFile LONGFILE SHORTFILE\n\n");
                exit(0);
        }
	
	printf("%s -> %s\n",argv[1],argv[2]);

        if ((LONGFILE = fopen(argv[1],"rb")) == NULL) {
                perror(argv[1]);
                exit(1);
        }

        if ((SHORTFILE = fopen(argv[2],"wb")) == NULL) {
                perror(argv[2]);
                exit(1);
        }

	DocID =0;	

        while (!feof(LONGFILE)) {


                if (fread(&LongRank,sizeof(LongRank),1,LONGFILE)  != 1) {
			//perror("fread");
		}

		if (LongRank != 0) {
			//log(100 000) * 50 = 250
			convertetrank = (int)(log10(LongRank) * 53);

        		if (convertetrank > 255) {
                		convertetrank = 255;
        		}
	        	ShortRank = convertetrank;
		}
		else {
			ShortRank = 0;
		}

		//printf("LongRank %i -> ShortRank %i\n",LongRank,(int)ShortRank);

		if (fwrite(&ShortRank,sizeof(ShortRank),1,SHORTFILE)  != 1) {
                        perror("fwrite");
                }


		//printf("DocID %u, Rank %i\n",DocID,ShortRank);

		DocID++;
        }



	fclose(LONGFILE);
	fclose(SHORTFILE);
}
