#include <time.h>
#include <string.h>

char *ctime_s(const time_t *timep) {

	static char tbuf[26];

	char *cp;

        //Finner menesklig lesbar tid
        ctime_r(timep,tbuf);
        //tar bort \n som ctime driver å legger på
        cp = strchr(tbuf,'\n');
        if (cp != NULL)
        	cp[0] = '\0';


	return tbuf;

}
