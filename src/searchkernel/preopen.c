/*

	runarb: 15 ja. Dette var en test av om å preåpne re filer ville øke ytelsen. Det ble ingen forskjell.
*/






#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include "../common/lot.h"
#include "../common/re.h"



void preopen() {
	//return;
	int i;
        DIR * dirh = listAllColl_start();
        if (dirh == NULL)
                err(1, "listAllColl_start()");

        char * subname;
        while ((subname = listAllColl_next(dirh)) != NULL) {
                printf("subname: %s\n", subname);
		for(i=1;i<12;i++) {
			reopen_cache(i,4, "filtypes",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE);
			reopen_cache(i,sizeof(int), "dates",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE);
			reopen_cache(i,sizeof(unsigned int), "crc32map",subname,RE_READ_ONLY|RE_POPULATE);
		}	
	}
        listAllColl_close(dirh);

}
