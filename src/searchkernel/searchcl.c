#include <stdio.h>
#include "searchkernel.h"

int main (int argc, char *argv[]) {

	int i;
	struct QueryDataForamt QueryData;


	//send out an HTTP header:
	printf("Content-type: text/xml\n\n");



        //hvis vi har argumeneter er det første et query
        if (getenv("QUERY_STRING") == NULL) {
                if (argc < 2 ) {
                        printf("Error ingen query spesifisert.\n\nEksempel på bruk for å søke på boitho:\n\tsearchkernel boitho\n");
                }
                else {
                        QueryData.query[0] = '\0';
                        for(i=1;i<argc ;i++) {
                                sprintf(QueryData.query,"%s %s",QueryData.query,argv[i]);
                        }
                        //strcpy(QueryData.query,argv[1]);
                        //printf("argc :%i %s %s\n",argc,argv[1],argv[2]);
                        printf("query %s\n",QueryData.query);
                }
        }
        else {
                parseTheEnv(&QueryData);

        }

        if (strlen(QueryData.query) > MaxQueryLen -1) {
                printf("query to long\n");
                exit(1);
        }

 	//gjør om til liten case
        for(i=0;i<strlen(QueryData.query);i++) {
                QueryData.query[i] = tolower(QueryData.query[i]);
        }


	dosearch(QueryData.query, strlen(QueryData.query));

}
