#include <stdio.h>

int main (int argc, char *argv[]) {

	FILE *LINKDBFILE;
	FILE *POPINDEX;
	struct linkdb_block
	{
    		unsigned int        DocID_from;
    		unsigned int        DocID_to;
	};

	struct linkdb_block linkdbPost;
	double Rank;
	double RankFrom;
	double RankTo;

        if (argc < 2) {
                printf("Dette programet tar inn en linkdb fil og lager Brank\n\n\tUsage: ./BrankCalculate linkdb\n");
                exit(0);
        }

	if ((LINKDBFILE = fopen(argv[1],"rb")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }
	if ((POPINDEX = fopen(argv[2],"r+b")) == NULL) {
                printf("Cant read linkdb ");
                perror(argv[1]);
                exit(1);
        }
	while (!feof(LINKDBFILE)) {



			fread(&linkdbPost,sizeof(linkdbPost),1,LINKDBFILE);

			if ((fseek(POPINDEX,linkdbPost.DocID_from* sizeof(RankFrom),SEEK_SET) == 0) && (fread(&RankFrom,sizeof(RankFrom),1,POPINDEX) != 0)){ 
	
			}
			else {
				RankFrom = 0;
			}

			if ((fseek(POPINDEX,linkdbPost.DocID_to* sizeof(RankTo),SEEK_SET) == 0) && (fread(&RankTo,sizeof(RankTo),1,POPINDEX) != 0)){ 

                        }
                        else {
                                RankTo = 0;
                        }

			//printf("RankFrom %f, RankTo %f\n",RankFrom,RankTo);

			Rank = ((RankFrom + 0.85) / 10) + RankTo;

			fseek(POPINDEX,linkdbPost.DocID_to* sizeof(Rank),SEEK_SET);
			fwrite(&Rank,sizeof(Rank),1,POPINDEX);

			//printf("%u -> %u. R %f\n",linkdbPost.DocID_from,linkdbPost.DocID_to,Rank);


	}	
	fclose(POPINDEX);
	fclose(LINKDBFILE);
}
