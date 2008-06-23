#include <stdio.h>

#include "common.h"


int
main(void)
{
	RepositoryHeader_t h;
	char *p, *p2;
#define PRINT_OFFSET(name) p = (char *)&h.name; printf("%s offset is: %d and has size: %d\n", #name, p - p2, sizeof(h.name));

	p2 = (char *)&h;
	PRINT_OFFSET(DocID);
	PRINT_OFFSET(url);
	PRINT_OFFSET(content_type);
	PRINT_OFFSET(IPAddress);
	PRINT_OFFSET(response);
	PRINT_OFFSET(htmlSize);
	PRINT_OFFSET(imageSize);
	PRINT_OFFSET(time);
	PRINT_OFFSET(userID);
	PRINT_OFFSET(clientVersion);

	return 0;
}
