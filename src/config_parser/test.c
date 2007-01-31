
#include <stdio.h>
#include "config_parser.h"

int main( int argc, char *argv[] )
{
    int		i;

    for (i=1; i<argc; i++)
	{
	    if (read_config(argv[i]) == -1)
		{
		    printf("Error opening \"%s\".\n", argv[i]);
		    return -1;
		}
	}

    printf("%10s == '%s'\n", "sju", config_value("sju") );
    printf("%10s == %i\n", "sju", config_value_int("sju") );
}
