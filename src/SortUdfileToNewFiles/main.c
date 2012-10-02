#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

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

	struct popl popindex;

	struct FileBucketsFormat {
		int count;
		int OpenFile;
		FILE *FFILHANDLER;
		char Filename[255];
	};

	struct FileBucketsFormat FileBuckets[NrOFFileBuckets];

        if (argc < 3) {
                printf("Dette programet sorterer en udfil etter rank og lagrer de i nye udfiler\n\n\t./SortUdfile udfile newprefix\n");
                exit(0);
        }

	char *udfile = argv[1];
	char *prefix = argv[2];


	for (i=0; i < NrOFFileBuckets; i++) {
		FileBuckets[i].count = 0;
		FileBuckets[i].OpenFile = 0;		
	}
        if ((UDFILE = fopen(udfile,"rb")) == NULL) {
                printf("Cant read udfile ");
                perror(udfile);
                exit(1);
        }
	flock(fileno(UDFILE),LOCK_EX);

	//popopen();
	popopen(&popindex,"/home/boitho/config/popindex");

	fstat(fileno(UDFILE),&inode);

	printf("udfile size: %" PRId64 "\n",inode.st_size);

        //while(!feof(UDFILE)) {
        while(fread(&udfilePost,sizeof(udfilePost),1,UDFILE) == 1) {


		/*
                if (fread(&udfilePost,sizeof(udfilePost),1,UDFILE) != 1) {
			printf("can't read more data\n");
		}
		*/

		//printf("%s\n",udfilePost.url);

		rank = popRankForDocID(&popindex,udfilePost.DocID);

		#ifdef DEBUG
		printf("DocID %u, rank %i\n",udfilePost.DocID,rank);
                #endif

		if (rank < 0) {
			#ifdef DEBUG
			fprintf(stderr,"can't read pop rank. Error nr %i. Perror : ",rank);
			perror("");
			#endif
			rank = 0;
		}
		else if (rank == 0) {
			bucket = 0;		
		}
		else {
			bucket = (int)ceil(log(rank));
		}

		if (bucket < 0) {
			printf("bucket smalet then 0. bucket %i, rank %i\n",bucket,rank);
		}
		else if (bucket > NrOFFileBuckets) {
			printf("Above NrOFFileBuckets (%i). bucket was %i\n",NrOFFileBuckets,bucket);
		}

		FileBuckets[bucket].count++;

		//tester om filen er open, hvis ikke opner vi
		if (!FileBuckets[bucket].OpenFile) {

			sprintf(FileBuckets[bucket].Filename,"%s%i",prefix,bucket); 

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


	if (!feof(UDFILE)) {
		printf("Error: dident manage to read mor data, but ient at eof.\n");
		perror("udfile");
	}

	popclose(&popindex);


	
        
	//for (i=(NrOFFileBuckets -1); i > 0; i--) {
	for(i=0; i < NrOFFileBuckets; i++) {
		if (FileBuckets[i].count != 0){ 
                	printf("%s%i %i\n",prefix,i,FileBuckets[i].count);
        	}
	
		//if (FileBuckets[i].OpenFile) {
		//
		//}
	}

        
}

