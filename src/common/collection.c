#include <ctype.h>

#include "debug.h"

void collection_normalize_name(char collection_name[], int length) {

	int y;

                // remove non-ascii characters, such as / and space. They cause problems
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
			else if (y != 0 && collection_name[y] == '.') {
				// .
			}
            else {
            	debug("removed noen ascii char \"%c\" from collection_name \n",
					collection_name[y]);

                collection_name[y] = 'X';
           }
     }
}
