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

#if 0
struct dburlkey {
	unsigned long int addr;
	char url[128];
};

#else

struct dburlkey {
	unsigned long int addr;
	unsigned int hash;
};

#endif



DB *urldb;


FILE *cfp;
FILE *log;

int
insert_url(DB *dbp, unsigned long int addr, char *url)
{
	struct dburlkey dbkey;
	char *p, *p2;
	unsigned int *hash;
	DBT key, data;
	DBT data2;
	int ret;
	int some;
	struct in_addr in;
	char url2[1024];
	
	if ((p = strstr(url, "://")) == NULL) {
		warnx("Not a valid url: %s", url);
		return 0;
	}
	p += 3;
	p2 = strchr(p, '/');

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	memset(&data2, 0, sizeof(data));
	dbkey.addr = addr;
#if 0
	if (p2 == NULL) {
		strcpy(dbkey.url, p);
	} else {
		strncpy(dbkey.url, p, p2-p);
		dbkey.url[p2-p] = '\0';
	}
	key.data = &dbkey;
	key.size = sizeof(dbkey.addr) + strlen(dbkey.url)+1;//sizeof(dbkey);
#else
	if (p2 == NULL) {
		strcpy(url2, p);
	} else {
		strncpy(url2, p, p2-p);
		url2[p2-p] = '\0';
	}
	dbkey.hash = crc32boitho(url2);
	key.data = &dbkey;
	key.size = sizeof(dbkey);
#endif

	some = 1;
	data.data = &some;
	data.size = sizeof(some);
#if 0
	data.data = p;
	if (p2 == NULL) {
		data.size = strlen(p)+1;
	} else {
		p[p2-p] = '\0';
		data.size = p2-p;
	}
#endif
	//printf("Url: %s\n", p);
	//data.data = &num;
	//data.size = sizeof(num);

	if ((ret = dbp->get(dbp, NULL, &key, &data2, 0)) == 0) {
		return 0;
	} else if (ret == DB_NOTFOUND) {
		in.s_addr = addr;
#if 0
		fprintf(log, "%s %s\n", inet_ntoa(in), dbkey.url);
#else
		fprintf(log, "%s %s\n", inet_ntoa(in), url2);
#endif
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0) {
			//printf("db: %d: key stored.\n", *(unsigned long int *)key.data);
		} else {
			dbp->err(dbp, ret, "DB->put");
			return 0;
			//exit(1);
		}
	} else {
		dbp->err(dbp, ret, "DB->get");

		return 0;
	}

	return 1;
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
		char ip[1024];

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

#if 0
		in.s_addr = documentIndex[n].IPAddress;
		//printf("%s %s\n", inet_ntoa(in), documentIndex[n].Url);
		strcpy(ip, inet_ntoa(in));
		if (strcmp(ip, "217.160.226.73") == 0)
			printf("%s\n", documentIndex[n].Url);
#endif

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

	
			if (insert_url(urldb, documentIndex[n].IPAddress, documentIndex[n].Url) == 0) {
				continue;
			}

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
			insert_url(urldb, documentIndex[n].IPAddress, documentIndex[n].Url);
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

#define dbCashe 4*314572800  //300 mb
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


#if 1
	log = fopen("db/log.urls", "w");
	if (log == NULL)
		err(1, "Unable to open %s for appending", "db/log.urls");
#endif


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

	
	if ((ret = db_create(&urldb, NULL, 0)) != 0) {
		err(1, "db_create: %s\n", db_strerror(ret));
	}
	if ((ret = urldb->set_cachesize(urldb, 0, dbCashe, dbCasheBlokes)) != 0) {
		urldb->err(urldb, ret, "set_cachesize");
	}


	if ((ret = urldb->open(urldb, NULL, "db/urldb", NULL, DB_BTREE, DB_CREATE|DB_EXCL, 0664)) != 0) {
	//if ((ret = urldb->open(urldb, NULL, "db/urldb", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
	//if ((ret = urldb->open(urldb, NULL, "db/urldb", NULL, DB_BTREE, DB_RDONLY, 0664)) != 0) {
		urldb->err(urldb, ret, "%s", "db/urldb");
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

	fclose(log);

	if (maindb->close(maindb, 0) != 0)
		return 1; 

	if (urldb->close(urldb, 0) != 0)
		return 1; 


	return 0;
}

