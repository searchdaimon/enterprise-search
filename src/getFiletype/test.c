
#include <stdio.h>
#include "getfiletype.h"

#include "../common/boithohome.h"

int main()
{
    filetypes_info	*fti = getfiletype_init(bfile("config/filetypes.eng.conf"));

    printf("Match: %s\n", getfiletype(fti, "pdf"));

    getfiletype_destroy(fti);
}
