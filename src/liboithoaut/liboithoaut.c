#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/define.h"
#include "../common/daemon.h"



int boitho_authenticat(char username_in[],char password_in[]) {
        int socketha;
        int response;

        char username[64];
        char password[64];

        strncpy(username,username_in,sizeof(username));
        strncpy(password,password_in,sizeof(password));

        socketha = cconnect("localhost", BADPORT);

        sendpacked(socketha,bad_askToAuthenticate,BLDPROTOCOLVERSION, 0, NULL,"");

	sendall(socketha,username, sizeof(username));
	sendall(socketha,password, sizeof(password));

	recvall(socketha,&response,sizeof(response));

	if (response == ad_userauthenticated_OK) {
		return 1;
	}
	else if (response == ad_userauthenticated_ERROR) {
		return 0;
	}
	else {
		fprintf(stderr,"dident get ad_userauthenticated_ERROR or ad_userauthenticated_OK\n");
		return 0;
	}

}
