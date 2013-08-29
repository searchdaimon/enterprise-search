#include <stdio.h>

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
	    char		*group, *descr, *icon, *version;
	    int			ret;

	    ret = fte_getdescription(fdata, "nbo", test_ext[i], &group, &descr, &icon, &version);
	    printf("Extension(%s): %s / %s, Icon=%s, Version=%s\n", test_ext[i], group, descr, icon, version);
	}

    fte_destroy(fdata);
}

