#include <stdio.h>
#include <stdlib.h>
#include "utf8-filter.h"


int main()
{
    //char	*test = "Dette er en test for æøå og Ã¦Ã¸Ã¥";
    char	test[] = "&aring;r fastsettes som en videref&oslash;ring av dagens leieniv&aring;. Med vennlig hilsen for <b>Forskningsparken</b> AS Svenning Torp <b>FORSKNINGSPARKEN</b> AS Oslo ...";

    char	*ny = utf8_filter(test);
    printf("%s\n", ny);
    free(ny);
}
