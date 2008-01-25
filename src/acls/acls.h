
/**
 *	CHANGELOG:	25.01.2008	Magnus	La til userToSubname_deletecol() for å slette collections.
 */

#ifndef _ACLS__H_
#define _ACLS__H_

#ifdef BLACK_BOKS

#include <db.h>

struct userToSubnameDbFormat {
        DB *dbp;
};

// mode : fil retighet, r for read, w for write
int userToSubname_open(struct userToSubnameDbFormat *userToSubnameDb, char mode);
int userToSubname_getsubnamesAsString(struct userToSubnameDbFormat *userToSubnameDb,char username[],char subnames[],int subnameslen);
int userToSubname_getsubnamesAsSaa(struct userToSubnameDbFormat *userToSubnameDb,char username[],char ***subnames, int *nr);
int userToSubname_close (struct userToSubnameDbFormat *userToSubnameDb);
int userToSubname_deletecol(struct userToSubnameDbFormat *userToSubnameDb,char subname[]);
#endif

#endif // _ACLS__H_
