#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h> /* for mode definitions */
#include <ctype.h>

#include "../common/define.h"
#include "../common/lot.h"

int sisdigit (char s[]) {
	int i;
	for (i=0;i<strlen(s);i++) {
		if (!isdigit(s[i])) {
			return 0;
		}
	}
	return 1;

}

int main (int argc, char *argv[]) {

        FILE *MAPLIST;
        char *line;
        char buff[1024];
        int count, start;
	int last;
	DIR *d;
	struct dirent *de;
	char dirs[NrOfDataDirectorys][255];

	MAPLIST = openMaplist();


        //lager mappe oversikt
        count = 0;

        while ((feof(MAPLIST) == 0) && (NrOfDataDirectorys > count)) {
                line = fgets(buff,sizeof(buff),MAPLIST);
                //line = gets(MAPLIST);
                chomp(line);

		strcpy(dirs[count],line);
		


                count++;
        }

	count = 0;
	start = 20;
	last = 0;
	while ((NrOfDataDirectorys > count) && (!last)) {
		printf("ll: %s\n",dirs[start]);

                if ((d = opendir(dirs[start])) != NULL) {
                        while (de = readdir(d)) {
                                if (sisdigit(de->d_name)) {
                                        //puts(de->d_name);
                                        printf("yy: -%s-\n",de->d_name);
                                }
                                //else {
                                //      printf("no: -%s-\n",de->d_name);
                                //}
                        }
                        closedir(d);
                }

		if (start == NrOfDataDirectorys) {
			start = 0;
		}

		++start;
		++count;
	}

}

