#ifndef _DICTIONARYWORDSLOT_ACL_H_
#define _DICTIONARYWORDSLOT_ACL_H_

#include "set.h"

#define DICT_ACL_LENGTH 1024

struct hashtable;

int add_acls(char *acl, set *s, struct hashtable *aclshash);
int dictionarywordLineSplit(char line[], char word[], unsigned int *nr, char *acl_allow, char *acl_denied);

#endif
