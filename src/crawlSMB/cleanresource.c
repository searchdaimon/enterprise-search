#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cleanresourceWinToUnix(char resource[]) {

        char *cp;

        //gjør om på formatet slik at \\ blir //
        if (strchr(resource,'\\') != NULL) {
                crawlWarn("collection \"%s\" contains \"\\\" characters. Corect format \
                        is //host/shares not \\\\host\\shares. Will convert \"\\\" to \"/\"",resource);

                while((cp = strchr(resource,'\\')) != NULL) {
                        (*cp) = '/';
                }

        }

}

int cleanresourceUnixToWin(char resource[]) {

        char *cp;

        //gjør om på formatet slik at 
        if (strchr(resource,'/') != NULL) {

                while((cp = strchr(resource,'/')) != NULL) {
                        (*cp) = '\\';
                }

        }

	printf("new %s\n",resource);
}

