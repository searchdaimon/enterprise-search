#ifdef WITH_ACL

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "acl.h"
#include "../boithoadClientLib/liboithoaut.h"

#define MIN(x,y) ((x) > (y) ? (y) : (x))

int acl_in_list(char *, char **);

void
acl_free_reslist(char **reslist, int n)
{
	int i;

	for (i = 0; i < n; i++)
		free(reslist[i]);
	free(reslist);
}

int
acl_is_allowed(char **allow, char **deny, char *group, char ***groups, int *num)
{
	int i;
	int gotallow = 0;

	printf("Looking up: %s\n", group);

	if (acl_in_list(group, deny))
		return 0;
	if (acl_in_list(group, allow))
		gotallow = 1;

	if (*groups == NULL)
		if (!boithoad_groupsForUser(group, groups, num))
			return 0;

	for (i = 0; i < *num; i++) {
		if (!gotallow) {
			if (acl_in_list((*groups)[i], allow)) {
				gotallow = 1;
			}
		}
		if (acl_in_list((*groups)[i], deny)) {
			return 0;
		}
	}

	//acl_free_reslist(groups, num);
	return gotallow;
}

int
acl_in_list(char *group, char **list)
{
	char *p, *p2;
	unsigned int grouplen;

	for (; *list != NULL; list++) {
		if (strcmp(*list, group) == 0)
			return 1;
	}

	return 0;
}

char **
acl_parse_list(char *list)
{
	char *p, *p2;
	char **acls;
	int aclsize;
	int i;

	aclsize = 0;
	acls = NULL;
	for (p = list, i = 0; p != NULL; p = p2+1, i++) {
		unsigned int len;

		if (aclsize < i+1) {
			if (aclsize == 0)
				aclsize = 4;
			else
				aclsize *= 2;
			acls = realloc(acls, aclsize * sizeof(char *));
			if (acls == NULL)
				return NULL;
		}
		p2 = strchr(p, ',');
		len = p2 == NULL ? strlen(p) : (unsigned int)(p2 - p);
		acls[i] = strndup(p, len);

		if (p2 == NULL) {
			i++;
			break;
		}
	}

	acls = realloc(acls, (i+1) * sizeof(char *));
	acls[i] = NULL;

	return acls;
}

void
acl_destroy(char **list)
{
	int i;

	for(i = 0; list[i] != NULL; i++)
		free(list[i]);
	free(list);
}

#endif
