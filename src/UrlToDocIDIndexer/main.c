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

#define nrOfFiles 64

#define dbCashe 314572800	//300 mb
#define dbCasheBlokes 1		//hvor mange deler vi skal dele cashen opp i. Ofte kan man ikke alokere store blikker samenhengenede

#include "../common/define.h"
#include "../common/crc32.h"

const char *progname = "UrlToDocIDIndexer";             /* Program name. */

int main (int argc, char *argv[]) {

	struct udfileFormat udfilePost;
	FILE *UDFILE;
	int count;
	int i,y;

	int dbNr = -1;

        if (argc < 2) {
                printf("Dette programet indekserer mange splitetet ud filer\n\n\tBruke\n\tUrlToDocIDIndexer slpitetUdfile ... n\n\n");
                exit(0);
        }



	int ret;
	//DB *dbp;
	unsigned long crc32Value;

	DB dbpArray;

	DB *dbp;

	DBT key, data;
	char fileName[256];
	int dbFileForUrl;

	unsigned int lastTime;
	unsigned int currentTime;
	double runtime;

for (i=1; i < argc;i++) {

		//finner db navne
		sprintf(fileName,"%s.db",argv[i],i);

                //#ifdef DEBUG
			printf("Opening file %s\n",argv[i]);
 
                	printf("Openig database: %s\n",fileName);
                //#endif

		//opner databasen
	/********************************************************************
        * Opening nrOfFiles to stor det data in
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
                if ((ret = dbp->open(dbp, fileName, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
                        dbp->err(dbp, ret, "%s: open", fileName);
                        //goto err1;
                }

        /********************************************************************/



	//opner filen

	printf("aa %s\n",argv[i]);
	
         if ((UDFILE = fopen(argv[i],"r")) == NULL) {
                 printf("Cant read udfile ");
                 perror(argv[i]);
                 exit(1);
         }
        while(!feof(UDFILE)) {
//      for (y=0;y<70;y++) {

                if (( ret = fread(&udfilePost,sizeof(udfilePost),1,UDFILE)) < 1) {
			perror("read udfile");
		}

                //printf("Url %s, DocID %u lemgth %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));

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
                                if (ret != DB_KEYEXIST) {
                                        fprintf(stderr, "%s: DB_KEYEXIST: %s\n", progname, db_strerror(ret));
                                        return (EXIT_FAILURE);
                                }

                                break;
                }


	}

	fclose(UDFILE);



                if ((ret = dbp->close(dbp, 0)) != 0) {
                        fprintf(stderr, "%s: DB->close: %s\n", progname, db_strerror(ret));
                        return (EXIT_FAILURE);
                }

}

exit(1);




	lastTime = (unsigned int)time(NULL);	
	count = 0;


		
		//lager en has verdi slik at vi kan velge en av filene
		crc32Value = crc32(udfilePost.url);
		dbFileForUrl = (crc32Value % nrOfFiles);


		if ((count % 100000) == 0) {
			currentTime = (unsigned int)time(NULL);

			runtime = (currentTime - lastTime);

			printf("komet til %i, time %f s\n", count,runtime);

			lastTime = currentTime;
		}

		count++;



}
