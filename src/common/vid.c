#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../common/crc32.h"


void vid_u(char buf[], int bufsize, char asalt[], unsigned int value, time_t etime, char ip[]) {

	#ifdef DEBUG
		printf("bufsize %i, salt %s, value %u, etime %u\n",bufsize,salt,value,etime);
	#endif
	
        unsigned int crc;
        snprintf(buf,bufsize,"%s%u%u%s",salt,value,(unsigned int)etime,ip);

        crc = crc32boitho(buf);

        snprintf(buf,bufsize,"%u-%u",crc,(unsigned int)etime);
}

