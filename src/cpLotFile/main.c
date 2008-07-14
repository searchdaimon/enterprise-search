
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <sys/file.h>


#include "../common/define.h"
#include "../common/reposetoryNET.h"

int main (int argc, char *argv[]) {


        if (argc < 5) {
                printf("Dette programet kopierer lotfiler over nettet\n\n\tcpLotFile SOURCE DEST lotnr host\n\n");
                exit(0);
        }

	rSendFile(argv[1], argv[2], atoi(argv[3]), argv[4]);


}

