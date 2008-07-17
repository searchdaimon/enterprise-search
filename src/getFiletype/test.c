
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
	    char		*group, *descr;
	    int			ret;

	    ret = fte_getdescription(fdata, "nbo", test_ext[i], &group, &descr);
	    printf("Group(%s): %s, Description: %s\t(%i)\n", test_ext[i], group, descr, ret);
	}

    char	*test_groups[] = {"Bilde","Word","Document","Other"};

    for (i=0; i<4; i++)
	{
	    char		**ptr1, **ptr2, **ptr_i;

	    if (fte_getextension(fdata, "xxx", test_groups[i], &ptr1, &ptr2))
		{
		    printf("Ext(%s):", test_groups[i]);
		    for (ptr_i=ptr1; ptr_i<ptr2; ptr_i++) printf(" %s", *ptr_i);
		    printf("\n");
		}
	    else printf("Ext(%s): <none>\n", test_groups[i]);
	}


    char	*test_ext2[] = {"htm","doc","docx","jpeg","tix"};

    for (i=0; i<5; i++)
	{
	    char		**ptr1, **ptr2, **ptr_i;

	    if (fte_getext_from_ext(fdata, test_ext2[i], &ptr1, &ptr2))
		{
		    printf("Ext(%s):", test_ext2[i]);
		    for (ptr_i=ptr1; ptr_i<ptr2; ptr_i++) printf(" %s", *ptr_i);
		    printf("\n");
		}
	    else printf("Ext(%s): <none>\n", test_ext2[i]);
	}

    fte_destroy(fdata);
}
