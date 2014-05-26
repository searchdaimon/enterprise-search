#include "../common/daemon.h"
#include "../common/define.h"
#include "../common/reposetoryNET.h"




//globals
static int socketha;
static int socketOpen = 0;




int main (int argc, char *argv[]) {

	struct ReposetoryHeaderFormat ReposetoryHeader;
        char htmlbuffer[524288];
        unsigned int radress;


	if (argc < 3) {
                printf("Ingen hostname gitt eller lot gitt\n\n\tBruk:\nboitholdTest HostName lotnr");
                exit(0);
        }
	

	while (rGetNextNET(argv[1],atoi(argv[2]),&ReposetoryHeader,htmlbuffer,NULL,&radress,0,0)) {
		printf("url2 %s\n", ReposetoryHeader.url);
	}

	//close(socket);
        exit(0);
}


