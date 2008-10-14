#ifdef BLACK_BOKS

#include <db.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>


#include "../common/define.h"
#include "../common/debug.h"
#include "../common/exeoc.h"
#include "../common/boithohome.h"
#include "../common/ht.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

#include "../common/reposetory.h"
#include "../common/bstr.h"
#include "../acls/acls.h"

#include "bbdocument.h"

#include "../common/bfileutil.h"
#include "../common/lot.h"

//#include "../common/chtbl.h"
#define PRIME_TBLSIZ 100

#include "../generateThumbnail/generate_thumbnail.h"
#include "../generateThumbnail/generate_thumbnail_by_convert.h"

#define filconvertetfile "/tmp/boithofilconverter"

#define html_tempelate "<html>\n<head>\n<title>%s</title>\n<meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1'>\n</head>\n<body>\n%s\n</body>\n</html>\n"

//muligens bare convert:
// ai
//bugger "psd"
char *supportetimages[] = {"png", "jpg", "jepg", "bmp", "tif", "tiff", "gif", "eps", "ai", '\0'};



//globals
//CHTbl htbl;
struct hashtable *h_fileFilter;

struct fileFilterFormat {
	char documentstype[12];
	char command[512];
	char outputtype[12];
	char outputformat[12];
	char comment[64];
	char format[12];
};

struct uriindexFormat {
        unsigned int DocID;
        unsigned int lastmodified;
};



static unsigned int bbdocument_h( void *key) {

        int ir;
        struct fileFilterFormat *fileFilter  = (struct fileFilterFormat *) key;
        ir = ((*fileFilter).documentstype[0] * (*fileFilter).documentstype[1] * (*fileFilter).documentstype[2]);

        return ir;
}
static int bbdocument_hmatch(void *key1, void *key2) {

        struct fileFilterFormat *fileFilter1  = (struct fileFilterFormat *) key1;
        struct fileFilterFormat *fileFilter2  = (struct fileFilterFormat *) key2;


        if (strcmp((*fileFilter1).documentstype,(*fileFilter2).documentstype) == 0) {
                return 1;
        }
        else {
                return 0;
        }

}





int canconvert(char have[]) {
        int i;

        i =0;
        while(supportetimages[i] != NULL) {
                if (strcmp(supportetimages[i],have) == 0) {
                        return 1;
                }
                ++i;
        }

        return 0;
}

int bbdocument_makethumb( char documenttype[],char document[],size_t dokument_size,char **imagebuffer,size_t *imageSize) {

	#ifdef BBDOCUMENT_IMAGE

	if (strcmp(documenttype,"pdf") == 0) {
		//pdf convert
		if (((*imagebuffer) = generate_pdf_thumbnail( document, dokument_size, imageSize )) == NULL) {
			return 0;

		}
			return 1;
		return 0;
	}
	else if (canconvert(documenttype) && (((*imagebuffer) = generate_thumbnail( document, dokument_size, imageSize )) == NULL)) {
		return 0;
	}

	#else
	#if BBDOCUMENT_IMAGE_BY_CONVERT
	if (strcmp(documenttype,"pdf") == 0) {
		//pdf convert
/*
		if (((*imagebuffer) = generate_pdf_thumbnail_by_convert( document, dokument_size, imageSize )) == NULL) {
			return 0;

		}
			return 1;
*/	
		return 0;
	}
	else if (canconvert(documenttype)) {

		if ((((*imagebuffer) = generate_thumbnail_by_convert( document, dokument_size, imageSize, documenttype)) == NULL )) {
			printf("error: cant run generate_thumbnail_by_convert\n");
			return 0;
		}
		else {
			return 1;
		}
	}
	#endif
	#endif
	
	//er bare her hvis vi ikke klarte og lage bilde, og da har vi selsakt ikke noe størelse
	//printf("imageSize %"PRId64" at %s:%i\n",(*imageSize),__FILE__,__LINE__);
	return 0;

}

int bbdocument_freethumb(char *imagebuffer) {
	#ifdef BBDOCUMENT_IMAGE
	imagebuffer = free_thumbnail_memory( imagebuffer );
	#endif
}

int bbdocument_init() {

	DIR *dirp;
	FILE *filep;
	char buf[512];
	char path[512];
	struct dirent *dp;
	char lines[512];
	char **splitdata;
	int TokCount;
	struct fileFilterFormat *fileFilter = NULL;

	char fileFilterName[] = "fileFilter";


	//chtbl_init(&htbl, PRIME_TBLSIZ, bbdocument_h, bbdocument_hmatch, free);
	h_fileFilter = create_hashtable(PRIME_TBLSIZ, bbdocument_h, bbdocument_hmatch);

	printf("opening %s\n",bfile(fileFilterName));
	if ((dirp = opendir(bfile(fileFilterName))) == NULL) {
		fprintf(stderr,"warn: cant open fileFilter \"%s\". Cant use fileFilters\n",bfile(fileFilterName));
		return 1;
	}  
	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.') {
			continue;
		}
		sprintf(path,"%s/%s/",bfile(fileFilterName),dp->d_name);
		sprintf(buf,"%sruninfo",path);
		printf("%s\n",buf);
		if ((filep = fopen(buf,"r")) == NULL) {
			printf("no runinfo file for \"%s\"\n",dp->d_name);	
			continue;
		}
		
		printf("loading \"%s\"\n",dp->d_name);


		while ((!feof(filep)) && (fgets(lines,sizeof(lines) -1,filep) != NULL)) {
			

			//blanke linjer og komentarer som starter på #
			if ((lines[0] == '\n') || (lines[0] == '#')) {
				continue;
			}

			//void chomp(char string[])
			chomp(lines);

			//printf("line %s\n",lines);
			TokCount = split(lines, ": ", &splitdata);			
			//printf("\tfound %d token(s):\n", TokCount);

			/*
			if (TokCount != 2) {
				printf("bad config line \"%s\". Splitet in %i elements\n",lines,TokCount);
				continue;
			}
			*/

			if (strcmp(splitdata[0],"documentstype") == 0) {
				//nyt filter
				if (fileFilter != NULL) {
					//add to hash
					printf("inserting %s\n",(*fileFilter).documentstype);
					//chtbl_insert(&htbl,(void *)fileFilter);
					if (!hashtable_insert(h_fileFilter,fileFilter->documentstype,fileFilter) ) {
                        		        printf("cant insert\n");
                		        	exit(-1);
		                        }

					printf("end inserting\n");
				}
				fileFilter = malloc(sizeof(struct fileFilterFormat));

				//ikke alle filfiltere har sat alle opsjoner, så vi nulstiller alt, slik at det er lett og strcmp()'e
				//etter en verdi, uten at vi må tenke på at den kansje ikke er satt.
				memset(fileFilter,'\0',sizeof(struct fileFilterFormat));

				strcpy((*fileFilter).documentstype,splitdata[1]);

			}
			else if (strcmp(splitdata[0],"command") == 0) {
				//vi kan ha : i komandoen. Kopierer derfor først inn hele, så fjerner vi command:
				//strcpy((*fileFilter).command,splitdata[1]);
			
				strscpy((*fileFilter).command,lines,sizeof((*fileFilter).command));
				strcasesandr((*fileFilter).command,sizeof((*fileFilter).command),"command: ","");
				//leger til path der vi har sakt vi skal ha lokal path ( ./ )
				strcasesandr((*fileFilter).command,sizeof((*fileFilter).command),"./",path);
				printf(".command %s\n",(*fileFilter).command);
			}
			else if (strcmp(splitdata[0],"comment") == 0) {
				strscpy((*fileFilter).comment,splitdata[1],sizeof((*fileFilter).comment));
			}
			else if (strcmp(splitdata[0],"format") == 0) {
				strscpy((*fileFilter).format,splitdata[1],sizeof((*fileFilter).format));
			}
			else if (strcmp(splitdata[0],"outputtype") == 0) {
				//stdio, file, osv,,
				strcpy((*fileFilter).outputtype,splitdata[1]);
			}
			else if (strcmp(splitdata[0],"outputformat") == 0) {
				//text, html
				strcpy((*fileFilter).outputformat,splitdata[1]);
			}
			else {
				printf("unknown command \"%s\"\n",lines);
			}

			//clean
			FreeSplitList(splitdata);

		}


		if (fileFilter != NULL) {
			//add to hash
			printf("inserting %s\n",(*fileFilter).documentstype);
			//chtbl_insert(&htbl,(void *)fileFilter);
			if (!hashtable_insert(h_fileFilter,fileFilter->documentstype,fileFilter) ) {
                                printf("cant insert\n");
                        	exit(-1);
                        }
			printf("end inserting\n");
		}
		fclose(filep);
	}
	closedir(dirp);
}

int bbdocument_delete (char uri[], char subname[]) {
	
	uriindex_delete (uri, subname);

	return 1;
}

int bbdocument_exist(char subname[],char documenturi[],unsigned int lastmodified) {
	docid DocID;
	unsigned int lastmodifiedForExistTest;

        debug("bbadocument_exist: %s, \"%s\", lastmodified %u",subname,documenturi,lastmodified);

	if (uriindex_get(documenturi,&DocID,&lastmodifiedForExistTest,subname)
	    && (((lastmodifiedForExistTest == lastmodified)
	        || (lastmodified == 0)))) {
		struct DocumentIndexFormat docindex;
		debug("bbdocument_exist: Uri \"%s\" exists with DocID \"%u\" and time \"%u\"\n",
		    documenturi, DocID, lastmodifiedForExistTest);
		// Update DI with new existed timestamp

#if defined(BLACK_BOKS) && !defined(_24SEVENOFFICE)
		DIRead(&docindex, DocID, subname);
		docindex.lastSeen = time(NULL);
		DIWrite(&docindex, DocID, subname, NULL);
		debug("bbdocument_exist: satte lastSeen \"%s\"",ctime(&docindex.lastSeen));
#endif
		return 1;
	}

	debug("bbdocument_exist: dokument dosent exist");

	return 0;
}

char *acl_normalize(char *acl[]) {
	int count;

	char *cal_buf = malloc(512);
	cal_buf[0] = '\0';

        count = 0;
        while( (acl[count] != NULL) ) {
                printf("bbdocument_add: acl: %s\n",acl[count]);
		strcat(cal_buf,acl[count]);
		strcat(cal_buf,",");
		++count;
        }

	if (count != 0) {
		cal_buf[strlen(cal_buf) -1] = '\0';
	}

	return cal_buf;

}
 
//stripper < og > tegn, da html parseren vil tro det er html tagger.
//de er jo som kjent på formater < og >
void stripTags(char *cpbuf, int cplength) {

	int i;

	for (i=0;i<cplength;i++) {
		if ((cpbuf[i] == '<') || (cpbuf[i] == '>')) {
			cpbuf[i] = ' ';
		}
	}
}

int bbdocument_convert(char filetype[],char document[],const int dokument_size,char **documentfinishedbuf,int *documentfinishedbufsize, const char titlefromadd[], char *subname, char *documenturi, unsigned int lastmodified, char *acl_allow, char *acl_denied, char *doctype, struct hashtable **metahash) {

	char **splitdata;
        int TokCount;
	FILE *fp;
	char filconvertetfile_real[216];
	char filconvertetfile_out_txt[216];
	char filconvertetfile_out_html[216];
	int exeocbuflen;
	int i;
	int ret;
	char *documentfinishedbuftmp;
	char fileconverttemplate[1024];

	printf("bbdocument_convert: dokument_size %i, title \"%s\",filetype \"%s\"\n",dokument_size,titlefromadd,filetype);

	//konverterer filnavn til liten case
	for (i=0;i < strlen(filetype);i++) {
		//printf("%c\n",filetype[i]);
		filetype[i] = btolower(filetype[i]);
	}

	#ifdef DEBUG
	printf("documentfinishedbufsize is %i\n",(*documentfinishedbufsize));
	#endif

	(*documentfinishedbufsize) = (dokument_size * 2) + 512;
	*documentfinishedbuf = malloc((*documentfinishedbufsize));
	documentfinishedbuftmp = *documentfinishedbuf;

	//hvis vi har et html dokument kan vi bruke dette direkte
	//er dog noe uefektist her, ved at vi gjør minnekopiering
	if ((strcmp(filetype,"htm") == 0) || (strcmp(filetype,"html") == 0 )) {
		memcpy(documentfinishedbuftmp,document,dokument_size);
                (*documentfinishedbufsize) = dokument_size;
		documentfinishedbuftmp[(*documentfinishedbufsize) +1] = '\0';
		return 1;
	}
	else if (strcmp(filetype,"hnxt") == 0) {
               
		ntobr(document,dokument_size);
 
		snprintf(documentfinishedbuftmp,(*documentfinishedbufsize),html_tempelate,titlefromadd,document);

                (*documentfinishedbufsize) = strlen(documentfinishedbuftmp);
		return 1;
	}

	#ifdef DEBUG
	printf("strcmp done\n");
	#endif


	struct fileFilterFormat *fileFilter = malloc(sizeof(struct fileFilterFormat));
	struct fileFilterFormat *fileFilterOrginal;

	//strcpy((*fileFilter).documentstype,filetype);
	
	//if (chtbl_lookup(&htbl,(void *)&fileFilterOrginal) != 0) { 
	if (NULL == (fileFilterOrginal = hashtable_search(h_fileFilter,filetype) )) {
		printf("don't have converter for \"%s\"\n",filetype);
		(*documentfinishedbufsize) = 0;

		//#ifdef DEBUG
		printf("writing to unknownfiltype.log\n");
		if ((fp = fopen(bfile("logs/unknownfiltype.log"),"ab")) == NULL) {
			perror(bfile("logs/unknownfiltype.log"));
		}
		else {
			printf("title %s\n",titlefromadd);
			printf("filetype %s\n",filetype);
			fprintf(fp,"%s: %s\n",titlefromadd,filetype);
			fclose(fp);
		}	
		printf("writing to unknownfiltype.log. done\n");
		//#endif

		return 0;
	}
	else {
		memcpy(fileFilter, fileFilterOrginal, sizeof(struct fileFilterFormat));

		#ifdef DEBUG
			printf("have converter for file type\n");
		#endif
	}

	//hvis dette er en fil av type text trenger vi ikke og konvertere den.
	if (strcmp((*fileFilter).format,"text") == 0) {
		#ifdef DEBUG
			printf("fileFilter ses it is a file of format text. Can use it direktly\n");
		#endif

		char *cpbuf;
		int cpbufsize;

		//konvertere alle \n til <br>
		cpbufsize = (dokument_size + 512 +1);
		cpbuf = malloc(cpbufsize);

		memcpy(cpbuf,document,dokument_size);
		cpbuf[dokument_size] = '\0';

		//stripper < og > tegn, da html parseren vil tro det er html tagger.
		//de er jo som kjent på formater < og >
		stripTags(cpbuf,dokument_size);

		printf("document %i\n",strlen(document));
		printf("documentfinishedbuf %i\n",(*documentfinishedbufsize));

		snprintf(documentfinishedbuftmp,(*documentfinishedbufsize),html_tempelate,titlefromadd,cpbuf);
                (*documentfinishedbufsize) = strlen(documentfinishedbuftmp);

		free(cpbuf);

                return 1;
	}

	/*****************************************************************************
		Vi har konverter. Må skrive til fil får å kunne sende den med
	*****************************************************************************/

	pid_t pid = getpid();

	sprintf(fileconverttemplate, "%s-%d", filconvertetfile, rand());
	sprintf(filconvertetfile_real,"%s-%u.%s",fileconverttemplate, (unsigned int)pid,filetype);
	sprintf(filconvertetfile_out_txt,"%s-%u.txt",fileconverttemplate, (unsigned int)pid);
	sprintf(filconvertetfile_out_html,"%s-%u.html",fileconverttemplate, (unsigned int)pid);

	#ifdef DEBUG
	printf("bbdocument_convert: filconvertetfile_real \"%s\"\n",filconvertetfile_real);
	#endif
	if ((fp = fopen(filconvertetfile_real,"wb")) == NULL) {
		perror(filconvertetfile_real);
		exit(1);
	}
	fwrite(document,1,dokument_size,fp);
	fclose(fp);
	//convert to text.
	/*****************************************************************************/


	#ifdef DEBUG
	printf("command: %s\n",(*fileFilter).command);
	#endif

	strsandr((*fileFilter).command,"#file",filconvertetfile_real);
	strsandr((*fileFilter).command,"#outtxtfile",filconvertetfile_out_txt);
	strsandr((*fileFilter).command,"#outhtmlfile",filconvertetfile_out_html);


	//hvis vi skal lage en ny fil må vi slette den gamle
	//sletter den etterpå i steden. Men før vi kaller return
	//if (strcmp((*fileFilter).outputformat,"textfile") == 0) {
	//	unlink(filconvertetfile_out_txt);
	//}

	//her parser vi argumenter selv, og hver space blir en ny argyment, selv om vi 
	//bruker "a b", som ikke riktig blir to argumenter her, a og b
	//splitter på space får å lage en argc
	TokCount = split((*fileFilter).command, " ", &splitdata);
	//#ifdef DEBUG
	printf("splitet comand in %i, program is \"%s\"\n",TokCount,splitdata[0]);
	//#endif
	printf("running: %s\n",(*fileFilter).command);
	//sender med størelsen på buferen nå. Vil få størelsen på hva vi leste tilbake
	exeocbuflen = (*documentfinishedbufsize);


	
	char envpairtemplate[] = "tmp/converter-metadata-XXXXXX";
	char *envpairpath = strdup(bfile(envpairtemplate));
	char envpair[PATH_MAX];
	mktemp(envpairpath);
	sprintf(envpair, "SDMETAFILE=%s", envpairpath);
	free(envpairpath);
	envpairpath = envpair + strlen("SDMETAFILE=");
        char *shargs[] = { "/usr/bin/env", NULL, "/bin/sh", "-c", NULL, NULL, };
	shargs[1] = envpair;
        shargs[4] = fileFilter->command;

	if (!exeoc_timeout(shargs,documentfinishedbuftmp,&exeocbuflen,&ret,120)) {

		printf("dident get any data from exeoc. But can be a filter that creates files, sow we wil continue\n");
		//kan ikke sette den til 0 da vi bruker den får å vite hvos stor bufferen er lengere nede
		//(*documentfinishedbufsize) = 0;
		documentfinishedbuftmp[0] = '\0';
		//return 0;

	}

	if (metahash) {
		FILE *metafp;

		*metahash = create_hashtable(3, ht_stringhash, ht_stringcmp);

		if ((metafp = fopen(envpairpath, "r")) != NULL) {
			char *key, *value, line[2048];

			printf("Fooop\n");
			while (fgets(line, sizeof(line), metafp)) {
				char *p, *p2;

				/* Comment */
				if (line[0] == '#')
					continue;

				key = line;
				p = strchr(key, '=');
				if (p == NULL) {
					fprintf(stderr, "Invalid format on meta spec file: %s\n", line);
					continue;
				}
				p2 = p;
				while (isspace(*(p2-1)))
					p2--;
				*p2 = '\0';
				p++; /* Skip past = */
				while (isspace(*p))
					p++;
				value = p;
				while (isspace(*key))
					key++;

				if (value[strlen(value)-1] == '\n')
					value[strlen(value)-1] = '\0';
				printf("Got pair: %s = %s\n", key, value);
				hashtable_insert(*metahash, strdup(key), strdup(value));
			}
			fclose(metafp);
			unlink(envpairpath);
		} else {
			printf("Couldn't open %s\n", envpairpath);
		}
	}

#ifdef DEBUG
	printf("did convert to %i bytes (strlen %i)\n",exeocbuflen,strlen(documentfinishedbuftmp));
#endif

	if (strcmp((*fileFilter).outputformat,"text") == 0) {
		//hvis dette er text skal det inn i et html dokument.
		//må ha med subjekt lengde her også

		int cpbufsize;
		char *cpbuf;

		cpbufsize = (strlen(html_tempelate) + exeocbuflen + strlen(titlefromadd)+1);	
		cpbuf = malloc(cpbufsize);
		//runarb: 30 jan 2008: hvorfor bruker vi documentfinished her, når det er documentfinishedbuf som returneres fra exeoc_timeout() ?
		//snprintf(cpbuf,cpbufsize,html_tempelate,titlefromadd,documentfinishedbuf);
		snprintf(cpbuf,cpbufsize,html_tempelate,titlefromadd,documentfinishedbuftmp);
		strscpy(documentfinishedbuftmp,cpbuf,(*documentfinishedbufsize));
		(*documentfinishedbufsize) = strlen(documentfinishedbuftmp);


		#ifdef DEBUG	
		printf("maked html document of %i b (html_tempelate is %i b, cpbuf %i)\n",strlen(documentfinishedbuftmp),strlen(html_tempelate),strlen(cpbuf));
		#endif

		free(cpbuf);


	}
	else if (strcmp((*fileFilter).outputformat,"html") == 0) {
		//html trenger ikke å konvertere
		//dette er altså outputformat html. Ikke filtype outputformat. Filtupe hondteres lengere oppe
		//ToDo: må vel kopiere inn noe data her???
	}
	else if (strcmp((*fileFilter).outputformat,"textfile") == 0) {
		FILE *fh;
		struct stat inode; 
		char *cpbuf;
		printf("filconvertetfile_out_txt: \"%s\"\n",filconvertetfile_out_txt);

		if ((fh = fopen(filconvertetfile_out_txt,"rb")) == NULL) {
			printf("can't open out file \"%s\"\n",filconvertetfile_out_txt);
			perror(filconvertetfile_out_txt);
			(*documentfinishedbufsize) = 0;
			free(fileFilter);
			unlink(filconvertetfile_real);
			unlink(filconvertetfile_out_txt);
			unlink(filconvertetfile_out_html);

			return 0;
		}		
       		fstat(fileno(fh),&inode);


                if ((cpbuf = malloc(inode.st_size +1)) == NULL) {
			perror("malloc");
			free(fileFilter);
			return 0;
		}
                
        	fread(cpbuf,1,inode.st_size,fh);
		cpbuf[inode.st_size] = '\0';

		printf("did read back %i bytes from file \"%s\"\n",(int)inode.st_size,filconvertetfile_out_txt);

		printf("strlen cpbuf: %i\n",strlen(cpbuf));

		fclose(fh);

		printf("have size %i\n",(*documentfinishedbufsize));

		snprintf(documentfinishedbuftmp,(*documentfinishedbufsize),html_tempelate,titlefromadd,cpbuf);
                (*documentfinishedbufsize) = strlen(documentfinishedbuftmp);

		printf("documentfinishedbufsize: %i\n",(*documentfinishedbufsize));

		free(cpbuf);

		//seltter filen vi lagde
		unlink(filconvertetfile_out_txt);

	}
	else if (strcmp((*fileFilter).outputformat,"htmlfile") == 0) {
		FILE *fh;
		struct stat inode; 
		if ((fh = fopen(filconvertetfile_out_html,"rb")) == NULL) {
			printf("can't open out file \"%s\"\n",filconvertetfile_out_html);
			perror(filconvertetfile_out_html);
			(*documentfinishedbufsize) = 0;
			unlink(filconvertetfile_real);
			unlink(filconvertetfile_out_txt);
			unlink(filconvertetfile_out_html);

			return 0;
		}		
       		fstat(fileno(fh),&inode);
		if ((*documentfinishedbufsize) > inode.st_size) {
			(*documentfinishedbufsize) = inode.st_size;
		}
        	fread(documentfinishedbuftmp,1,(*documentfinishedbufsize),fh);

		fclose(fh);
	}
	else if (strcmp(fileFilter->outputformat, "dir") == 0 || strcmp(fileFilter->outputformat, "diradd") == 0) {
		char *p;
		/* Does len do anything any more? */
		int len, failed = 0;
		int iter = 0;
		char *curdocp = documentfinishedbuftmp;
		int type; /* 1 for dir, 2 for diradd */

		type = (strcmp(fileFilter->outputformat, "dir") == 0) ? 1 : 2;

		len = exeocbuflen;
		p = strdup(documentfinishedbuftmp);
		if (p == NULL) {
			free(fileFilter);
			unlink(filconvertetfile_real);
			unlink(filconvertetfile_out_txt);
			unlink(filconvertetfile_out_html);

			return 0;
		}
		sprintf(*documentfinishedbuf, html_tempelate, titlefromadd, "");
		curdocp += strlen(*documentfinishedbuf);
		while (*p != '\0') {
			char *ft, *path;
			char *part = NULL;

			ft = p;
			for (; *p != ' '; p++)
				len--;
			*p = '\0';
			if (type == 2) {
				part = ++p;
				for (; *p != ' '; p++)
					len--;
				*p = '\0';
			}
			path = ++p;
			/* XXX: strchr() */
			for (; *p != '\n'; p++)
				len--;

			if (*p == '\n')
				*p++ = '\0';

			curdocp[0] = '\0';
			/* We have a new file, let's get to work on it */
			//printf("########Got: %s: %s\n", ft, path);
			{
				char *docbuf;
				char *convdocbuf;
				int docbufsize, convdocbufsize;
				struct stat st;
				int n;
				char buf[1024];
				FILE *fp;

				if (stat(path, &st) == -1) { /* Unable to access file, move on to the next */
					fprintf(stderr, "File: %s\n", path);
					perror("stat");
					failed++;
					continue;
				}

				docbuf = malloc(st.st_size + 1); /* Make room for our lovely '\0' */
				if (docbuf == NULL) {
					perror("malloc");
					failed++;
					free(docbuf);
					continue;
				}
				docbufsize = st.st_size;
				if ((fp = fopen(path, "r")) == NULL) {
					perror("fopen");
					failed++;
					free(docbuf);
					continue;
				}
				fread(docbuf, 1, docbufsize, fp);
				fclose(fp);
				unlink(path);
				docbuf[docbufsize] = '\0';
				convdocbufsize = 0;

				if (type == 1)  {
					while (((curdocp - documentfinishedbuftmp) + (char *)docbufsize) >
					    (char *)documentfinishedbufsize) {
						char *oldptr = *documentfinishedbuf;
						
						*documentfinishedbufsize *= 2;
						*documentfinishedbuf = realloc(*documentfinishedbuf, *documentfinishedbufsize);
						curdocp = *documentfinishedbuf + (curdocp - oldptr);
						documentfinishedbuftmp = *documentfinishedbuf;
					}
				}
				//runarb: 18 jan 2008: har var titel "", ikke titlefromadd, som gjorde at 24so crawling mistet titler.
				if (bbdocument_convert(ft, docbuf, docbufsize, &convdocbuf, &convdocbufsize, titlefromadd, subname, documenturi, lastmodified, acl_allow, acl_denied, "", NULL) == 0) {
					fprintf(stderr, "Failed on bbdocument_convert.\n");
					failed++;
					free(docbuf);
					free(convdocbuf);
					continue;
				}
				if (type == 1) {
					memcpy(curdocp, convdocbuf, convdocbufsize);
					curdocp += convdocbufsize;
				} else if (type == 2) {
					char subnamenew[256];
					//int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],const int dokument_size,unsigned int lastmodified,char *acl_allow, char *acl_denied,const char title[], char doctype[]) {
					snprintf(subnamenew, sizeof(subnamenew), "%s-%s", subname, part);
					bbdocument_add(subnamenew, documenturi, "html", convdocbuf, convdocbufsize, lastmodified, acl_allow, acl_denied, titlefromadd, "", "");
				}
				
				free(convdocbuf);
				free(docbuf);
			}
		}
		if (type == 1) {
			*curdocp = '\0';
			*documentfinishedbufsize = curdocp - documentfinishedbuftmp; 
		} else if (type == 2) {
			*documentfinishedbufsize = 1;
			*documentfinishedbuf = strdup(".");
		}
		//printf("Got this: %d %d<<\n%s\n", strlen(*documentfinishedbuf), *documentfinishedbufsize, *documentfinishedbuf);
		//free(p);
	}
	else {
		printf("unknown dokument outputformat \"%s\"\n",fileFilter->outputformat);
		(*documentfinishedbufsize) = 0;
		free(fileFilter);
		unlink(filconvertetfile_out_txt);
		unlink(filconvertetfile_out_html);
		unlink(filconvertetfile_real);
		return 0;
	}

	unlink(filconvertetfile_real);
	unlink(filconvertetfile_out_txt);
	unlink(filconvertetfile_out_html);

	#ifndef DEBUG
		//runarb: 13okr2007: hvorfor ver denne komentert ut? Det hoper seg opp med filer
		//unlink(filconvertetfile_real);
	#endif

	//printf("documentfinishedbuf is: \n...\n%s\n...\n", *documentfinishedbuf);

	free(fileFilter);

	return 1;
}


//stenger ned alle åpne filer
int bbdocument_close () {
	rclose();

}

int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],const int dokument_size,unsigned int lastmodified,char *acl_allow, char *acl_denied,const char title[], char doctype[], char *attributes) {

	struct ReposetoryHeaderFormat ReposetoryHeader;

	int htmlbuffersize = 0;//((dokument_size *2) +512);	//+512 da vi skal ha med div meta data, som html kode
	char *htmlbuffer = NULL;// = malloc(htmlbuffersize);
	char *imagebuffer;
	char *documenttype_real;
	size_t imageSize;
	unsigned int DocIDForExistTest;
	unsigned int lastmodifiedForExistTest;
	struct hashtable *metahash = NULL;

	printf("bbdocument_add: \"%s\"\n",documenturi);


	//tester at det ikke finnes først
	if ((uriindex_get(documenturi,&DocIDForExistTest,&lastmodifiedForExistTest,subname))
		&& (lastmodifiedForExistTest == lastmodified)
		) {
		printf("bbdocument_add: Uri \"%s\" all redy exist with DocID \"%u\" and time \"%u\"\n",documenturi,DocIDForExistTest,lastmodifiedForExistTest);
		return 0;
	}

	if (documenttype[0] == '\0') {
		if ((documenttype_real = sfindductype(documenturi)) == NULL) {
			printf("Will use .none because I can't decide format. File name isent dos type (8.3): %s\n", documenturi);
			documenttype_real = strdup("none");
			
		}
	}
	else {
		documenttype_real = malloc(strlen(documenttype)+1);
		strcpy(documenttype_real,documenttype);
	}


	//hvis vi ikke her med noen egen doctype så bruker vi den vi har fått via documenttype
	if (doctype[0] == '\0') {
		//vi trenger ikke å ha \0 på slutten her
		strncpy(ReposetoryHeader.doctype,documenttype_real,sizeof(ReposetoryHeader.doctype));
	}
	else {
		//vi trenger ikke å ha \0 på slutten her
		strncpy(ReposetoryHeader.doctype,doctype,sizeof(ReposetoryHeader.doctype));
	}

	if (!bbdocument_convert(documenttype_real,document,dokument_size,&htmlbuffer,&htmlbuffersize,title,subname,documenturi, lastmodified,acl_allow, acl_denied, doctype, &metahash)) {

		printf("can't run bbdocument_convert\n");
		//lager en tom html buffer
		//Setter titelen som subjekt. Hva hvis vi ikke har title?
		htmlbuffersize = strlen(html_tempelate) + strlen(title) + 1;
		htmlbuffer = malloc(htmlbuffersize);
		snprintf(htmlbuffer, htmlbuffersize, html_tempelate,title,"");
		htmlbuffersize = strlen(htmlbuffer);
		printf("useing title \"%s\" as title\n",title);
		printf("htmlbuffersize %i\n",htmlbuffersize);
	}

	if (metahash) {
		struct hashtable_itr *itr;

		if (hashtable_count(metahash) > 0) {
			itr = hashtable_iterator(metahash);
			do {
				char *key, *value;

				key = hashtable_iterator_key(itr);
				value = hashtable_iterator_value(itr);

				printf("Key: %s Value: %s\n", key, value);

				if (strcmp(key, "lastmodified") == 0) {
					lastmodified = atol(value);
				}
			} while (hashtable_iterator_advance(itr));
		}
		hashtable_destroy(metahash, 1);
	}

	//printf("document (size %i)\"%s\"\n",htmlbuffersize,htmlbuffer);


	//prøver å lag et bilde
	//if ( (imagebuffer = generate_thumbnail( document, dokument_size, &imageSize )) == NULL ) {
	if (!bbdocument_makethumb(documenttype_real,document,dokument_size,&imagebuffer,&imageSize)) {
		printf("can't generate image\n");
		ReposetoryHeader.imageSize = 0;
	}
	else {
		debug("generated image\n");
		ReposetoryHeader.imageSize = imageSize;
	}


	ReposetoryHeader.htmlSize = htmlbuffersize;
	ReposetoryHeader.clientVersion = 2.14;

	//runarb:  8 juli 2008: tar bort bruken av ReposetoryHeader's url
	//runarb: 11 juli 2008: kan ikke gjøre dette, da vi kopierer den inn i DocumentIndex fra ReposetoryHeader 
	strncpy(ReposetoryHeader.url,documenturi,sizeof(ReposetoryHeader.url));
	//hvis vi har DocID 0 har vi et system som ikke tar vare på docider. For eks bb eller bdisk.
	ReposetoryHeader.DocID = rGeneraeADocID(subname);
		
#ifdef DEBUG
	printf("Dident have a known DocID for document. Did generet on. DOcID is now %u\n",ReposetoryHeader.DocID);
#endif

	ReposetoryHeader.response = 200;
	strcpy(ReposetoryHeader.content_type,"htm");

	ReposetoryHeader.acl_allowSize = strlen(acl_allow);
#ifdef IIACL
	ReposetoryHeader.acl_deniedSize = strlen(acl_denied);
#endif
	ReposetoryHeader.time = lastmodified;

	ReposetoryHeader.storageTime = 0; //setes automatisk av rApendPostcompress

#ifdef DEBUG
	printf("ACL was allow \"%s\", %i bytes, denied \"%s\", %i bytes\nsubname %s\n",acl_allow,ReposetoryHeader.acl_allowSize,acl_allow,ReposetoryHeader.acl_allowSize,subname);
#endif

	ReposetoryHeader.urllen = strlen(documenturi);
	ReposetoryHeader.attributeslen = strlen(attributes);
	rApendPostcompress(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_denied, NULL, documenturi, attributes);

#ifdef DEBUG	
	printf("legger til DocID \"%u\", time \"%u\"\n",ReposetoryHeader.DocID,lastmodified);
	printf("htmlSize %ho, imageSize %ho\n",ReposetoryHeader.htmlSize,ReposetoryHeader.imageSize);
	printf("html: -%s-\n",htmlbuffer);
#endif

	uriindex_add(documenturi,ReposetoryHeader.DocID,lastmodified,subname);

	free(htmlbuffer);
	free(documenttype_real);
}	

int bbdocument_deletecoll(char collection[]) {
	char command[512];
	int LotNr;
	int i;
	char FilePath[512];
	char IndexPath[512];
	char DictionaryPath[512];
	FILE *fh;

	debug("Deleting collection: \"%s\"\n",collection);

	LotNr = 1;
	while((fh =lotOpenFileNoCasheByLotNr(LotNr,"reposetory","r",'s',collection)) != NULL) {
		GetFilPathForLot(FilePath,LotNr,collection);

		fclose(fh);

		rrmdir(FilePath);

		++LotNr;
	}

	for (i=0; i < 64; i++) {
		GetFilePathForIindex(FilePath,IndexPath,i,"Main","aa",collection);
		GetFilePathForIDictionary(FilePath,DictionaryPath,i,"Main","aa",collection);
		//printf("FilePath: %s\nIndexPath: %s\nDictionaryPath: %s\n",FilePath,IndexPath,DictionaryPath);
		if (rrmdir(IndexPath) != 1) {
                        perror("remove IndexPath");
                }
		if (rrmdir(DictionaryPath) != 1) {
                        perror("remove DictionaryPath");
                }
	}

	//sletter i userToSubname.db
        struct userToSubnameDbFormat userToSubnameDb;

        if (!userToSubname_open(&userToSubnameDb,'w')) {
                printf("can't open users.db\n");
        }
        else {
		userToSubname_deletecol(&userToSubnameDb,collection);

                userToSubname_close(&userToSubnameDb);
        }

	return 1;
}


unsigned int bbdocument_nrOfDocuments (char subname[]) {

	return rLastDocID(subname);
}



int uriindex_open(DB **dbpp, char subname[]) {

	DB *dbp = (*dbpp);


	char fileName[512];
	int ret;

	GetFilPathForLotFile(fileName,"urls.db",1,subname);

	#ifdef DEBUG
		printf("uriindex_open: Trying to open lotfile \"%s\"\n",fileName);
	#endif
        /********************************************************************
        * Opening nrOfFiles to stor the data in
        ********************************************************************/
                /* Create and initialize database object */
                if ((ret = db_create(&dbp, NULL, 0)) != 0) {
                        fprintf(stderr,
                            "%s: db_create: %s\n", "bbdocument", db_strerror(ret));
                        return 0;
                }

		//#define dbCashe 314572800       //300 mb
                ////setter cashe størelsen manuelt
                //if ((ret = dbp->set_cachesize(dbp, 0, dbCashe, dbCasheBlokes)) != 0) {
                //        dbp->err(dbp, ret, "set_cachesize");
                //}



                /* open the database. */
                if ((ret = dbp->open(dbp, NULL, fileName, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
                        dbp->err(dbp, ret, "%s: open", fileName);
                        //goto err1;
			//dette skjer nor collection mappen ikke er opprettet enda, typisk forde vi ikke har lagret et dokument der enda
			#ifdef DEBUG
			printf("can't dbp->open(), but db_create() was sucessful!\n");
			#endif

			dbp->close(dbp, 0);

			return 0;
                }

        /********************************************************************/

	#ifdef DEBUG
		printf("uriindex_open: finished\n");
	#endif

	(*dbpp) = dbp;

	return 1;
}

int uriindex_close (DB **dbpp) {

	DB *dbp = (*dbpp);

	int ret;

	#ifdef DEBUG
                printf("uriindex_close: closeing\n");
        #endif
      	if ((ret = dbp->close(dbp, 0)) != 0) {
        	fprintf(stderr, "%s: DB->close: %s\n", "bbdocument", db_strerror(ret));
                return (EXIT_FAILURE);
       	}

	#ifdef DEBUG
                printf("uriindex_close: finished\n");
        #endif

	(*dbpp) = dbp;
}
int uriindex_add (char uri[], unsigned int DocID, unsigned int lastmodified, char subname[]) {
        DB dbpArray;

        DB *dbp = NULL;

        DBT key, data;
	int ret;

	struct uriindexFormat uriindex;


	#ifdef DEBUG
	printf("uriindex_add: subname %s\n",subname);
	#endif

	uriindex_open(&dbp,subname);


	//resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

        //legger inn datane i bdb strukturen
        key.data = uri;
        key.size = strlen(uri);

	uriindex.DocID = DocID;
	uriindex.lastmodified = lastmodified;

        //data.data = &DocID;
        //data.size = sizeof(DocID);

	data.data = &uriindex;
	data.size = sizeof(uriindex);

        //legger til i databasen
        if  ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
                dbp->err(dbp, ret, "DB->put");
		//kan ikke returnere her for da blir den aldr lukket.
        	//return (EXIT_FAILURE);
        }
        



	uriindex_close(&dbp);



}

int uriindex_get (char uri[], unsigned int *DocID, unsigned int *lastmodified, char subname[]) {
        DB dbpArray;

        DB *dbp = NULL;

        DBT key, data;
        int ret;
	int forreturn = 1;
	struct uriindexFormat uriindex;

        if (!uriindex_open(&dbp,subname)) {
		#ifdef DEBUG
			fprintf(stderr,"can't open uriindex\n");
		#endif
		return 0;
	}


        //resetter minne
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

	//legger inn datane i bdb strukturen
        key.data = uri;
        key.size = strlen(uri);

        if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {

		*DocID = (*(struct uriindexFormat *)data.data).DocID;
		*lastmodified = (*(struct uriindexFormat *)data.data).lastmodified;
                forreturn = 1;
        }
        else if (ret == DB_NOTFOUND) {
		#ifdef DEBUG
                	dbp->err(dbp, ret, "DBcursor->get");
			printf("search for \"%s\", len %i\n",key.data,key.size);		
		#endif
                forreturn = 0;
        }
        else {
                dbp->err(dbp, ret, "DBcursor->get");
                forreturn = 0;
        }

	uriindex_close(&dbp);

	return forreturn;

}


int uriindex_delete (char uri[], char subname[]) {
        DB dbpArray;

        DB *dbp = NULL;

        DBT key;
        int ret;
	int forreturn = 1;
	struct uriindexFormat uriindex;

        if (!uriindex_open(&dbp,subname)) {
		fprintf(stderr,"can't open uriindex\n");
		return 0;
	}


        //resetter minne
        memset(&key, 0, sizeof(DBT));

	//legger inn datane i bdb strukturen
        key.data = uri;
        key.size = strlen(uri);

        if ((ret = dbp->del(dbp, NULL, &key, 0)) == 0) {
		forreturn = 1;
        }
        else {
                dbp->err(dbp, ret, "DBcursor->get");
                forreturn = 0;
        }

	uriindex_close(&dbp);

	return forreturn;

}

#endif
