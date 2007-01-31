int crawlLocalFiles (
        struct collectionFormat *collection,
        int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat*crawldocumentExist),
        int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd),
        char prefix[],
        char path[]
        );
