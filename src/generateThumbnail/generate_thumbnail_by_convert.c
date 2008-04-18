#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include "../common/exeoc.h"

#define convertpath "/usr/bin/convert"
#define compositepath "/usr/bin/composite"

#define converttemptemplate "/tmp/generateThumbnail"

#define backgroundpng "/home/boitho/boithoTools/data/100x100.png"
#define gspath "/usr/bin/gs"

#define exeocbuflen 2048

unsigned char* generate_thumbnail_by_convert(const void *document, const size_t size, size_t *new_size, char type[]) {

	void *image;
	void *thumbnail;
	FILE *fp;
	char command[512];
	char documentfile[PATH_MAX];
	char imagefile[PATH_MAX];
	struct stat inode;      // lager en struktur for fstat å returnere.
	//int n;
	int exeocbuflenret;
	char exeocbuf[exeocbuflen];
	int ret;

	//tmpfilename = mktemp("/tmp/generateThumbnail_XXXXXX"); //make a unique temporary file name


	
	snprintf(documentfile,sizeof(documentfile),"%s-doc-%u.%s",converttemptemplate,(unsigned int)getpid(),type);
	snprintf(imagefile,sizeof(imagefile),"%s-image-%u.png",converttemptemplate,(unsigned int)getpid());
	
	if ((fp = fopen(documentfile,"wb")) == NULL) {
		printf(documentfile);
		return NULL;
	}
	fwrite(document,1,size,fp);
	fclose(fp);

	snprintf(command,sizeof(command),"%s %s -resize 98x98 -bordercolor black -border 1x1 %s",convertpath,documentfile,imagefile);

	printf("runing %s\n",command);

	//system(command);
        char *shargs[] = {"/bin/sh","-c",NULL ,'\0'};
	shargs[2] = command;
        printf("generate_thumbnail_by_convert: runnig: /bin/sh -c %s\n",command);
	exeocbuflenret = exeocbuflen;
        exeoc_timeout(shargs,exeocbuf,&exeocbuflenret,&ret,120);


	//composite -gravity center /home/boitho/public_html/div/test.png /home/boitho/boithoTools/data/100x100.png /home/boitho/public_html/div/test2.png
	snprintf(command,sizeof(command),"%s -gravity center  %s %s %s",compositepath,imagefile,backgroundpng,imagefile);
	printf("runing %s\n",command);
	//n = system(command);

	shargs[2] = command;
        printf("generate_thumbnail_by_convert: runnig: /bin/sh -c %s\n",command);
	exeocbuflenret = exeocbuflen;
        exeoc_timeout(shargs,exeocbuf,&exeocbuflenret,&ret,120);



	if ((fp = fopen(imagefile,"rb")) == NULL) {
                printf(imagefile);
                return NULL;
        }
	fstat(fileno(fp),&inode);
	image = malloc(inode.st_size);
	fread(image,1,inode.st_size,fp);

	(*new_size) = inode.st_size;

	fclose(fp);

	printf("created image om size %i\n",(*new_size));
    
	unlink(documentfile);
	unlink(imagefile);

	return image;
	
}




unsigned char* generate_pdf_thumbnail_by_convert( const void *document, const size_t size, size_t *new_size ) {

	void *image;
	void *thumbnail;
	FILE *fp;
	char command[512];
	char documentfile[PATH_MAX];
	char imagefile[PATH_MAX];
	struct stat inode;      // lager en struktur for fstat å returnere.

	int exeocbuflenret;
	char exeocbuf[exeocbuflen];
	int ret;
        char *shargs[] = {"/bin/sh","-c",command ,'\0'};

	//tmpfilename = mktemp("/tmp/generateThumbnail_XXXXXX"); //make a unique temporary file name


	
	snprintf(documentfile,sizeof(documentfile),"%s.pdf",converttemptemplate);
	snprintf(imagefile,sizeof(imagefile),"%s.png",converttemptemplate);
	
	if ((fp = fopen(documentfile,"wb")) == NULL) {
		printf(documentfile);
		return NULL;
	}
	fwrite(document,1,size,fp);
	fclose(fp);

	snprintf(command,sizeof(command),"%s -dBATCH -dFirstPage=1 -dLastPage=1 -sDEVICE=png256 -dNOPAUSE -dSAFER -sOutputFile=%s %s",gspath,imagefile,documentfile);

	printf("runing %s\n",command);
	//system(command);
	shargs[2] = command;
	exeocbuflenret = exeocbuflen;
        exeoc_timeout(shargs,exeocbuf,&exeocbuflenret,&ret,120);

	snprintf(command,sizeof(command),"%s %s -resize 98x98 -bordercolor black -border 1x1 %s",convertpath,imagefile,imagefile);

	printf("runing %s\n",command);
	//system(command);
	shargs[2] = command;
	exeocbuflenret = exeocbuflen;
        exeoc_timeout(shargs,exeocbuf,&exeocbuflenret,&ret,120);


	snprintf(command,sizeof(command),"%s -gravity center  %s %s %s",compositepath,imagefile,backgroundpng,imagefile);
	printf("runing %s\n",command);
	//system(command);
	shargs[2] = command;
	exeocbuflenret = exeocbuflen;
        exeoc_timeout(shargs,exeocbuf,&exeocbuflenret,&ret,120);


	if ((fp = fopen(imagefile,"rb")) == NULL) {
                printf(imagefile);
                return NULL;
        }
	fstat(fileno(fp),&inode);
	image = malloc(inode.st_size);
	fread(image,1,inode.st_size,fp);
	fclose(fp);

	(*new_size) = inode.st_size;

	unlink(documentfile);
	unlink(imagefile);

	return image;
}

