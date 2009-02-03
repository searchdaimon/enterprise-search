#include <mysql.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../common/define.h"
#include "../common/reposetory.h"
#include "../common/lot.h"
#include "../common/re.h"
#include "../common/boithohome.h"
#include "../common/logs.h"
#include "../common/time.h"

#include "../common/iindex.h"
#include "../common/gc.h"
#include "../common/gcrepo.h"
#include "../bbdocument/bbdocument.h"

/* MYSQL login information */
#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"



struct gcaoptFormat {
	unsigned int MaxAgeDiflastSeen;
	int dryRun;
	FILE *log;
	FILE *logSummary;
	int keept;
	int gced;
	int lastSeenHack;
};

int
gcdecide(int LotNr, char *subname, struct gcaoptFormat *gcaopt, time_t newest_document)
{
	int i;
	
	struct reformat *re;

	FILE *DOCINDEXFH;




	//åpner dokument indeks får å teste at vi har en, hvis ikke kan vi bare avslutte.
        if ( (DOCINDEXFH = lotOpenFileNoCasheByLotNr(LotNr,"DocumentIndex","rb", 's',subname)) == NULL) {
		#ifdef DEBUG
                	printf("lot dont have a DocumentIndex file\n");
		#endif

                return 0;
        }
	fclose(DOCINDEXFH);

	blog(gcaopt->log,1,"Runing gc for collection \"%s\", lot nr %i",subname,LotNr);

	if((re = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_COPYONCLOSE|RE_HAVE_4_BYTES_VERSION_PREFIX)) == NULL) {
		perror("can't reopen()");
		exit(1);
	}



	//går gjenom alle på jakt etter de som kan slettes
	for (i=0;i<NrofDocIDsInLot;i++) {

	
		if (DIS_isDeleted(REN_DocumentIndex(re, i))) {
			continue;
		}

		#ifdef DEBUG
			#ifdef BLACK_BOKS
				printf("dokument \"%s\", lastSeen: %s",
					REN_DocumentIndex(re, i)->Url,
					ctime_s(&REN_DocumentIndex(re, i)->lastSeen));
			#endif
		#endif

		#ifdef BLACK_BOKS
		if (	( (gcaopt->lastSeenHack == 1) && (REN_DocumentIndex(re, i)->lastSeen == 0) )
			|| ( (REN_DocumentIndex(re, i)->lastSeen != 0) && (newest_document > (REN_DocumentIndex(re, i)->lastSeen + gcaopt->MaxAgeDiflastSeen)) )
		) {


			//sletter
			DIS_delete(REN_DocumentIndex(re, i));

			//sletter dokumentet i bb spesefike ting.
			bbdocument_delete (REN_DocumentIndex(re, i)->Url, subname);

			blog(gcaopt->log,2,"dokument \"%s\" can be deleted. Last seen: %s, DocID %u",REN_DocumentIndex(re, i)->Url,ctime_s(&REN_DocumentIndex(re, i)->lastSeen),LotDocIDOfset(LotNr) +i);
			++gcaopt->gced;
		
		} 
		else {
			++gcaopt->keept;
		}
		#endif
	}


	//markerer hva vi kan slette.
	gc_reduce(re, LotNr, subname);

	reclose(re);

	//trunkerer reposetoryet.
	gcrepo(LotNr, subname);


	//vasker iindex
        struct IndekserOptFormat IndekserOpt;
        IndekserOpt.optMustBeNewerThen = 0;
        IndekserOpt.optAllowDuplicates = 0;
        IndekserOpt.optValidDocIDs = NULL;
        IndekserOpt.sequenceMode =1;
        IndekserOpt.garbareCollection = 1;

	for (i=0;i<64;i++) {
		Indekser(LotNr,"Main",i,subname,&IndekserOpt);
	}
	for (i=0;i<64;i++) {
		Indekser(LotNr,"acl_allow",i,subname,&IndekserOpt);
	}
	for (i=0;i<64;i++) {
		Indekser(LotNr,"acl_denied",i,subname,&IndekserOpt);
	}

	//siden vi nå har lagt til alle andringer fra rev index kan vi nå slettet gced filen også
	//Indekser_deleteGcedFile(LotNr, subname);
	lotDeleteFile("gced", LotNr, subname);


	return 0;
}


FILE *lockcoll(char subname[]) {

	char lockfile[512];
	FILE *LOCK;

        //oppretter var mappen hvis den ikke finnes. Dette slik at vi slipper og gjøre dette under instalsjonen
        bmkdir_p(bfile("var/"),0755);

        sprintf(lockfile,"var/boitho-collections-%s.lock",subname);

        printf("locking lock \"%s\"\n",lockfile);

        if ((LOCK = bfopen(lockfile,"w+")) == NULL) {
                perror(lockfile);
                return NULL;
        }

        //geting the lock. 
        if (flock(fileno(LOCK),LOCK_EX) != 0) {
                fclose(LOCK);
                return NULL;
        }


	return LOCK;
        

}

int isOkCrawled (char subname[], struct gcaoptFormat *gcaopt) {

        char mysql_query [2048];
        static MYSQL demo_db;
        MYSQL_RES *mysqlres; /* To be used to fetch information into */
        MYSQL_ROW mysqlrow;
	int nrofcollections;
        int i,y;
	int crawler_success = 0;


        mysql_init(&demo_db);

        //koble til mysql
        if(!mysql_real_connect(&demo_db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, BOITHO_MYSQL_DB, 3306, NULL, 0)){
                printf(mysql_error(&demo_db));
                blog(gcaopt->logSummary,1,"MySQL Error: \"%s\".",mysql_error(&demo_db));
                exit(1);
        }



        sprintf(mysql_query, "select crawler_success from shares where collection_name='%s'",subname);
     
        debug("mysql_query: %s\n",mysql_query);

        if(mysql_real_query(&demo_db, mysql_query, strlen(mysql_query))){ /* Make query */
                printf(mysql_error(&demo_db));
                blog(gcaopt->logSummary,1,"MySQL Error: \"%s\".",mysql_error(&demo_db));

                exit(1);
        }
        mysqlres=mysql_store_result(&demo_db); /* Download result from server */

        nrofcollections = (int)mysql_num_rows(mysqlres);

	if (nrofcollections == 0) {
		blog(gcaopt->logSummary,1,"Skipping \"%s\": dident find any rows",subname);
	}
	else if (nrofcollections != 1) {
		blog(gcaopt->logSummary,1,"Skipping \"%s\": has more then 1 rows",subname);
	}
	else {
		while ((mysqlrow=mysql_fetch_row(mysqlres)) != NULL) { /* Get a row from the results */

                	if (mysqlrow[0] != NULL) {
                        	crawler_success = atoi(mysqlrow[0]);
                	}

		}
	}

	mysql_free_result(mysqlres);
	mysql_close(&demo_db);


	printf("~isOkCrawled(crawler_success=%i)\n",crawler_success);




	if (crawler_success == 1) {
		return 1;
	}
	else {
		return 0;
	}

}

void gc_coll(char subname[], struct gcaoptFormat *gcaopt) {

	int LotNr, i;
	int DocIDcount = 0;
	FILE *LOCK;
	struct reformat *re;

	
	time_t newest_document = 0;

	gcaopt->keept = 0;
	gcaopt->gced = 0;

	if (!isOkCrawled(subname,gcaopt)) {
		blog(gcaopt->logSummary,1,"Skipping \"%s\". Was not correctly crawled.",subname);		
		return;
	}

	if ((LOCK = lockcoll(subname)) == NULL) {
		fprintf(stderr,"Can't lock lockfile!\n");
		exit(-1);
	}
	

	#ifdef BLACK_BOKS

		for(LotNr=1;LotNr<maxLots;LotNr++) {


			if((re = reopen(LotNr, sizeof(struct DocumentIndexFormat), "DocumentIndex", subname, RE_READ_ONLY|RE_HAVE_4_BYTES_VERSION_PREFIX)) == NULL) {
				continue;
			}

			//finner nyeste dokument 
			for (i=0;i<NrofDocIDsInLot;i++) {

				if ((REN_DocumentIndex(re, i)->lastSeen != 0) && (newest_document < REN_DocumentIndex(re, i)->lastSeen)) {
       			                newest_document = REN_DocumentIndex(re, i)->lastSeen;
					//printf("newest_document: i: %i, url \"%s\", time %s\n",i,REN_DocumentIndex(re, i)->Url, ctime_s(&REN_DocumentIndex(re, i)->lastSeen));
        		       	}

			}

			reclose(re);

		}

	#endif


	//hack: setter datoen til i dag. Forutsetter at vi nettopp har kjørt crawling.
	//printf("\n<######################## with runarb newest_document hack###################>\n");
	//newest_document = time(NULL);
	//printf("</######################## with runarb newest_document hack###################>\n\n");

	#ifdef BLACK_BOKS
		blog(gcaopt->log,1,"Newest document: %s",ctime_s(&newest_document));
	#endif


	for(LotNr=1;LotNr<maxLots;LotNr++) {
		gcdecide(LotNr,subname, gcaopt, newest_document);
	}

	/***************************/
	//merger indexene
        //skal lage for alle bøttene
	printf("merging Main\n");
        for (i=0;i<NrOfDataDirectorys;i++) {
		#ifdef DEBUG
        	printf("gc_coll: bucket: %i\n",i);
		#endif
		mergei(i,0,0,"Main","aa",subname,&DocIDcount);
        }

	printf("merging acl_allow\n");
        for (i=0;i<NrOfDataDirectorys;i++) {
		#ifdef DEBUG
        	printf("gc_coll: bucket: %i\n",i);
		#endif
		mergei(i,0,0,"acl_allow","aa",subname,&DocIDcount);
	}

	printf("merging acl_denied\n");
        for (i=0;i<NrOfDataDirectorys;i++) {
		#ifdef DEBUG
        	printf("gc_coll: bucket: %i\n",i);
		#endif
		mergei(i,0,0,"acl_denied","aa",subname,&DocIDcount);
	}

        printf("DocIDcount: %i (/64)\n",DocIDcount);

	/***************************/
	blog(gcaopt->log,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt->keept,gcaopt->gced);
	blog(gcaopt->logSummary,1,"gc'ed \"%s\". Keept %i, gced %i",subname,gcaopt->keept,gcaopt->gced);

	fclose(LOCK);
}

int
main(int argc, char **argv)
{

	int LotNr, i;
	char *subname;
	struct gcaoptFormat gcaopt;

	gcaopt.MaxAgeDiflastSeen  = (86400 * 5); //86400=1 dag
	gcaopt.dryRun  = 0;
	gcaopt.log = NULL;
	gcaopt.logSummary = NULL;
	gcaopt.lastSeenHack = 0;

        extern char *optarg;
        extern int optind, opterr, optopt;
        char c;
        while ((c=getopt(argc,argv,"t:dls"))!=-1) {
                switch (c) {
                        case 't':
                                gcaopt.MaxAgeDiflastSeen  = atou(optarg);
                                break;
			case 'd':
				gcaopt.dryRun = 1;
				break;
			case 'l':
				if ((gcaopt.log = fopen(bfile("logs/gc"),"ab")) == NULL) {
					perror("logs/gc");
					exit(-1);
				}
				if ((gcaopt.logSummary = fopen(bfile("logs/gcSummary"),"ab")) == NULL) {
					perror("logs/gcSummary");
					exit(-1);
				}

				break;
			case 's':
				gcaopt.lastSeenHack = 1;
				break;
                        default:
                                exit(1);
                }

        }
        --optind;



	DIR *ll;


	#ifndef BLACK_BOKS
		fprintf("dette fungerer bare med black boks for nå\n");
		exit(1);
	#endif

	if ((argc -optind) == 2) {
		subname = argv[1 +optind];

		gc_coll(subname, &gcaopt);


	}
	else if ((argc -optind) == 1) {

		ll = listAllColl_start();

		while((subname = listAllColl_next(ll)) != NULL) {
			printf("indexing collection \"%s\"\n",subname);

			gc_coll(subname, &gcaopt);

		}

		listAllColl_close(ll);

	}
	else {
		errx(1, "Usage: ./gcrepo subname [ lotnr ]");
	}



	return 0;
}

