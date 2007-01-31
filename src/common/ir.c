#include <stdlib.h>
#include <string.h>

//sjekker om et ord er stoppord
int isStoppWord(char *term) {
	if (strlen(term) < 2) {
		return 1;
	}
	else {
		return 0;
	}
}

int find_domain (char url[],char domain[]) {

        char *cpnt;


        //kutter av http:// (de 7 første tegnene)
        strcpy(domain,url + 7);

        //søker oss til / og kapper
        if ((cpnt = strchr(domain,'/')) == NULL) {
                //printf("bad url\n");
                return 0;
        }
        else {
                domain[cpnt - domain] = '\0';
                return 1;
        }
}

