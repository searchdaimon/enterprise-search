#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../common/bstr.h"
#include "../common/strlcpy.h"
#include "../common/stdlib.h"
#include "../common/define.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "set.h"
#include "acl.h"


int dictionarywordLineSplit(char line[], char word[], unsigned int *nr, char *acl_allow, char *acl_denied) {
	int splits;
	char **data;

	if ((splits = split(line, " ", &data)) < 3) {
		saafree(data);
		return 0;
	}

	strlcpy(word, data[0], maxWordlLen);
	free(data[0]);
	*nr = atou(data[1]);
	free(data[1]);
	strlcpy(acl_allow, data[2], DICT_ACL_LENGTH);
	free(data[2]);
	if (splits == 4) {
		strlcpy(acl_denied, data[3], DICT_ACL_LENGTH);
		free(data[3]);
	} else if (splits == 3) {
		acl_denied[0] = '\0';
	}

	free(data);
	
	return 1;
}



int
add_acls(char *acl, set *s, struct hashtable *aclshash)
{
	char **acls;
	int a, i;

	if (split(acl, ",", &acls) == -1)
		return 0;
	for (i = 0; acls[i] != NULL; i++) {
		char *entry;
		if (strcmp(acls[i], "") == 0) {
			free(acls[i]);
			continue;
		}
		if ((entry = hashtable_search(aclshash, acls[i])) == NULL) {
			entry = acls[i];
			hashtable_insert(aclshash, acls[i], acls[i]);
		} else {
			free(acls[i]);
		}
		if (set_add(s, entry) == 2)
			;
	}
	free(acls);

	return 1;
}

