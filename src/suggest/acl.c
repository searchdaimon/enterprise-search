#ifdef WITH_ACL

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "acl.h"
#include "../boithoadClientLib/boithoad.h"
#if defined US_BOITHOAD
#include "../boithoadClientLib/liboithoaut.h"
#elif defined USERSYSTEM
#include "../crawlManager2/client.h"
#elif !defined WITH_ACL

#else
//#error "Unknown usersystem type"
#endif
#include "../3pLibs/keyValueHash/hashtable.h"

#define MIN(x,y) ((x) > (y) ? (y) : (x))

int acl_in_list(char *, char **);

void
acl_free_reslist(char **reslist, int n)
{
	int i;

	if (reslist == NULL) {
		fprintf(stderr,"Error: acl_free_reslist(reslist=%p, reslist=%d): reslist is NULL\n", reslist, n);
		return;
	}

	for (i = 0; i < n; i++)
		free(reslist[i]);
	free(reslist);
}

#ifdef USERSYSTEM
int
acl_usersystem(char *group, char ***groupsout, int *num)
{
	int s, i;
	int port;
	char buf[1024];
	int usersystem = 0; //usersystem 0 is primary system
	char **_gs;
	char **groups;

	port = 7392;
	cmc_conect(&s, buf, sizeof(buf), port);

	*num = cmc_groupsforuserfromusersystem(s, group, usersystem, &_gs, "");
	printf("Found %d groups in user susyem %d\n",*num, usersystem);

	groups = calloc(*num, sizeof(char *));
	for (i = 0; i < *num; i++) {
		groups[i] = strdup((char *)_gs + (i*MAX_LDAP_ATTR_LEN));
		printf("Group: \"%s\"\n", groups[i]);
	}
	*groupsout = groups;

	cmc_close(s);

	return 1;
}
#endif

int
acl_is_allowed(char **allow, char **deny, char *group, char ***groups, int *num)
{
	int i;
	int gotallow = 0;

	if (acl_in_list(group, deny))
		return 0;
	if (acl_in_list(group, allow))
		gotallow = 1;

	if (*groups == NULL) {
#ifdef US_BOITHOAD
		if (!boithoad_groupsForUser(group, groups, num))
			return 0;
#elif defined USERSYSTEM
		if (!acl_usersystem(group, groups, num))
			return 0;
#else
		return 0;
#endif
	}

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
acl_parse_list(char *list, struct hashtable *aclshash)
{
	char *p, *p2;
	char **acls;
	int aclsize;
	int i;

	aclsize = 0;
	acls = NULL;
	for (p = list, i = 0; p != NULL; p = p2+1, i++) {
		unsigned int len;
		char *aclentry, *newentry;

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
		aclentry = strndup(p, len);
		if ((newentry = hashtable_search(aclshash, aclentry)) == NULL) {
			newentry = aclentry;
			hashtable_insert(aclshash, newentry, newentry);
		} else {
			free(aclentry);
		}
		acls[i] = newentry;

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
