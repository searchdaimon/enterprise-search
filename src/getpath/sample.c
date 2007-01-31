/* sample.c  -- example of use of getpath.c */

#include "getpath.h"

main()
{
     path_t *file_path;
     char   file_name[81];

     printf("\n Enter a DOS file name: [d:][\path]name[.ext]\n");
     scanf("%s", file_name);
     file_path = getpath(file_name);
     printf("\n file disk = %s\n", file_path->fil_disk);
     printf(" file path = %s\n", file_path->fil_path);
     printf(" file name = %s\n", file_path->fil_name);
     printf(" file ext  = %s\n", file_path->fil_ext);

     exit(0);
}

