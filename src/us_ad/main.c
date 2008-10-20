#include <stdio.h>

#include "../boithoadClientLib/liboithoaut.h"

#include "../crawlManager2/usersystem.h"

/*
 * XXX: Merge boithoad into this module
 */

int
ad_list_users(usersystem_data_t *data, char ***users, int *n_users)
{
	return boithoad_listUsers(users, n_users);
}

int
ad_list_groupsforuser(usersystem_data_t *data, const char *user, char ***groups, int *n_groups)
{
	return boithoad_groupsForUser(user, groups, n_groups);
}

/* XXX */
int
ad_authenticate_user(usersystem_data_t *data, char *user, char *password)
{
	return 0;
}

usersystem_t usersystem_info = {
	US_TYPE_AD,
	ad_authenticate_user,
	ad_list_users,
	ad_list_groupsforuser,
};
