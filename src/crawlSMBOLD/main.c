
#include <stdlib.h>
#include <stdio.h>

#include "crawlsmb.h"

#include "../boitho-bbdn/bbdnclient.h"


int main( int argc, char *argv[] )
{
    char	*prefix;

    if (argc != 5)
	{
	    printf("usage ./crawlSMB collection username password path\n");
	    exit(1);
	}
	struct bbdnFormat bbdh;

#ifndef NO_BB
    //bbdocument_init();


	if (!bbdn_conect(&bbdh,"")) {
                printf("cant conetc to bbdh.\n");
                exit(1);
	}
#endif

    prefix = smb_mkprefix( argv[2], argv[3] );
/*
    // Eksempel på bruk av smb_test_open:

    if (smb_test_open(prefix, argv[4]))
	{
	    printf("User %s has access to %s\n", argv[2], argv[4]);
	}
    else
	{
	    printf("User %s has NOT access to %s\n", argv[2], argv[4]);
	}
*/
    smb_recursive_get( argv[1], prefix, argv[4],&bbdh );
    free(prefix);
}
