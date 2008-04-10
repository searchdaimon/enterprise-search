
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../common/url.h"

int main (int argc, char *argv[]) {

char *A[][2] = {
		// from, to 

		//normale url, disse skal altid fungere
		{"http://www.boitho.com/File.html","http://www.boitho.com/File.html"}, 
		{"http://sub.boitho.com/File.html","http://sub.boitho.com/File.html"}, 

		{"http://www.boitho.com","http://www.boitho.com/"},
		{"http://boitho.com/","http://www.boitho.com/"},
		{"http://boitho.com","http://www.boitho.com/"},
		{"http://boitho.com/File.html","http://www.boitho.com/File.html"},
		{"http://www.boitho.com:80/","http://www.boitho.com/"},
		{"http://www.boitho.com:80/File.html","http://www.boitho.com/File.html"},
		{"http://www.boitho.com?a=b","http://www.boitho.com/?a=b"},
		{"HTTP://www.Boitho.coM/File.html","http://www.boitho.com/File.html"},
		{"http://www.boitho.com/side.html#august","http://www.boitho.com/side.html"},

		// ikke gyldige, men normaliseres OK
		{"http://www.boitho.comindex.html?a=b","http://www.boitho.com/index.html?a=b"}

	};
int Asize = sizeof(A) / (sizeof(char *) * 2);

int i;
char url[200];


for(i=0;i<Asize;i++) {

	//printf("%s %s\n",A[i][0],A[i][1]);
	strcpy(url,A[i][0]);

	//normaliser
	if (!url_normalization(url,sizeof(url))) {
		printf("Error: bad status from url_normalization: %s -> %s\n",A[i][0],A[i][1]);
	}
	else if (!gyldig_url(url)) {
		printf("Warn: ikke gyldig: %s -> %s (did become \"%s\")\n",A[i][0],A[i][1],url);	
	}
	else if (strcmp(url,A[i][1]) == 0) {
		printf("OK: %s -> %s\n",A[i][0],A[i][1]);	
	}
	else {
		printf("Error: %s -> %s : %s\n",A[i][0],A[i][1],url);	

	}

}

}
