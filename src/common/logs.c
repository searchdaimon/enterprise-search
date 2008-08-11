
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>

#include "boithohome.h"

//FILE *LOGACCESS, *LOGERROR;

void bvlog(FILE *LOG, int level,const char *fmt,va_list ap) {

        vfprintf(LOG,fmt,ap);
	fprintf(LOG,"\n");
}


void blog(FILE *LOG, int level, const char *fmt, ...) {
	time_t now;
	char buf[1024];
	int len;
        va_list     ap;
	struct tm tm;

        va_start(ap, fmt);

		//skriver til log
		if (LOG == NULL) {
                	printf("log (log filehandler is NULL, won't log to file): ");
                	vprintf(fmt,ap);
                	printf("\n");

		}
		else {

			/* Output time */
			now = time(NULL);
			localtime_r(&now, &tm);
			len = strftime(buf, sizeof(buf), "%F %T", &tm);
			buf[len-1] = '\0';
			printf("%s: ", buf);
			fprintf(LOG, "%s: ", buf);

			//skriver beskjeden til log fil
	                vfprintf(LOG, fmt,ap);
			fprintf(LOG, "\n");

		}

		//viser på skjerm
		if (level <= 1) {
               		printf("log: ");
               		vprintf(fmt,ap);
          		printf("\n");
		}
        va_end(ap);
}



int openlogs(FILE **LOGACCESS, FILE **LOGERROR, char name[]) {

	char file[PATH_MAX];

	sprintf(file,"logs/%s_access",name);
	//opener logger
	if (((*LOGACCESS) = bfopen(file,"a")) == NULL) {
		fprintf(stderr,"openlogs: can't open access logfile.\n");
		perror(bfile(file));

		(*LOGACCESS) = NULL;
		//return 0;
	}
	else {
		printf("opened log \"%s\"\n",file);
	}

	sprintf(file,"logs/%s_error",name);
	if (((*LOGERROR) = bfopen(file,"a")) == NULL) {
		fprintf(stderr,"openlogs: can't open error logfile.\n");
		perror(bfile(file));

		(*LOGERROR) = NULL;
		//return 0;
	}
	else {
		printf("opened log \"%s\"\n",file);
	}
	return 1;
}

void closelogs(FILE *LOGACCESS, FILE *LOGERROR) {
	if (LOGACCESS != NULL) {
		fclose(LOGACCESS);
	}

	if (LOGERROR != NULL) {
		fclose(LOGERROR);
	}
}
