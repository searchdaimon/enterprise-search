/*
 *
 *
 */

#define _GNU_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include <db.h>

#include "../common/define.h"
#include "../common/crc32.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

#define TMPFILE "tmp/collected.urls"


struct dbkey {
	unsigned int num;
	unsigned long int addr;
};

FILE *cfp;

void
insert_url(DB *dbp, unsigned long int addr, unsigned int num, char *url)
{
	struct dbkey dbkey;
	char *p, *p2;
	unsigned int *hash;
	DBT key, data;
	int ret;
	
	if ((p = strstr(url, "://")) == NULL) {
		warnx("Not a valid url: %s", url);
		return;
	}
	p += 3;
	p2 = strchr(p, '/');

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	dbkey.addr = addr;
	dbkey.num = num;
	key.data = &dbkey;
	key.size = sizeof(dbkey);
	data.data = p;
	if (p2 == NULL) {
		data.size = strlen(p)+1;
	} else {
		p[p2-p] = '\0';
		data.size = p2-p;
	}
	//printf("Url: %s\n", p);
	//data.data = &num;
	//data.size = sizeof(num);

	if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
		//printf("db: %d: key stored.\n", *(unsigned long int *)key.data);
	} else {
		dbp->err(dbp, ret, "DB->put");
		//exit(1);
	}

}

#define NUMDI 128

void
traverse_dictionary(char *filename, DB *dbp)
{
	FILE *fp;
	struct DocumentIndexFormat documentIndex[NUMDI];
	int n, ret, sum;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		warn("Unable to open %s", filename);
		return;
	}

	n = 0;
	sum = 0;
	do {
		struct hashtable *urlhash;
		struct in_addr in;
		unsigned int num;
		DBT key, data;
		struct dbkey dbkey;

		if (n == 0) {
			if ((n = fread(documentIndex, sizeof(struct DocumentIndexFormat), NUMDI, fp)) == 0) {
				if (feof(fp)) {
					break;
				}
				err(1, "Unable to read from documentIndex");
			}
			sum += n;
		}
		n--;

		if (documentIndex[n].IPAddress == 0 || documentIndex[n].Url[0] == '\0')
			continue;

		//in.s_addr = documentIndex[n].IPAddress;
		//printf("%s %s\n", inet_ntoa(in), documentIndex[n].Url);

		num = 0;
		memset(&key, 0, sizeof(key));
		memset(&data, 0, sizeof(data));
		dbkey.addr = documentIndex[n].IPAddress;
		dbkey.num = 0;
		key.data = &dbkey;
		key.size = sizeof(dbkey);
		//data.data = &num;
		//data.size = sizeof(num);

		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) {
			int anum = *(unsigned int *)data.data;
			//printf(" %d\n", num);// documentIndex[n].IPAddress, num);
			in.s_addr = dbkey.addr; //*(unsigned long int *)key.data;
			//printf("db: %s: key retrieved: data was %d.\n", inet_ntoa(in), anum); 

#if 1
			memset(&key, 0, sizeof(key));
			memset(&data, 0, sizeof(data));
			dbkey.addr = documentIndex[n].IPAddress;
			dbkey.num = 0;
			key.data = &dbkey;
			key.size = sizeof(dbkey);
			data.data = &num; 
			data.size = sizeof(num);
#endif

#if 0
			if ((ret = dbp->del(dbp, NULL, &key, 0)) == 0) {
				//printf("db: %s: key was deleted.\n", (char *)key.data);
			} else {
				dbp->err(dbp, ret, "DB->del");
				exit(1);
			}
#endif

			anum++;
			memset(&key, 0, sizeof(key));
			memset(&data, 0, sizeof(data));
			dbkey.addr = documentIndex[n].IPAddress;
			dbkey.num = 0;
			key.data = &dbkey;
			key.size = sizeof(dbkey);
			data.data = &anum; 
			data.size = sizeof(anum);

			if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
				//printf("db: %d: key stored.\n", *(unsigned long int *)key.data);
			} else {
				dbp->err(dbp, ret, "DB->put");
				//exit(1);
			}
			insert_url(dbp, documentIndex[n].IPAddress, anum, documentIndex[n].Url);
		} else if (ret == DB_NOTFOUND) {
			int anum = 1;

#if 1
			memset(&key, 0, sizeof(key));
			memset(&data, 0, sizeof(data));
			dbkey.addr = documentIndex[n].IPAddress;
			dbkey.num = 0;
			key.data = &dbkey;
			key.size = sizeof(dbkey);
#endif
			data.data = &anum; 
			data.size = sizeof(anum);
			if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
				//printf("db: %d: key stored.\n", *(unsigned long int *)key.data);
			} else {
				dbp->err(dbp, ret, "DB->put");
				//exit(1);
			}
			insert_url(dbp, documentIndex[n].IPAddress, 1, documentIndex[n].Url);
		} else {
			dbp->err(dbp, ret, "DB->get");
			//exit(1);
		} 


#if 0
		if (strcmp(inet_ntoa(in), "40.180.19.64") == 0)
			fprintf(stdout, "%s %lu\n", documentIndex[n].Url, documentIndex[n].IPAddress);
		continue;
#endif

#if 0
		/* Lookup ip */
		urlhash = hashtable_search(iphash, &documentIndex[n].IPAddress);
		if (urlhash == NULL) {
			key = malloc(sizeof(*key));
			if (key == NULL)
				err(1, "Unable to allocate space for ip key");
			urlhash = create_hashtable(1, hashfromkey_url, equalkeys_url);
			if (urlhash == NULL)
				err(1, "Unable to allocate space for url hash");

			*key = documentIndex[n].IPAddress;
			if (!hashtable_insert(iphash, key, urlhash))
				err(1, "Unable to insert ip into hash");

			insert_url(urlhash, documentIndex[n].Url);

		} else {
			insert_url(urlhash, documentIndex[n].Url);
		}
#endif
	} while (1);
	printf("Found: %d\n", sum);

	fclose(fp);
}

#define dbCashe 314572800+314572800       //300 mb
#define dbCasheBlokes 1         //hvor mange deler vi skal dele cashen opp i. Ofte kan man ikke alokere store blikker samenhengenede

int
main(int argc, char **argv)
{
	int i, ret;
	DB *maindb;
	DBC *dbcp;
	DBT key, data;

	if (argc < 2) {
		fprintf(stderr, "Usage: ./prog dictionaryIndex\n");
		return 1;
	}

#if 0
	cfp = fopen(TMPFILE, "a");
	if (cfp == NULL)
		err(1, "Unable to open %s for appending", TMPFILE);
#endif
	
	if ((ret = db_create(&maindb, NULL, 0)) != 0) {
		err(1, "db_create: %s\n", db_strerror(ret));
	}
	if ((ret = maindb->set_cachesize(maindb, 0, dbCashe, dbCasheBlokes)) != 0) {
		maindb->err(maindb, ret, "set_cachesize");
	}


	if ((ret = maindb->open(maindb, NULL, "db/maindb", NULL, DB_BTREE, DB_CREATE|DB_EXCL, 0664)) != 0) {
	//if ((ret = maindb->open(maindb, NULL, "db/maindb", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
	//if ((ret = maindb->open(maindb, NULL, "db/maindb", NULL, DB_BTREE, DB_RDONLY, 0664)) != 0) {
		maindb->err(maindb, ret, "%s", "db/maindb");
		exit(1);
	}

#if 1
	for (i = 1; i < argc; i++) {
		int urls;

		fprintf(stderr, "Traversing: %s\n", argv[i]);

		traverse_dictionary(argv[i], maindb);

	}

	//fclose(cfp);



#endif

#if 0

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	/* Acquire a cursor for the database. */
	if ((ret = maindb->cursor(maindb, NULL, &dbcp, 0)) != 0) {
		maindb->err(maindb, ret, "DB->cursor");
		exit(1);
	}

	/* Walk through the database and print out the key/data pairs. */
	while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		struct in_addr in;
		struct dbkey dbkey;
		unsigned int num;


		num = *(unsigned int *)data.data;
		memcpy(&dbkey, key.data, key.size);
		
		if (dbkey.num != 0)
			continue;
		in.s_addr = dbkey.addr;
		printf("Got %d record for %s\n", num, inet_ntoa(in));
	}
	if (ret != DB_NOTFOUND) {
		maindb->err(maindb, ret, "DBcursor->get");
		exit(1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0)
		maindb->err(maindb, ret, "DBcursor->close");
#endif

	if (maindb->close(maindb, 0) != 0)
		return 1; 

	return 0;
}

