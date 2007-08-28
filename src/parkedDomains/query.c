
#define _GNU_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include <db.h>

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"


static unsigned int
hashfromkey_url(void *k)
{
	char *url = k;
	char *str;
	//unsigned int a = *(unsigned int *)k;
	unsigned int hash;

#if 1
	for (str = url, hash = strlen(url); *str != '\0'; str++) 
		hash = (33*hash)+(*str);
	return hash;
#endif
	//return a;
}

static int
equalkeys_url(void *k1, void *k2)
{
	char *a1, *a2;

	a1 = (char *)k1;
	a2 = (char *)k2;

	return (strcmp(a1, a2) == 0);
	//return (a1 == a2);
}


struct dbkey {
	unsigned int num;
	unsigned long int addr;
};

DB *maindb;

#define THEDB "db/maindb"

int fakeval = 1;

void
print_distinct(int cutoff)
{
	struct hashtable *hash;
	DBC *dbcp;
	DBT key, data;
	int ret;

	//create_hashtable(1, hashfromkey_url, equalkeys_url);


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
		struct dbkey dbkey2;
		struct dbkey dbkey3;
		unsigned int num;
		unsigned int i;
		char *url;
		DBT key2, data2;
		DBT key3, data3;
		char url2[10240];

		num = *(unsigned int *)data.data;
		memcpy(&dbkey, key.data, key.size);

		if (dbkey.num != 0)
			continue;
		in.s_addr = dbkey.addr;

		if (num < cutoff)
			continue;
		printf("Got %d record for %s\n", num, inet_ntoa(in));
#if 0
		hash = create_hashtable(10, hashfromkey_url, equalkeys_url);
		for (i = 1; i <= num; i++) {
			char *hashkey;
			dbkey2.addr = in.s_addr;
			if (num > 1)
				dbkey2.num = 2;
			else
				dbkey2.num = i;

			memset(&key2, 0, sizeof(key2));
			key2.data = &dbkey2;
			key2.size = sizeof(dbkey2);
			memset(&data2, 0, sizeof(data2));
			//data2.data = url2;
			//data2.size = sizeof(url2);

			if ((ret = maindb->get(maindb, NULL, &key2, &data2, 0) != 0)) {
				maindb->err(maindb, ret, "DBcursor->get");
				exit(1);
			}

			strncpy(url2, data2.data, data2.size);
			url2[data2.size] = '\0';
			//printf("\tFound: %d\n", data2.size);
			//printf("\tFound: %s\n", url2);

			hashkey = strdup(url2);
			if (hashkey == NULL)
				err(1, "Oops, hashkey is null");
			if (!hashtable_search(hash, url2)) {
				hashkey = strdup(url2);
				if (hashkey == NULL)
					err(1, "Oops, hashkey is null");

				hashtable_insert(hash, hashkey, &fakeval);
			}

		}
		if (num != hashtable_count(hash) && hashtable_count(hash) != 1) {
			printf("%d is really %d\n", num, hashtable_count(hash));
		}
		hashtable_destroy(hash, 0);
#endif
	}
	if (ret != DB_NOTFOUND) {
		maindb->err(maindb, ret, "DBcursor->c_get");
		exit(1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0)
		maindb->err(maindb, ret, "DBcursor->close");

}



void
print_all(void)
{
	struct hashtable *hash;
	DBC *dbcp;
	DBT key, data;
	int ret;

	create_hashtable(1, hashfromkey_url, equalkeys_url);


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
		struct dbkey dbkey2;
		struct dbkey dbkey3;
		unsigned int num;
		unsigned int i;
		char *url;
		DBT key2, data2;
		DBT key3, data3;
		char url2[10240];

		num = *(unsigned int *)data.data;
		memcpy(&dbkey, key.data, key.size);

		if (dbkey.num != 0)
			continue;
		in.s_addr = dbkey.addr;

		printf("Got %d record for %s\n", num, inet_ntoa(in));
		for (i = 1; i <= num; i++) {
			char *hashkey;
			dbkey2.addr = in.s_addr;
			if (num > 1)
				dbkey2.num = 2;
			else
				dbkey2.num = i;

			memset(&key2, 0, sizeof(key2));
			key2.data = &dbkey2;
			key2.size = sizeof(dbkey2);
			memset(&data2, 0, sizeof(data2));
			//data2.data = url2;
			//data2.size = sizeof(url2);

			if ((ret = maindb->get(maindb, NULL, &key2, &data2, 0) != 0)) {
				maindb->err(maindb, ret, "DBcursor->get");
				exit(1);
			}

			strncpy(url2, data2.data, data2.size);
			url2[data2.size] = '\0';
			//printf("\tFound: %d\n", data2.size);
			printf("\tFound: %s\n", url2);
		}
	}
	if (ret != DB_NOTFOUND) {
		maindb->err(maindb, ret, "DBcursor->get");
		exit(1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0)
		maindb->err(maindb, ret, "DBcursor->close");

}




#define dbCashe 314572800+314572800       //300 mb
#define dbCasheBlokes 1         //hvor mange deler vi skal dele cashen opp i. Ofte kan man ikke alokere store blikker samenhengenede


int
main(int argc, char **argv)
{
	int ret;

	if (argc < 2)
		errx(1, "Usage: ./prog command");

	if ((ret = db_create(&maindb, NULL, 0)) != 0) {
		err(1, "db_create: %s\n", db_strerror(ret));
	}

	if ((ret = maindb->set_cachesize(maindb, 0, dbCashe, dbCasheBlokes)) != 0) {
		maindb->err(maindb, ret, "set_cachesize");
	}


	if ((ret = maindb->open(maindb, NULL, THEDB, NULL, DB_BTREE, DB_RDONLY, 0664)) != 0) {
		maindb->err(maindb, ret, "%s", "db/maindb");
		exit(1);
	}

	if (strcmp(argv[1], "distinct") == 0) {
		int cutoff = 10;

		if (argc >= 3)
			cutoff = atoi(argv[2]);
		print_distinct(cutoff);
	} else if (strcmp(argv[1], "all") == 0) {
		print_all();
	} else if (strcmp(argv[1], "list") == 0) {
		printf("Commands:\n\n");
		printf("distinct [cutoff]\n");
	}



	if (maindb->close(maindb, 0) != 0)
		return 1;

	
	return 0;
}
