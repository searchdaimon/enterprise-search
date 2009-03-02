
#include <stdio.h>
//#include "getfiletype.h"

//#include "../common/boithohome.h"
/*
int main()
{
    filetypes_info	*fti = getfiletype_init(bfile("config/filetypes.eng.conf"));

    printf("Match: %s\n", getfiletype(fti, "pdf"));

    getfiletype_destroy(fti);
}
*/

#include "identify_extension.h"


int main()
{
    struct fte_data	*fdata = fte_init("../../config/file_extensions.conf");

    char	*test_ext[] = {"bmp","docx","doc","dat"};
    int		i;

    for (i=0; i<4; i++)
	{
	    char		*group, *descr, *icon;
	    int			ret;

	    ret = fte_getdescription(fdata, "nbo", test_ext[i], &group, &descr, &icon);
	    printf("Extension(%s): %s / %s, Icon=%s\n", test_ext[i], group, descr, icon);
	}

    fte_destroy(fdata);
}

