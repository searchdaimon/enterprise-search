#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "getdate.h"

//Runarb: Vi vil ikke ha en main metode i o filen vi skal inkludere andre plasser
int
main(int argc, char **argv)
{
        struct datelib dl;
	int n;
	char buf[512];


	sprintf(buf,"today");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"yesterday");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"last week");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"last months");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"this year");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));

	sprintf(buf,"this week");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));



	sprintf(buf,"last year");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"two years plus");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"today");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));

	sprintf(buf,"this months");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));

	sprintf(buf,"1395059009");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));




/*
	sprintf(buf,"2 years 5 days 1 week");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"1 years");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"2 months");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


	sprintf(buf,"1 months");
	printf("date: %s\n",buf);
        n = sd_getdate(buf, &dl);
	printf("n: %i\n",n);
        printf("%s\n", ctime(&dl.start));
        printf("%s\n", ctime(&dl.end));


*/




        return 0;
}


