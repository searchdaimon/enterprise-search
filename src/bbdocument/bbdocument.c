#ifdef BLACK_BOX

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
#include <errno.h>
#include <assert.h>
#include <err.h>

#include "bbdocdefine.h"
#include "bbfilters.h"
#include "bbdocument.h"

#include "../common/define.h"
#include "../common/debug.h"
#include "../common/exeoc.h"
#include "../common/boithohome.h"
#include "../common/ht.h"
#include "../perlembed/perlembed.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"
#include "../common/timediff.h"
#include "../common/reposetory.h"
#include "../common/bstr.h"
#include "../common/bprint.h"
#include "../common/strlcpy.h"
#include "../common/DocumentIndex.h"
#include "../acls/acls.h"
#include "../common/bfileutil.h"
#include "../common/lot.h"
#include "../ds/dcontainer.h"
#include "../generateThumbnail/generate_thumbnail.h"
#include "../generateThumbnail/generate_thumbnail_by_convert.h"

#define PRIME_TBLSIZ 100

#define filconvertetfile "/tmp/boithofilconverter"

#define html_tempelate "<html>\n<head>\n<title>%s</title>\n<meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1'>\n</head>\n<body>\n%s\n</body>\n</html>\n"
#define html_text_tempelate "<html>\n<head>\n<title>%s</title>\n<meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1'>\n</head>\n<body>\n<pre style='word-wrap:break-word;'>%s</pre>\n</body>\n</html>\n"

// possibly just convert:
// ai
// bugger "psd"
// Magnus: it is actually called "jpeg", not "jepg" ;)
char *supportetimages[] = {"png", "jpg", "jpeg", "bmp", "tif", "tiff", "gif", "eps", "ai", '\0'};


//globals
//CHTbl htbl;
struct hashtable *h_fileFilter;




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

int uriindex_open(DB **dbpp, char subname[], u_int32_t flags) {

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

		/*
		//Set the cache size manually. May improve performance
		#define dbCashe 314572800       //300 mb
                if ((ret = dbp->set_cachesize(dbp, 0, dbCashe, dbCasheBlokes)) != 0) {
                        dbp->err(dbp, ret, "set_cachesize");
                }
		*/


                /* open the database. */
                if ((ret = dbp->open(dbp, NULL, fileName, NULL, DB_BTREE, flags, 0664)) != 0) {

			if (ret != ENOENT) {
				printf("can't dbp->open(), but db_create() was sucessful!\n");
			}

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


	return 1;
}

int uriindex_add (char uri[], unsigned int DocID, unsigned int lastmodified, char subname[]) {

	struct uriindexFormat uriindex;
        DB *dbp = NULL;

        DBT key, data;
	int ret;

	#ifdef DEBUG
	printf("uriindex_add: subname %s\n",subname);
	#endif

	uriindex_open(&dbp,subname, DB_CREATE);


	// reset memory
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

        // put the data into the bdb struct
        key.data = uri;
        key.size = strlen(uri);

	uriindex.DocID = DocID;
	uriindex.lastmodified = lastmodified;

	data.data = &uriindex;
	data.size = sizeof(uriindex);

        // add to the database
        if  ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
                dbp->err(dbp, ret, "DB->put");
		// can't return here, because then it would never be closed.
        	//return (EXIT_FAILURE);
        }

	uriindex_close(&dbp);

	return 1;

}

int uriindex_get (char uri[], unsigned int *DocID, unsigned int *lastmodified, char subname[]) {

        DB *dbp = NULL;

        DBT key, data;
        int ret;
	int forreturn = 1;

        if (!uriindex_open(&dbp,subname, DB_RDONLY)) {
		#ifdef DEBUG
			fprintf(stderr,"can't open uriindex\n");
		#endif
		return 0;
	}


        // reset memory
        memset(&key, 0, sizeof(DBT));
        memset(&data, 0, sizeof(DBT));

	// put the data into the bdb struct
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

        DB *dbp = NULL;

        DBT key;
        int ret;
	int forreturn = 1;

        if (!uriindex_open(&dbp,subname, DB_CREATE)) {
		fprintf(stderr,"can't open uriindex\n");
		return 0;
	}


        // reset memory
        memset(&key, 0, sizeof(DBT));

	// put the data into the bdb struct
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

int bbdocument_makethumb( char documenttype[],char document[],size_t dokument_size,char **imagebuffer,size_t *imageSize) {

	#ifdef BBDOCUMENT_IMAGE

	if (strcmp(documenttype,"pdf") == 0) {
		// pdf convert
		if (((*imagebuffer) = generate_pdf_thumbnail( document, dokument_size, imageSize )) == NULL) {
			return 0;

		}
		return 1;
	}
	else if (canconvert(documenttype) && (((*imagebuffer) = generate_thumbnail( document, dokument_size, imageSize )) == NULL)) {
		return 0;
	}

	#else
	#if BBDOCUMENT_IMAGE_BY_CONVERT
	if (strcmp(documenttype,"pdf") == 0) {
		// pdf convert
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
	
	// only here if we failed to create an image, and then of course we have no size
	//printf("imageSize %"PRId64" at %s:%i\n",(*imageSize),__FILE__,__LINE__);
	return 0;

}

int bbdocument_freethumb(char *imagebuffer) {
	#ifdef BBDOCUMENT_IMAGE
	imagebuffer = free_thumbnail_memory( imagebuffer );
	#endif

	return 1;
}

void bbdocument_clean() {

	perl_embed_clean();
}

int bbdocument_init(container **attrkeys) {

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
	perl_embed_init(NULL, 1);


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
			

			// blank lines and comments that start with #
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

				// add the old filter
				if (fileFilter != NULL) {
					if (NULL != hashtable_search(h_fileFilter,fileFilter->documentstype )) {
						printf("####################### BUG ################################\n");
						printf("allredy have a filter for \"%s\"!\n",fileFilter->documentstype);
						printf("#######################/BUG ################################\n");
					}
					//add to hash
					printf("inserting %s\n",(*fileFilter).documentstype);
					//chtbl_insert(&htbl,(void *)fileFilter);
					if (!hashtable_insert(h_fileFilter,fileFilter->documentstype,fileFilter) ) {
                        		        printf("cant insert\n");
                		        	exit(-1);
		                        }

					printf("end inserting\n");
				}
				// start a new filter

				fileFilter = malloc(sizeof(struct fileFilterFormat));
				fileFilter->attrwhitelist = NULL;

				// not all file filters have set all options, so we zero everything out, to make it easy to strcmp()
				// for a value, without having to worry that it might not be set.
				memset(fileFilter,'\0',sizeof(struct fileFilterFormat));

				// default to FILTER_EXEOC
				fileFilter->filtertype = FILTER_EXEOC;

				strcpy((*fileFilter).documentstype,splitdata[1]);
				
				strlcpy(fileFilter->path, path, sizeof fileFilter->path);
				

			}
			else if (strcmp(splitdata[0],"command") == 0) {
				// we may have a : in the command. So first copy in the whole thing, then we strip out command:
				//strcpy((*fileFilter).command,splitdata[1]);
			
				strscpy((*fileFilter).command,lines,sizeof((*fileFilter).command));
				strcasesandr((*fileFilter).command,sizeof((*fileFilter).command),"command: ","");
				// add the path where we have said we want a local path ( ./ )
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
				// stdio, file, etc,,
				strcpy((*fileFilter).outputtype,splitdata[1]);
			}
			else if (strcmp(splitdata[0],"outputformat") == 0) {
				//text, html
				strcpy((*fileFilter).outputformat,splitdata[1]);
			}
			else if (strcmp(splitdata[0], "filtertype") == 0) {
			
				if (strcmp(splitdata[1], FILTER_EXEOC_STR) == 0)
					fileFilter->filtertype = FILTER_EXEOC;

				else if (strcmp(splitdata[1], FILTER_PERL_PLUGIN_STR) == 0) 
					fileFilter->filtertype = FILTER_PERL_PLUGIN;

				else 
					errx(1, "Unknown filtertype %s\n", splitdata[1]);
			
			}
			else if (strcmp(splitdata[0], "attrwhitelist") == 0) {
				// TODO: Free fileFilter->attrwhitelist
				if (!split(splitdata[1], ",", &fileFilter->attrwhitelist))
					warnx("attrwhitelist was empty.");

			}
			else {
				printf("unknown command \"%s\"\n",lines);
			}

			// clean
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
		// mark that we have added it
		fileFilter = NULL;

		fclose(filep);
	}
	closedir(dirp);

	if (attrkeys != NULL) {
		*attrkeys = ropen();
	}

	return 1;
}

int bbdocument_delete (char uri[], char subname[]) {
	
	uriindex_delete (uri, subname);

	return 1;
}
void bbdocument_exist_update_di(char subname[],docid DocID) {

	// Update DI with new existed timestamp
	struct DocumentIndexFormat docindex;

	memset(&docindex,0,sizeof(docindex));

	#if defined(BLACK_BOX) && !defined(_24SEVENOFFICE)
		DIRead(&docindex, DocID, subname);
		docindex.lastSeen = time(NULL);
		DIWrite(&docindex, DocID, subname, NULL);
		debug("bbdocument_exist: satte lastSeen \"%s\"",ctime(&docindex.lastSeen));
	#endif

}
int bbdocument_exist_zero_lottery(int odds) {
	return ( (rand() % odds) == 1 );
}

int bbdocument_exist(char subname[],char documenturi[],unsigned int lastmodified, unsigned int no_lottery) {
	docid DocID;
	unsigned int lastmodifiedForExistTest;

        debug("bbadocument_exist(subname=\"%s\", documenturi=\"%s\", lastmodified=%u, no_lottery=%u)",subname,documenturi,lastmodified,no_lottery);

	if (!uriindex_get(documenturi,&DocID,&lastmodifiedForExistTest,subname)) {
		debug("bbdocument_exist: uriindex_get() feil. This must be an unknow url. Will crawl\n");
		return 0;
	}
	else if (lastmodifiedForExistTest == lastmodified) {
		debug("bbdocument_exist: Uri \"%s\" exists with DocID \"%u\" and time \"%u\"\n", documenturi, DocID, lastmodifiedForExistTest);
		bbdocument_exist_update_di(subname,DocID);
		return 1;
	}
	else if ((no_lottery == 0) && (lastmodified == 0) && bbdocument_exist_zero_lottery(20)) {
		// we need to occasionally crawl those with unknown/0 lastmodified
		debug("bbdocument_exist: Uri \"%s\" exists and we got 0 as current time. But we did winn in the lottery, and are askinf for it to be crawled. DocID \"%u\"\n", documenturi, DocID);
		return 0;
	}
	else if (lastmodified == 0) {
		debug("bbdocument_exist: Uri \"%s\" exists and we got 0 as current time.. DocID \"%u\"\n", documenturi, DocID);
		bbdocument_exist_update_di(subname,DocID);
		return 1;
	}


	// none of the tests matched. The document does not exist
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
 
// strip < and > characters, since the html parser would think they are html tags.
// they are of course in the form < and >
void stripTags(char *cpbuf, int cplength) {

	int i;

	for (i=0;i<cplength;i++) {
		if ((cpbuf[i] == '<') || (cpbuf[i] == '>')) {
			cpbuf[i] = ' ';
		}
	}
}

int bbdocument_convert(char filetype[],char document[],const int dokument_size, buffer *outbuffer, const char titlefromadd[], char *subname, char *documenturi, unsigned int lastmodified, char *acl_allow, char *acl_denied, struct hashtable **metahash) {

	FILE *filconfp=NULL;
	char filconvertetfile_real[216] = "";
	char filconvertetfile_out_txt[216] = "";
	char filconvertetfile_out_html[216] = "";
	int exeocbuflen;
	int i;
	char *documentfinishedbuftmp;
	char fileconverttemplate[1024];
	struct fileFilterFormat *fileFilter = NULL;

        #ifdef DEBUG_TIME
                struct timeval start_time, end_time;
	#endif

	printf("bbdocument_convert: dokument_size %i, title \"%s\",filetype \"%s\"\n",dokument_size,titlefromadd,filetype);

	// convert filename to lowercase
	for (i=0;i < strlen(filetype);i++) {
		//printf("%c\n",filetype[i]);
		filetype[i] = btolower(filetype[i]);
	}

	// if we have an html document we can use it directly
	// this is a bit inefficient though, since we do a memory copy
	if ((strcmp(filetype,"htm") == 0) || (strcmp(filetype,"html") == 0 )) {
		if (titlefromadd[0]=='\0') {
			bmemcpy(outbuffer, document, dokument_size);
		}
		else {
			// Some documents may have supplied a title at add time without having a title in the html (e.g. email).
			// Add the correct title to the document.
			// The html parser only honors the first title, so it should be enough to add it
			// at the top of the document.
			bprintf(outbuffer, "<title>%s</title>\n", titlefromadd);
			bmemcpy(outbuffer, document, dokument_size);
		}
		return 1;
	}
	else if (strcmp(filetype,"hnxt") == 0) {
		ntobr(document, dokument_size);
		bprintf(outbuffer, html_tempelate, titlefromadd, document);
		return 1;
	}

	#ifdef DEBUG
	printf("strcmp done\n");
	#endif

	struct fileFilterFormat *fileFilterOrginal;



	
	if (NULL == (fileFilterOrginal = hashtable_search(h_fileFilter,filetype) )) {
		printf("don't have converter for \"%s\"\n",filetype);

		#ifdef DEBUG
			printf("writing to unknownfiltype.log\n");
			FILE *fp;
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
		#endif

		return 0;
	}

	// if this is a file of type text we don't need to convert it.
	if (strcmp((*fileFilterOrginal).format,"text") == 0) {
		#ifdef DEBUG
			printf("fileFilter ses it is a file of format text. Can use it direktly\n");
		#endif

		char *cpbuf;
		int cpbufsize;

		// convert all \n to <br>
		cpbufsize = (dokument_size + 512 +1);
		cpbuf = malloc(cpbufsize);

		memcpy(cpbuf,document,dokument_size);
		cpbuf[dokument_size] = '\0';

		// strip < and > characters, since the html parser would think they are html tags.
		// they are of course in the form < and >
		stripTags(cpbuf,dokument_size);

		#ifdef DEBUG
		printf("document %i\n",strlen(document));
		#endif

		bprintf(outbuffer, html_text_tempelate, titlefromadd, cpbuf);
		//printf("documentfinishedbuf %i\n", buffer_length(outbuffer));
		free(cpbuf);

                return 1;
	}

	// we have to make a copy of the file filter info, since we are going to modify it.
	fileFilter = malloc(sizeof(struct fileFilterFormat));

	memcpy(fileFilter, fileFilterOrginal, sizeof(struct fileFilterFormat));

	#ifdef DEBUG
		printf("have converter for file type\n");
	#endif


	/*****************************************************************************
		We have a converter. Must write to a file in order to be able to pass it along
	*****************************************************************************/

	pid_t pid = getpid();

	sprintf(fileconverttemplate, "%s-%d", filconvertetfile, rand());
	sprintf(filconvertetfile_real,"%s-%u.%s",fileconverttemplate, (unsigned int)pid,filetype);
	sprintf(filconvertetfile_out_txt,"%s-%u.txt",fileconverttemplate, (unsigned int)pid);
	sprintf(filconvertetfile_out_html,"%s-%u.html",fileconverttemplate, (unsigned int)pid);

	#ifdef DEBUG
	printf("bbdocument_convert: filconvertetfile_real \"%s\"\n",filconvertetfile_real);
	#endif
	if ((filconfp = fopen(filconvertetfile_real,"wb")) == NULL) {
		perror(filconvertetfile_real);
		exit(1);
	}
	flock(fileno(filconfp),LOCK_EX);
	fwrite(document,1,dokument_size,filconfp);
	fclose(filconfp);

	// reopen it read only, and take a shared lock on the file, so that we avoid the perl /tmp watch deleting it.
	if ((filconfp = fopen(filconvertetfile_real,"rb")) == NULL) {
		perror(filconvertetfile_real);
		exit(1);
	}
	flock(fileno(filconfp),LOCK_SH);

	//convert to text.
	/*****************************************************************************/


	strsandr((*fileFilter).command,"#file",filconvertetfile_real);
	strsandr((*fileFilter).command,"#outtxtfile",filconvertetfile_out_txt);
	strsandr((*fileFilter).command,"#outhtmlfile",filconvertetfile_out_html);
	exeocbuflen = (dokument_size * 2) + 513; // XXX: Find a better way //(*documentfinishedbufsize);
	if ((documentfinishedbuftmp = malloc(exeocbuflen)) == NULL) {
		perror("Can't malloc documentfinishedbuftmp");
		return 0;
	}

	switch (fileFilter->filtertype) {
		case FILTER_EXEOC:
			run_filter_exeoc(
				documentfinishedbuftmp,  
				exeocbuflen,
				fileFilter, 
				metahash
			);
			break;
		case FILTER_PERL_PLUGIN:
			run_filter_perlplugin(
				documentfinishedbuftmp,
				exeocbuflen ,
				fileFilter,
				metahash
			);
			break;
		default:
			errx(1, "Unknown filtertype '%d'", fileFilter->filtertype);
	}

#ifdef USE_LIBEXTRACTOR
	if (fileFilter->attrwhitelist != NULL)
		add_libextractor_attr(metahash, filconvertetfile_real, fileFilter->attrwhitelist);
#endif


//<<<<<<< bbdocument.c

//=======
	// here we parse arguments ourselves, and each space becomes a new argument, even when we
	// use "a b", which doesn't really become two arguments here, a and b
	// split on space to build an argc
	/*TokCount = split((*fileFilter).command, " ", &splitdata);
	//#ifdef DEBUG
	printf("splitet comand in %i, program is \"%s\"\n",TokCount,splitdata[0]);
	//#endif
	printf("running: %s\n",(*fileFilter).command);
	// pass along the size of the buffer now. Will get back the size of what we read
	char *execobuf = malloc(exeocbuflen);
//>>>>>>> 1.64

//<<<<<<< bbdocument.c
//=======
	char *envpairpath = strdup(/tmp/converter-metadata-XXXXXX);
	char envpair[PATH_MAX];
	mktemp(envpairpath);
	sprintf(envpair, "SDMETAFILE=%s", envpairpath);
	free(envpairpath);
	envpairpath = envpair + strlen("SDMETAFILE=");
        char *shargs[] = { "/usr/bin/env", NULL, "/bin/sh", "-c", NULL, NULL, };
	shargs[1] = envpair;
        shargs[4] = fileFilter->command;

        #ifdef DEBUG_TIME
                gettimeofday(&start_time, NULL);
        #endif

	if (!exeoc_timeout(shargs, execobuf, &exeocbuflen, &ret, 120)) {
		printf("dident get any data from exeoc. But can be a filter that creates files, sow we wil continue\n");
		execobuf[0] = '\0';
		exeocbuflen = 0;
	}

        #ifdef DEBUG_TIME
                gettimeofday(&end_time, NULL);
                printf("Time debug: exeoc_timeout() time: %f\n",getTimeDifference(&start_time, &end_time));
        #endif
	*/
/*
	if (metahash) {
		FILE *metafp;

		*metahash = create_hashtable(3, ht_stringhash, ht_stringcmp);

		if ((metafp = fopen(envpairpath, "r")) != NULL) {
			char *key, *value, line[2048];

			while (fgets(line, sizeof(line), metafp)) {
				char *p, *p2;

				// Comment
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
				p++; // Skip past = 
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
	} */

//>>>>>>> 1.64
#ifdef DEBUG
	//printf("did convert to %i bytes (strlen %i)\n",exeocbuflen,strlen(documentfinishedbuftmp));
#endif

	if (strcmp((*fileFilter).outputformat,"text") == 0) {
                // strip < and > characters, since the html parser would think they are html tags.
                // they are of course in the form < and >
                stripTags(documentfinishedbuftmp,strlen(documentfinishedbuftmp));

		bprintf(outbuffer, html_text_tempelate,titlefromadd,documentfinishedbuftmp);
	}
	else if (strcmp((*fileFilter).outputformat,"html") == 0) {
		// html doesn't need converting
		// this is the output format html. Not the filetype output format. Filetype is handled further up
		// ToDo: should probably copy in some data here???
		bprintf(outbuffer, "%s", documentfinishedbuftmp);
		// In file conversion the title passed along (from add) will be the filename.
		// We only use it if the document itself does not have a title.
		// Therefore that title is added at the bottom of the document:
		bprintf(outbuffer, "<title>%s</title>\n", titlefromadd);
	}
	else if (strcmp((*fileFilter).outputformat,"textfile") == 0) {
		FILE *fh;
		struct stat inode; 
		char *cpbuf;
		printf("filconvertetfile_out_txt: \"%s\"\n",filconvertetfile_out_txt);

		if ((fh = fopen(filconvertetfile_out_txt,"rb")) == NULL) {
			printf("can't open out file \"%s\"\n",filconvertetfile_out_txt);
			perror(filconvertetfile_out_txt);
			goto bbdocument_convert_error;
		}		
       		fstat(fileno(fh),&inode);


                if ((cpbuf = malloc(inode.st_size +1)) == NULL) {
			perror("malloc");
			goto bbdocument_convert_error;
		}
                
        	fread(cpbuf,1,inode.st_size,fh);
		cpbuf[inode.st_size] = '\0';

		printf("did read back %i bytes from file \"%s\"\n",(int)inode.st_size,filconvertetfile_out_txt);

		printf("strlen cpbuf: %i\n",strlen(cpbuf));

                // strip < and > characters, since the html parser would think they are html tags.
                // they are of course in the form < and >
                stripTags(cpbuf,inode.st_size);

		fclose(fh);

		//printf("have size %i\n",(*documentfinishedbufsize));

		bprintf(outbuffer, html_text_tempelate, titlefromadd, cpbuf);
		free(cpbuf);

		// delete the file we created
		unlink(filconvertetfile_out_txt);
	}
	else if (strcmp((*fileFilter).outputformat,"htmlfile") == 0) {
		FILE *fh;
		struct stat inode; 
		size_t n;
		char buf[4096];
		if ((fh = fopen(filconvertetfile_out_html,"rb")) == NULL) {
			printf("can't open out file \"%s\"\n",filconvertetfile_out_html);
			perror(filconvertetfile_out_html);
			goto bbdocument_convert_error;
		}		
       		fstat(fileno(fh),&inode);
#if 0
		if ((*documentfinishedbufsize) > inode.st_size) {
			(*documentfinishedbufsize) = inode.st_size;
		}
#endif
		while ((n = fread(buf, 1, sizeof(buf)-1, fh)) > 0) {
			bmemcpy(outbuffer, buf, n);
		}

		fclose(fh);
		unlink(filconvertetfile_out_html);
		bprintf(outbuffer, "<title>%s</title>\n", titlefromadd);
	}
	else if (strcmp(fileFilter->outputformat, "dir") == 0 || strcmp(fileFilter->outputformat, "diradd") == 0) {
		char *p, *pstart;
		/* Does len do anything any more? */
		int len, failed = 0;
		int type; /* 1 for dir, 2 for diradd */

		type = (strcmp(fileFilter->outputformat, "dir") == 0) ? 1 : 2;

		len = exeocbuflen;
		p = strdup(documentfinishedbuftmp);
		pstart = p;
		if (p == NULL) {
			goto bbdocument_convert_error;
		}
		bprintf(outbuffer, html_text_tempelate, titlefromadd, "");
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

			/* We have a new file, let's get to work on it */
			//printf("########Got: %s: %s\n", ft, path);
			{
				char *docbuf;
				int docbufsize;
				struct stat st;
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

				//runarb: 18 jan 2008: here the title was "", not titlefromadd, which made 24so crawling lose titles.
				if (bbdocument_convert(ft, docbuf, docbufsize, outbuffer, titlefromadd, subname, documenturi, lastmodified, acl_allow, acl_denied,  NULL) == 0) {
					fprintf(stderr, "Failed on bbdocument_convert.\n");
					failed++;
					free(docbuf);
					continue;
				}
				
				free(docbuf);
			}
		}
		if (type == 2) {
			assert(0);
#if 0
			*documentfinishedbufsize = 1;
			*documentfinishedbuf = strdup(".");
#endif
		}
		//printf("Got this: %d %d<<\n%s\n", strlen(*documentfinishedbuf), *documentfinishedbufsize, *documentfinishedbuf);
		free(pstart);
	}
	else {
		printf("unknown dokument outputformat \"%s\"\n",fileFilter->outputformat);
		free(documentfinishedbuftmp);
		goto bbdocument_convert_error;
	}

	free(documentfinishedbuftmp);

	unlink(filconvertetfile_real);
	unlink(filconvertetfile_out_txt);
	unlink(filconvertetfile_out_html);

	fclose(filconfp);

	#ifndef DEBUG
		//runarb: 13okr2007: why is this commented out? Files pile up
		//unlink(filconvertetfile_real);
	#endif

	//printf("documentfinishedbuf is: \n...\n%s\n...\n", *documentfinishedbuf);

	free(fileFilter);

	return 1;

	bbdocument_convert_error:
		if (filconvertetfile_real[0] != '\0') {
			unlink(filconvertetfile_real);
		}
		if (filconvertetfile_out_txt[0] != '\0') {
			unlink(filconvertetfile_out_txt);
		}
		if (filconvertetfile_out_html[0] != '\0') {
			unlink(filconvertetfile_out_html);
		}

		if (filconfp != NULL) {
			fclose(filconfp);
		}
		if (fileFilter != fileFilter) {
			free(fileFilter);
		}

		if (fileFilter != NULL) {
			free(fileFilter);
		}
		return 0;
}


// close down all open files
int bbdocument_close (container *attrkeys) {
	rclose(attrkeys);

	return 1;
}

int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],const int dokument_size,unsigned int lastmodified,char *acl_allow, char *acl_denied,const char title[], char doctype[], char *attributes, container *attrkeys, char *image, int image_size, unsigned char PopRank) {

	struct ReposetoryHeaderFormat ReposetoryHeader;

	size_t htmlbuffersize = 0;//((dokument_size *2) +512);	//+512 because we want to include various meta data, as html code
	char *htmlbuffer = NULL;// = malloc(htmlbuffersize);
	char *imagebuffer;
	char *documenttype_real;
	size_t imageSize;
	unsigned int DocIDForExistTest;
	unsigned int lastmodifiedForExistTest;
	struct hashtable *metahash = NULL;
	buffer *documentbuffer;

	memset(&ReposetoryHeader,0,sizeof(ReposetoryHeader));

	printf("bbdocument_add: \"%s\"\n",documenturi);

	printf("bbdocument_add: Attributes = \"%s\" (", attributes);
	if (attrkeys!=NULL) printf("+)\n");
	else printf(" )\n");


	// first test that it doesn't already exist. If it does we handle it. Possibly also store the new version if it has changed.
	if (uriindex_get(documenturi,&DocIDForExistTest,&lastmodifiedForExistTest,subname)) {

		if (lastmodified == 0) {
			printf("bbdocument_add: Uri \"%s\" all redy exist. And we was asked not to test for modify time (modify time was 0). Will not add.\n",documenturi);
			return 1;
		}

		if (lastmodifiedForExistTest == lastmodified) {
			printf("bbdocument_add: Uri \"%s\" all redy exist with DocID \"%u\" and time \"%u\". Will not add.\n",documenturi,DocIDForExistTest,lastmodifiedForExistTest);
			return 1;
		}
		
		printf("We have document from before, but modified time have been updated. Will update the document.\n");
		printf("lastmodifiedForExistTest: %u, lastmodified: %u\n",lastmodifiedForExistTest,lastmodified);
		// if the url is known, but updated, we reuse it.
		ReposetoryHeader.DocID = DocIDForExistTest;
	}
	else {
		// if we don't have a DocID we have a system that needs to have it created. For example bb or bdisk.
		ReposetoryHeader.DocID = rGeneraeADocID(subname);

		#ifdef DEBUG
			printf("Dident have a known DocID for document. Did generet on. DOcID is now %u\n",ReposetoryHeader.DocID);
		#endif

	}

	// if we have a "text applicatino" encoding, which isn't a normal text file but text coming from
	// an application, we treat it as text.
	//if (strcmp(documenttype,"tapp") == 0) {
	//	documenttype_real = strdup("text");
	//}
	//else 
	if (documenttype[0] == '\0') {
		if ((documenttype_real = sfindductype(documenturi)) == NULL) {
			printf("Will use .none as documenttype because I can't decide format. File name isent dos type (8.3): %s\n", documenturi);
			documenttype_real = strdup("none");
			
		}
	}
	else {
		documenttype_real = malloc(strlen(documenttype)+1);
		strcpy(documenttype_real,documenttype);
	}



	// if we don't have a separate doctype here, we use the one we got via documenttype
	if (doctype[0] == '\0') {
		// we don't need to have \0 at the end here
		strncpy(ReposetoryHeader.doctype,documenttype_real,sizeof(ReposetoryHeader.doctype));
	}
	else {
		// we don't need to have \0 at the end here
		strncpy(ReposetoryHeader.doctype,doctype,sizeof(ReposetoryHeader.doctype));
	}

	documentbuffer = buffer_init(0);
	if (!bbdocument_convert(documenttype_real,document,dokument_size,documentbuffer,title,subname,documenturi, lastmodified,acl_allow, acl_denied, &metahash)) {

		printf("can't run bbdocument_convert\n");
		//create an empty html buffer
		// Set the title as the subject. What if we don't have a title?
		htmlbuffersize = strlen(html_text_tempelate) + strlen(title) + 1;
		htmlbuffer = malloc(htmlbuffersize);
		snprintf(htmlbuffer, htmlbuffersize, html_text_tempelate,title,"");
		htmlbuffersize = strlen(htmlbuffer);
		printf("useing title \"%s\" as title\n",title);
		printf("htmlbuffersize %i\n",htmlbuffersize);
		free(buffer_abort(documentbuffer));
	} else {
		htmlbuffersize = buffer_length(documentbuffer);
		htmlbuffer = buffer_exit(documentbuffer);
	}

	buffer *attrbuffer = buffer_init(-1);
	bprintf(attrbuffer, "%s,", attributes);

	if (metahash) {
		struct hashtable_itr *itr;

		if (hashtable_count(metahash) > 0) {
			itr = hashtable_iterator(metahash);
			do {
				char *key, *value;

				key = hashtable_iterator_key(itr);
				value = hashtable_iterator_value(itr);

				printf("%s: Key: %s Value: %s\n", documenttype_real, key, value);

				if (strcmp(key, "lastmodified") == 0) {
					lastmodified = atol(value);
					continue;
				}

				bprintf(attrbuffer, "%s=%s,", key, value);
			} while (hashtable_iterator_advance(itr));

			free(itr);
		}
		hashtable_destroy(metahash, 1);
	}
	char *all_attributes = buffer_exit(attrbuffer);


	// try to create an image
	if ((image_size != 0) && (bbdocument_makethumb("jpg",image,image_size,&imagebuffer,&imageSize))) {
		debug("generated image from input image\n");
		ReposetoryHeader.imageSize = imageSize;
	}
	else if (bbdocument_makethumb(documenttype_real,document,dokument_size,&imagebuffer,&imageSize)) {
		debug("generated image from document\n");
		ReposetoryHeader.imageSize = imageSize;
	}
	else {
		printf("Can't generate image.\n");
		ReposetoryHeader.imageSize = 0;
		imagebuffer = NULL;
	}


	ReposetoryHeader.clientVersion = 2.14;

	//runarb:  8 juli 2008: removing the use of ReposetoryHeader's url
	//runarb: 11 juli 2008: can't do this, since we copy it into DocumentIndex from ReposetoryHeader 
	strncpy(ReposetoryHeader.url,documenturi,sizeof(ReposetoryHeader.url));
		

	ReposetoryHeader.response = 200;
	strcpy(ReposetoryHeader.content_type,"htm");

	ReposetoryHeader.acl_allowSize = strlen(acl_allow);
#ifdef IIACL
	ReposetoryHeader.acl_deniedSize = strlen(acl_denied);
#endif
	ReposetoryHeader.time = lastmodified;

	ReposetoryHeader.storageTime = 0; // set automatically by rApendPostcompress

#ifdef DEBUG
	printf("ACL was allow \"%s\", %i bytes, denied \"%s\", %i bytes\nsubname %s\n",acl_allow,ReposetoryHeader.acl_allowSize,acl_allow,ReposetoryHeader.acl_allowSize,subname);
#endif

	ReposetoryHeader.urllen = strlen(documenturi);
	ReposetoryHeader.attributeslen = strlen(all_attributes);
	ReposetoryHeader.PopRank = PopRank;
	rApendPostcompress(&ReposetoryHeader, htmlbuffer, imagebuffer, subname, acl_allow, acl_denied, NULL, documenturi, all_attributes, attrkeys, htmlbuffersize);

#ifdef DEBUG	
	printf("legger til DocID \"%u\", time \"%u\"\n",ReposetoryHeader.DocID,lastmodified);
	printf("htmlSize %u, imageSize %ho\n",ReposetoryHeader.htmlSize2,ReposetoryHeader.imageSize);
	printf("html: -%s-\n",htmlbuffer);
#endif

	uriindex_add(documenturi,ReposetoryHeader.DocID,lastmodified,subname);

	free(all_attributes);
	free(htmlbuffer);
	free(documenttype_real);

	if (imagebuffer != NULL) {
		free(imagebuffer);
	}

	return 1;
}	

int bbdocument_deletecoll(char collection[]) {

	int LotNr;
	int i, y;
	char FilePath[512];
	char IndexPath[512];
	char DictionaryPath[512];
	FILE *fh;
	char *iindexes[] = { "Main","acl_allow","acl_denied","attributes", NULL };

	debug("Deleting collection: \"%s\"\n",collection);

	// An empty name would make the paths below point at the shared "iindex/"
	// trees themselves; "www" is the legacy shared layout with no per-collection
	// directory. Neither is a collection anyone can delete.
	if (collection[0] == '\0' || strcmp(collection,"www") == 0) {
		fprintf(stderr,"bbdocument_deletecoll: refusing to delete collection \"%s\"\n",collection);
		return 0;
	}

	LotNr = 1;
	while((fh =lotOpenFileNoCasheByLotNr(LotNr,"reposetory","r",'s',collection)) != NULL) {
		GetFilPathForLot(FilePath,LotNr,collection);

		fclose(fh);

		rrmdir(FilePath);

		++LotNr;
	}

	for (i=0; i < 64; i++) {

		for (y=0; iindexes[y] != NULL; y++) {

			GetFilePathForIindex(FilePath,IndexPath,i,iindexes[y],"aa",collection);
			#ifdef DEBUG
				printf("Deleting: FilePath: %s\nIndexPath: %s\n",FilePath,IndexPath);
			#endif

			if ((unlink(IndexPath) != 1) && (errno != ENOENT)) { //ENOENT=No such file or directory. Don't show an error if the file didn't exist. That is completely normal
                	        perror("remove IndexPath");
                	}


			GetFilePathForIDictionary(FilePath,DictionaryPath,i,iindexes[y],"aa",collection);
			#ifdef DEBUG
				printf("Deleting: FilePath: %s\nDictionaryPath: %s\n",FilePath,DictionaryPath);
			#endif

			if ((unlink(DictionaryPath) != 0) && (errno != ENOENT)) {//ENOENT=No such file or directory. Don't show an error if the file didn't exist. That is completely normal
                	        perror("remove DictionaryPath");
                	}
		}
	}

	// The loop above only knows the index files for one language ("aa") and
	// left everything else behind: per-language index and dictionary files,
	// attribute indexes and the directories themselves. A collection recreated
	// under the same name then answered queries with stale DocIDs. Remove the
	// whole iindex/<collection> tree from every data directory.
	for (i=0; i < NrOfDataDirectorys; i++) {
		struct stat st;
		GetFilePathForIindexCollection(IndexPath,i,collection);
		if (stat(IndexPath,&st) == 0 && S_ISDIR(st.st_mode)) {
			debug("Deleting iindex tree: %s\n",IndexPath);
			rrmdir(IndexPath);
		}
	}

	// delete from userToSubname.db
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


#endif
