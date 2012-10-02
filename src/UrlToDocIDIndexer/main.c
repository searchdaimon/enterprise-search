#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif


#define dbCashe 314572800	//300 mb
#define dbCasheBlokes 1		//hvor mange deler vi skal dele cashen opp i. Ofte kan man ikke alokere store blikker samenhengenede

#include "../common/define.h"

const char *progname = "UrlToDocIDIndexer";             /* Program name. */

int main (int argc, char *argv[]) {

	struct udfileFormat udfilePost;
	FILE *UDFILE;
	int count;
	int i,y;

	int dbNr = -1;

        if (argc != 3) {
                printf("Dette programet indekserer mange splitetet ud filer\n \
		udfiler er mappen med splittede urfiler\nbdbfiler er mappen der man skal lagre databasen\n\tBruk:\n\t \
		udfiler/ bdbfiler/\n");
                exit(0);
        }


	char *udfilerdir = argv[1];
	char *bdbfilerdir = argv[2];

	printf("udfilerdir \"%s\", bdbfilerdir \"%s\"\n",udfilerdir,bdbfilerdir);

	char udfile[512];
	char bdbfile[512];

	int ret;
	//DB *dbp;
	unsigned long crc32Value;

	DB dbpArray;

	DB *dbp;

	DBT key, data;
	//char fileName[256];
	int dbFileForUrl;

	unsigned int lastTime;
	unsigned int currentTime;
	double runtime;
	//int emtyUrls;
	int urls;

for (i=0; i < UrlToDocIDnrOfFiles;i++) {

		//finner db navne
		sprintf(udfile,"%s%i",udfilerdir,i);
		sprintf(bdbfile,"%s%i.db",bdbfilerdir,i);

                //#ifdef DEBUG
			printf("Opening file %s\n",udfile);
 
                	printf("Openig database: %s\n",bdbfile);
                //#endif

		//opner databasen
	/********************************************************************
        * Opening db to stor det data in
        ********************************************************************/
                /* Create and initialize database object */
                if ((ret = db_create(&dbp, NULL, 0)) != 0) {
                        fprintf(stderr,
                            "%s: db_create: %s\n", progname, db_strerror(ret));
                        return (EXIT_FAILURE);
                }


                //setter cashe størelsen manuelt
                if ((ret = dbp->set_cachesize(dbp, 0, dbCashe, dbCasheBlokes)) != 0) {
                        dbp->err(dbp, ret, "set_cachesize");
                }


                /* open the database. */
                if ((ret = dbp->open(dbp, NULL, bdbfile, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
                //if ((ret = dbp->open(dbp, NULL, bdbfile, NULL, DB_HASH, DB_CREATE, 0664)) != 0) {
                        dbp->err(dbp, ret, "%s: open", bdbfile);
                        //goto err1;
                }

        /********************************************************************/



	//opner filen

	printf("udfile %s\n",udfile);
	
         if ((UDFILE = fopen(udfile,"r")) == NULL) {
                 printf("Cant read udfile ");
                 perror(udfile);
                 exit(1);
         }

	urls = 0;
//        while(!feof(UDFILE)) {
//      for (y=0;y<70;y++) {
	while (( ret = fread(&udfilePost,sizeof(udfilePost),1,UDFILE)) > 0) {
		/*
                if (( ret = fread(&udfilePost,sizeof(udfilePost),1,UDFILE)) < 1) {
			perror("read udfile");
		}
		*/
		if (udfilePost.url[0] == '\0') {
                	printf("Url is emty. Url %s, DocID %u length %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));
			exit(1);			
		}

		#ifdef DEBUG
                printf("Url %s, DocID %u length %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));
		#endif

		//resetter minne
                memset(&key, 0, sizeof(DBT));
                memset(&data, 0, sizeof(DBT));

                //legger inn datane i bdb strukturen
                key.data = udfilePost.url;
                key.size = strlen(udfilePost.url);

                data.data = &udfilePost.DocID;
                data.size = sizeof(udfilePost.DocID);

		//legger til i databasen
                switch (ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) {
                        case 0:
                                break;
                        default:
                                dbp->err(dbp, ret, "DB->put");
                                fprintf(stderr, "%s: DB_KEYEXIST: %s (key \"%s\", val \"%u\")\n", progname, db_strerror(ret),key.data,data.data);
                		fprintf(stderr,"Url %s, DocID %u length %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));

                                if (ret != DB_KEYEXIST) {
					//vi dør ikke hvis det bare er en vedi som finnes
                                        exit (EXIT_FAILURE);
                                }
				

                                break;
                }


		++urls;
	}

	printf("urls %i\n",urls);

	fclose(UDFILE);



                if ((ret = dbp->close(dbp, 0)) != 0) {
                        fprintf(stderr, "%s: DB->close: %s\n", progname, db_strerror(ret));
                        return (EXIT_FAILURE);
                }

}



}
