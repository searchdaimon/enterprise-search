#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../bbdocument/bbdocument.h"
#include "../common/bfileutil.h"

main (int argc, char *argv[]) {

	FILE *fh;
	struct stat inode;      // lager en struktur for fstat å returnere.
	char *data;
	char *documenttype_real;
	char *documentfinishedbuf;
	int documentfinishedbufsize;

	if (argc != 2) {
		printf("usage ./%s doc\n",argv[0]);
		exit(1);
	}

	char *filname = argv[1];
	bbdocument_init();

	if ((fh = fopen(filname,"rb")) == NULL){
		perror(filname);
		exit(1);
	}
	fstat(fileno(fh),&inode);
	data = malloc(inode.st_size +1);
	fread(data,1,inode.st_size,fh);
	fclose(fh);

	if ((documenttype_real = sfindductype(filname)) == NULL) {
                        printf("can't add type because I cant decide format. File name isent dos type (8.3).\n");
                        exit(1);;
        }

	documentfinishedbufsize = ((inode.st_size *2) +512);
	documentfinishedbuf = malloc(documentfinishedbufsize);

	if (!bbdocument_convert(documenttype_real,data,inode.st_size,documentfinishedbuf,&documentfinishedbufsize,filname)) {
		printf("cant convert\n");
		exit(1);
	}
	

	printf("convertet to %ib\nDocument:\n%s\n",documentfinishedbufsize,documentfinishedbuf);

}
