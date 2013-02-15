#ifndef _USERSYSTEM_H_
#define _USERSYSTEM_H_

#define US_TYPE_INHERIT		0
#define US_TYPE_AD		1
#define US_TYPE_SUPEROFFICE	2
#define US_TYPE_SQLBB		3
#define US_TYPE_MAPBACK		4
#define US_TYPE_SHELL		5

/* The first usersystem type the users can use, all below are reserved for SD */
#define US_TYPE_FIRST_USER	100000

struct hashtable;

struct _usersystem_container;

typedef struct {
	unsigned int id;
	char is_primary;
	unsigned int type;
	struct hashtable *parameters;
	struct _usersystem_container *usc;
} usersystem_data_t;

typedef struct {
	unsigned int type;

	int (*us_authenticate)(usersystem_data_t *data, char *user, char *password);
	int (*us_listUsers)(usersystem_data_t *data, char ***users, int *n_users);
	int (*us_listGroupsForUser)(usersystem_data_t *data, const char *user, char ***groups, int *n_groups);
	char *(*us_getName)(void *);
} usersystem_t;

typedef struct {
	unsigned int type;
	char *perlpath;

	usersystem_t *us;
} usersystem_perl_t;

typedef struct _usersystem_container {
	enum {
		USC_TYPE_C,
		USC_TYPE_PERL,
	} moduletype;
	union {
		usersystem_t *us_c;
		usersystem_perl_t *us_perl;
	} usersystem;
} usersystem_container_t;

int us_authenticate_perl(usersystem_data_t *data, char *user, char *password);
int us_listUsers_perl(usersystem_data_t *data, char ***users, int *n_users);
int us_listGroupsForUser_perl(usersystem_data_t *data, const char *user, char ***groups, int *n_groups);
char *us_getName_perl(void *data);

// an structure used for C calback, if we need it in the future.
struct us_cargsF {
        usersystem_data_t *data;
};


#endif /* _USERSYSTEM_H_ */
