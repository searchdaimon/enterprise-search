#ifndef _USERSYSTEM_H_
#define _USERSYSTEM_H_

#define US_TYPE_AD		1
#define US_TYPE_SUPEROFFICE	2
#define US_TYPE_SQLBB		3

struct hashtable;

typedef struct {
	unsigned int id;
	char *hostname;
	char *username;
	char *password;
	char is_primary;
	unsigned int type;
	struct hashtable *parameters;
} usersystem_data_t;

typedef struct {
	unsigned int type;

	int (*us_authenticate)(usersystem_data_t *data, char *user, char *password);
	int (*us_listUsers)(usersystem_data_t *data, char ***users, int *n_users);
	int (*us_listGroupsForUser)(usersystem_data_t *data, const char *user, char ***groups, int *n_groups);
} usersystem_t;

#endif /* _USERSYSTEM_H_ */
