#include <stdlib.h>
#include <string.h>

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/ir.h"
#include "../common/revindex.h"
#include "../common/url.h"
#include "../common/bstr.h"

#define subname "www"


int main (int argc, char *argv[]) {

	struct DocumentIndexFormat DocumentIndexPost;
	int lotNr;
	int i;
	unsigned int DocID;
	char domain[64];

        if (argc < 2) {
                printf("Dette programet leser en DocumentIndex. Gi det et lot nr. \n\n\tUsage: ./readDocumentIndex 1");
               exit(0);
        }

	lotNr = atoi(argv[1]);
	DocID = 0;

	revindexFilesOpenLocal(revindexFilesHa,lotNr,"Url","wb",subname);


	while (DIGetNext (&DocumentIndexPost,lotNr,&DocID,subname)) {


		if (DocumentIndexPost.Url[0] == '\0') {

		}
		else if (strncmp(DocumentIndexPost.Url,"http://",7) != 0) {
			//printf("no http: %s\n",DocumentIndexPost.Url);
		}
		else if (!find_domain(DocumentIndexPost.Url,domain,sizeof(domain))) {
			//printf("!find_domain %s\n",DocumentIndexPost.Url);
		}
		else {
			//printf("DocID: %u, url: %s\n",DocID,DocumentIndexPost.Url);

			//#ifdef DEBUG
			printf("url: \"%s\", DocID %u\n",DocumentIndexPost.Url,DocID);
			printf("dd: %s\n",domain);
			//#endif


			}

			
		}
	}


	//DIClose();


}

