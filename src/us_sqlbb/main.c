#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

#include "../crawlManager2/usersystem.h"

void sql_connect(MYSQL *db);

int
sqlbb_list_users(usersystem_data_t *data, char ***users, int *n_users)
{
	char query[1024];
	size_t querylen;
	MYSQL db;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int i;

	sql_connect(&db);
	querylen = snprintf(query, sizeof(query), "SELECT DISTINCT username FROM foreignUserSystem WHERE usersystem = %d",
			data->id);
	if (mysql_real_query(&db, query, querylen)) {
		fprintf(stderr, "Failed to remove rows, Error: %s\n", mysql_error(&db));
		*n_users = 0;
		mysql_close(&db);
		return 0;
	}

	res = mysql_store_result(&db);
	*n_users = mysql_num_rows(res);

	*users = malloc(((*n_users)+1) * sizeof(char *));

	i = 0;
	while ((row = mysql_fetch_row(res)) != NULL) {
		(*users)[i] = strdup(row[0]);
		i++;
	}
	(*users)[i] = NULL;

	mysql_free_result(res);
	mysql_close(&db);

	return 1;
}

int
sqlbb_list_groupsforuser(usersystem_data_t *data, const char *user, char ***groups, int *n_groups)
{
	char query[1024];
	size_t querylen;
	MYSQL db;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int i;

	sql_connect(&db);
	querylen = snprintf(query, sizeof(query), "SELECT groupname FROM foreignUserSystem WHERE usersystem = %d AND username = '%s'",
			data->id, user);
	if (mysql_real_query(&db, query, querylen)) {
		fprintf(stderr, "Failed to remove rows, Error: %s\n", mysql_error(&db));
		*n_groups= 0;
		mysql_close(&db);
		return 0;
	}

	res = mysql_store_result(&db);
	*n_groups = mysql_num_rows(res);

	*groups = malloc(((*n_groups)+1) * sizeof(char *));

	i = 0;
	while ((row = mysql_fetch_row(res)) != NULL) {
		(*groups)[i] = strdup(row[0]);
		i++;
	}
	(*groups)[i] = NULL;

	mysql_free_result(res);
	mysql_close(&db);

	return 1;

}

/* XXX */
int
sqlbb_authenticate_user(usersystem_data_t *data, char *user, char *password)
{
	return 0;
}

usersystem_t usersystem_info = {
	US_TYPE_SQLBB,
	NULL,
	sqlbb_list_users,
	sqlbb_list_groupsforuser,
};
