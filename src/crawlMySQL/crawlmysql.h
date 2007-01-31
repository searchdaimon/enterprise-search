int mysql_recurse (char host[], char user[], char password[],char dbname[],
        struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd));

