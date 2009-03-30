#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../crawlManager2/usersystem.h"

int
mapback_list_users(usersystem_data_t *data, char ***users, int *n_users)
{
	*users = malloc(1 * sizeof(char *));
	if (*users == NULL)
		return 0;
	(*users)[0] = NULL;
	*n_users = 0;

	return 1;
}

int
mapback_list_groupsforuser(usersystem_data_t *data, const char *user, char ***groups, int *n_groups)
{
	char *group, *p;
	size_t n;

	*groups = malloc(3 * sizeof(char *));
	if (*groups == NULL)
		return 0;
	(*groups)[0] = strdup(user);
	group = strdup(user);
	p = strchr(group, '-');
	n = 1;
	if (p != NULL) {
		*p = '\0';
		(*groups)[n] = group;
		n++;
	}
		
	(*groups)[n] = NULL;
	*n_groups = n;

	return 1;
}

/* XXX */
int
mapback_authenticate_user(usersystem_data_t *data, char *user, char *password)
{
	return 0;
}

char *
mapback_get_name(void)
{
	        return strdup("MapBack");
}

usersystem_t usersystem_info = {
	US_TYPE_MAPBACK,
	NULL,
	mapback_list_users,
	mapback_list_groupsforuser,
	mapback_get_name,
};
