
#include <stdio.h>
#include "getfiletype.h"


int main()
{
    filetypes_info	*fti = getfiletype_init("filetypes.nbo.conf");

    printf("Match: %s\n", getfiletype(fti, "pdf"));

    getfiletype_destroy(fti);
}
