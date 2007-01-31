#include <stdio.h>
#include <unistd.h> //for ftruncate()
#include <sys/stat.h>
#include <sys/file.h>


#include "../common/poprank.h"
#include "../common/define.h"

#define NrOFFileBuckets 1000

int main (int argc, char *argv[]) {


	FILE *UDFILE;
	struct udfileFormat udfilePost;
	int rank;
	int bucket;
	int i;

	struct stat inode;

	struct FileBucketsFormat {
		int count;
		int OpenFile;
		FILE *FFILHANDLER;
		char Filename[255];
	};

	struct FileBucketsFormat FileBuckets[NrOFFileBuckets];

        if (argc < 2) {
                printf("Dette programet sorterer en udfil etter rank\n\n\t./SortUdfile udfil\n");
                exit(0);
        }


	for (i=0; i < NrOFFileBuckets; i++) {
		FileBuckets[i].count = 0;
		FileBuckets[i].OpenFile = 0;		
	}
        if ((UDFILE = fopen(argv[1],"r+b")) == NULL) {
                printf("Cant read udfile ");
                perror(argv[1]);
                exit(1);
        }
	flock(fileno(UDFILE),LOCK_EX);

	popopen();

	fstat(fileno(UDFILE),&inode);

	printf("udfile size: %i\n",inode.st_size);

        while(!feof(UDFILE)) {


                fread(&udfilePost,sizeof(udfilePost),1,UDFILE);


		//printf("%s\n",udfilePost.url);

		rank = popRankForDocID(udfilePost.DocID);
                

		if (rank == 0) {
			bucket = 0;		
		}
		else {
			bucket = (int)ceil(log(rank));
		}

		FileBuckets[bucket].count++;

		//tester om filen er open, hvis ikke opner vi
		if (!FileBuckets[bucket].OpenFile) {

			sprintf(FileBuckets[bucket].Filename,"/tmp/boithoSort_%i",bucket); 

			if ((FileBuckets[bucket].FFILHANDLER = fopen(FileBuckets[bucket].Filename,"w+b")) == NULL) {
	                	printf("Cant open temp file \n");
        	        	perror(FileBuckets[bucket].Filename);
                		exit(1);
			}

			FileBuckets[bucket].OpenFile = 1;
		}

		if (fwrite(&udfilePost,sizeof(udfilePost),1,FileBuckets[bucket].FFILHANDLER) != 1) {
			printf("Cant write to temp file \n");
			perror(FileBuckets[bucket].Filename);
                        exit(0);
		}

		//if (bucket != 0) {
		//	printf("DocID: %i, %i, %i\n",udfilePost.DocID,rank,bucket);
        	//}
	}
	popclose();
        
	//resetter filpekeren for udfilen slik at vi skriver til bunnanen av den
	//rewind(UDFILE);
	fflush(UDFILE);

	ftruncate(fileno(UDFILE),0);
	fseek(UDFILE,0, SEEK_SET);
	
        
	//for (i=(NrOFFileBuckets -1); i > 0; i--) {
	for(i=0; i < NrOFFileBuckets; i++) {
		if (FileBuckets[i].count != 0){ 
                	printf("%i %i\n",i,FileBuckets[i].count);
        	}

	
		if (FileBuckets[i].OpenFile) {


			fflush(FileBuckets[i].FFILHANDLER);
			//begynner fra bagyndelsen
			//rewind(FileBuckets[i].FFILHANDLER);
			fseek(FileBuckets[i].FFILHANDLER, 0, SEEK_SET);
			

			//koppierer ny data
			while(!feof(FileBuckets[i].FFILHANDLER)) {


				//printf("%s\n",udfilePost.url);
				
				fread(&udfilePost,sizeof(udfilePost),1,FileBuckets[i].FFILHANDLER);
					

				if (fwrite(&udfilePost,sizeof(udfilePost),1,UDFILE) != 1) {
                		        printf("Cant write to UDFILE file \n");
		                        perror(argv[1]);
                        		exit(0);
                		}

			}

			//lokker og fjerner opne filer
			fclose(FileBuckets[i].FFILHANDLER);
			remove(FileBuckets[i].Filename);
			
			FileBuckets[i].OpenFile = 0;
		}
	}

	fclose(UDFILE);	
}

