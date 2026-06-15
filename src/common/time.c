#include <time.h>
#include <string.h>

char *ctime_s(const time_t *timep) {

	static char tbuf[26];

	char *cp;

        // Find human-readable time
        ctime_r(timep,tbuf);
        // remove the \n that ctime keeps adding
        cp = strchr(tbuf,'\n');
        if (cp != NULL)
        	cp[0] = '\0';


	return tbuf;

}
