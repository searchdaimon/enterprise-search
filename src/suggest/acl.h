#ifdef WITH_ACL
/*
 *
 */

struct hashtable;

int acl_is_allowed(char **, char **, char *, char ***, int *);
char **acl_parse_list(char *, struct hashtable *);

#endif
