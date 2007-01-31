#include "../common/lotlist.h"

int main () {

        char buff[512];

        lotlistLoad();

        lotlistGetServer(buff,3000);

        printf("server: %s\n",buff);

        lotlistMarkLocals("bbs-001.boitho.com");

}

