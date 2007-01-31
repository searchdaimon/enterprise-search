
#ifndef _CRAWLSMB_H_
#define _CRAWLSMB_H_


#include <libsmbclient.h>

#include "../boitho-bbdn/bbdnclient.h"

char* smb_mkprefix( char *username, char *passwd );

void smb_recursive_get( char *collection, char *prefix, char *dir_name,struct bbdnFormat *bbdh );

int smb_test_open( char *prefix, char *dir_name );


#endif	// _CRAWLSMB_H_
