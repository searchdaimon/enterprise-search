
#ifndef _ACLS__H_
#define _ACLS__H_

#ifdef BLACK_BOKS

#include <db.h>

struct userToSubnameDbFormat {
        DB *dbp;
};

int userToSubname_open(struct userToSubnameDbFormat *userToSubnameDb);
int userToSubname_getsubnamesAsString(struct userToSubnameDbFormat *userToSubnameDb,char username[],char subnames[],int subnameslen);
int userToSubname_getsubnamesAsSaa(struct userToSubnameDbFormat *userToSubnameDb,char username[],char ***subnames, int *nr);
int userToSubname_close (struct userToSubnameDbFormat *userToSubnameDb);

#endif

#endif // _ACLS__H_
