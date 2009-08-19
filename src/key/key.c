#include <mysql.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include "key.h"


/* MYSQL login information */
#define MYSQL_HOST	"localhost"
#define MYSQL_USER	"boitho"
#define MYSQL_PASS	"G7J7v5L5Y7"
#define MYSQL_DB	"boithobb"

int key_get_existingconn(void *db, char *keyout) {
	MYSQL_ROW row;
	MYSQL_RES *res;

	size_t qlen;
	char query[2048];

	qlen = snprintf(query, sizeof(query), "SELECT configvalue FROM config WHERE configkey = 'key'");

	if (mysql_real_query(db, query, qlen))
		errx(1, "Unable to perform query: %s: %s", query, mysql_error(db));

	res = mysql_store_result(db);
	row = mysql_fetch_row(res);
	if (row == NULL || mysql_num_rows(res) < 1)
		errx(1, "No key found");
		
	strlcpy(keyout, row[0], KEY_STR_LEN);

	return 1;
}

int
key_get(char *keyout)
{
	MYSQL db;

	if (!mysql_init(&db))
		errx(1, "mysql_init error");

	if (!mysql_real_connect(&db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 3306, NULL, 0))
		errx(1, "Unable to connect to mysql: %s", mysql_error(&db));

	int ret = key_get_existingconn(&db, keyout);
	mysql_close(&db);

	return ret;
}

int
key_equal(char *k1, char *k2)
{
	return (strcmp(k1, k2) == 0);
}
