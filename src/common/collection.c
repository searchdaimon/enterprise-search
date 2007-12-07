#include "debug.h"

void collection_normalize_name(char collection_name[], int length) {

	int y;

                //fjerner ikke aski tegn, som / og space. De fører til problemer
                for(y=0;y<length;y++) {
                        if (isalnum(collection_name[y])) {
                                //er arcii
                        }
			else if (collection_name[y] == 32) {
				//space
			}
			else if (collection_name[y] == 95) {
				// _
			}
			else if (collection_name[y] == 45) {
				// -
			}
                        else {
                                debug("removed noen ascii char \"%c\" from collection_name \n",collection_name[y]);
                                collection_name[y] = 'X';
                        }
                }

}
