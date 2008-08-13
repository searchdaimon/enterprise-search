#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include "../common/lot.h"

int main(void) {
	DIR * dirh = listAllColl_start();
	if (dirh == NULL) 
		err(1, "listAllColl_start()");
	
	char * subname;
	while ((subname = listAllColl_next(dirh)) != NULL)
		printf("subname: %s\n", subname);

	listAllColl_close(dirh);
	return 0;
}

