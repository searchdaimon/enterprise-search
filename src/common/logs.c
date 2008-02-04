
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "boithohome.h"

//FILE *LOGACCESS, *LOGERROR;

void bvlog(FILE *LOG, int level,const char *fmt,va_list ap) {

        vfprintf(LOG,fmt,ap);
	fprintf(LOG,"\n");
}


void blog(FILE *LOG, int level, const char *fmt, ...) {

        va_list     ap;

        va_start(ap, fmt);

		//skriver til log
                vfprintf(LOG, fmt,ap);

		//viser på skjerm
                printf("log: ");
                vprintf(fmt,ap);
                printf("\n");

        va_end(ap);
}



int openlogs(FILE **LOGACCESS, FILE **LOGERROR, char name[]) {

	char file[PATH_MAX];

	sprintf(file,"logs/%s_access",name);
	//opener logger
	if (((*LOGACCESS) = bfopen(file,"a")) == NULL) {
		fprintf(stderr,"openlogs: can't open logfile.\n");
		perror(bfile(file));
		return 0;
	}
	printf("opened log \"%s\"\n",file);

	sprintf(file,"logs/%s_error",name);
	if (((*LOGERROR) = bfopen(file,"a")) == NULL) {
		perror("error log");
		return 0;
	}
	printf("opened log \"%s\"\n",file);

	return 1;
}

void closelogs(FILE *LOGACCESS, FILE *LOGERROR) {
	fclose(LOGACCESS);
	fclose(LOGERROR);
}
