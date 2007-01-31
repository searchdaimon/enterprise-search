#include <db.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include "../common/define.h"

const char *progname = "ex_access";             /* Program name. */

int main (int argc, char *argv[]) {

	struct udfileFormat udfilePost;
	FILE *UDFILE;
	int count;
	int i;

	int dbNr = -1;

        if (argc < 3) {
                printf("Dette programet indekserer en ud file\n\n\tBruke\n\tUrlToDocIDIndexer indexfile.db udfile\n\n");
                exit(0);
        }



	int ret;
	DB *dbp;

	DBT key, data;

	
	
	count = 0;
//        while(!feof(UDFILE)) {
	  for (i=0;i<900000;i++) {



		fread(&udfilePost,sizeof(udfilePost),1,UDFILE);


		

        	/* Create and initialize database object */
        	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        	        fprintf(stderr,
        	            "%s: db_create: %s\n", progname, db_strerror(ret));
        	        return (EXIT_FAILURE);
        	}

        	/* open the database. */
        	if ((ret = dbp->open(dbp, argv[1], NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        	        dbp->err(dbp, ret, "%s: open", argv[1]);
        	        //goto err1;
        	}

        	if ((UDFILE = fopen(argv[2],"r")) == NULL) {
        	        printf("Cant read udfile ");
        	        perror(argv[1]);
        	        exit(1);
	        }


		//printf("Url %s, DocID %u lemgth %i\n",udfilePost.url,udfilePost.DocID,strlen(udfilePost.url));
	
		memset(&key, 0, sizeof(DBT));
                memset(&data, 0, sizeof(DBT));

		//legger inn datane i bdb strukturen
		key.data = udfilePost.url;
		key.size = strlen(udfilePost.url);

		data.data = &udfilePost.DocID;
		data.size = sizeof(udfilePost.DocID);


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

	if ((count % 100000) == 0) {
		printf("komet til %i\n", count);
	}

	count++;
	}

	fclose(UDFILE);

	/* Close everything down. */
        if ((ret = dbp->close(dbp, 0)) != 0) {
                fprintf(stderr, "%s: DB->close: %s\n", progname, db_strerror(ret));
                return (EXIT_FAILURE);
        }


}
