#include "debug.h"

void collection_normalize_name(char collection_name[], int length) {

	int y;

                //fjerner ikke aski tegn, som / og space. De fører til problemer
                for(y=0;y<length;y++) {
                        if ((collection_name[y] > 48) && (collection_name[y] < 122)) {
                                //er arcii
                        }
                        else {
                                debug("removed noen ascii char \"%c\" from collection_name \n",collection_name[y]);
                                collection_name[y] = 'X';
                        }
                }

}
