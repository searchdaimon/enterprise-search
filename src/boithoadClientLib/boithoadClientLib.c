#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "../common/define.h"
//#include "../common/daemon.h"
#include "daemon.h"
#include "boithoad.h"

//motar en enkel respons liste. Den begynner med en int som sier hov lang den er
int boithoa_getLdapResponsList(int socketha,char **respons_list[],int *nrofresponses) {

	char ldaprecord[MAX_LDAP_ATTR_LEN];
	int intresponse,i,len;

	if (!recvall(socketha,&intresponse,sizeof(intresponse))) {
		return 0;
	}

	#ifdef DEBUG
	printf("nr %i\n",intresponse);
	#endif

	//void *, ikke +1 da vi setter pekeren til \0, ikke bare en char 
	if (( (*respons_list) = malloc((sizeof(char *) * intresponse) +sizeof(void *))) == NULL ) {
		perror("boithoa_getLdapResponsList: can't malloc respons array");
		return 0;
	}
        (*nrofresponses) = 0;

	for (i=0;i<intresponse;i++) {
		if (!recvall(socketha,ldaprecord,sizeof(ldaprecord))) {
			fprintf(stderr,"can't recvall() ldaprecord\n");
			return 0;
		}

		#ifdef DEBUG
		printf("record \"%s\", len %i\n",ldaprecord,strlen(ldaprecord));
		#endif

		len = strnlen(ldaprecord,MAX_LDAP_ATTR_LEN);
                (*respons_list)[(*nrofresponses)] = malloc(len +1); 

		//ToDO: strscpy
                strncpy((*respons_list)[(*nrofresponses)],ldaprecord,len +1);

		++(*nrofresponses);
	}

	(*respons_list)[(*nrofresponses)] = '\0';

	return 1;
}


int boitho_authenticat(const char username_in[],char password_in[]) {
        int socketha;
        int response;

        char username[64];
        char password[64];

        strncpy(username,username_in,sizeof(username));
        strncpy(password,password_in,sizeof(password));

        if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		perror("localhost");
		return 0;
	}

	#ifdef DEBUG
		printf("boitho_authenticat: dic cconnect\n");
	#endif

        sendpacked(socketha,bad_askToAuthenticate,BADPROTOCOLVERSION, 0, NULL,"");

	sendall(socketha,username, sizeof(username));
	sendall(socketha,password, sizeof(password));

	recvall(socketha,&response,sizeof(response));

	close(socketha);


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
int boithoad_listGroups( char **respons_list[],int *nrofresponses){

	int socketha;

	if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		return 0;
	} 

	sendpacked(socketha,bad_listGroups,BADPROTOCOLVERSION, 0, NULL,"");


	if (!boithoa_getLdapResponsList(socketha,respons_list,nrofresponses)) {
		return 0;
	}

	close(socketha);


	return 1;

}

int boithoad_listUsers(char **respons_list[],int *nrofresponses){

	int socketha;

	if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		#ifdef DEBUG
			printf("cant connect to boithobd at %s:%d\n",__FILE__,__LINE__);
		#endif
		return 0;
	} 

	sendpacked(socketha,bad_listUsers,BADPROTOCOLVERSION, 0, NULL,"");


	if (!boithoa_getLdapResponsList(socketha,respons_list,nrofresponses)) {
		return 0;
	}

	close(socketha);


	return 1;
}

int boithoad_listMailUsers(char **respons_list[],int *nrofresponses){

	int socketha;

	if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		#ifdef DEBUG
			printf("cant connect to boithobd at %s:%d\n",__FILE__,__LINE__);
		#endif
		return 0;
	} 

	sendpacked(socketha,bad_listMailUsers,BADPROTOCOLVERSION, 0, NULL,"");


	if (!boithoa_getLdapResponsList(socketha,respons_list,nrofresponses)) {
		return 0;
	}

	close(socketha);


	return 1;
}



void boithoad_respons_list_free(char *respons_list[]) {
	int i = 0;
        while(respons_list[i] != '\0') {
                free(respons_list[i]);
                i++;
        }
        free(respons_list);
}

int boithoad_groupsForUser(const char username_in[],char **respons_list[],int *nrofresponses) {

	int socketha;
	int intresponse;
	char username[64];
     
	//ToDo: strscpy
	strncpy(username,username_in,sizeof(username));

        if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		printf("boithoad_groupsForUser: can't connect\n");
		return 0;
	}

        sendpacked(socketha,bad_groupsForUser,BADPROTOCOLVERSION, 0, NULL,"");

	sendall(socketha,username, sizeof(username));
	
	if (!boithoa_getLdapResponsList(socketha,respons_list,nrofresponses)) {
		printf("boithoad_groupsForUser: can't getLdapResponsList for user \"%s\"\n",username);
		return 0;
	}

	close(socketha);

	#ifdef DEBUG
		int i_debug;
		printf("debug: have %i groups\n",*nrofresponses);

		for (i_debug=0;i_debug<*nrofresponses;i_debug++) {
			printf("debug: group name \"%s\", nr %i\n",(*respons_list)[i_debug],i_debug);
		}
	#endif

	return 1;
}

int boithoad_getPassword(const char username_in[], char password_in[]) {

	int socketha;
	int intresponse;
	char username[64];
	char password[64];
     	int forreturn = 0;

	//ToDo: strscpy
	strncpy(username,username_in,sizeof(username));

        if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		return 0;
	}

        sendpacked(socketha,bad_getPassword,BADPROTOCOLVERSION, 0, NULL,"");

	sendall(socketha,username, sizeof(username));
	
	//read respons
        if (!recvall(socketha,&intresponse,sizeof(intresponse))) {
                return 0;
        }

	if (intresponse == 1) {
		if (!recvall(socketha,password,sizeof(password))) {
                	return 0;
        	}
		//printf("got \"%s\"\n",password);
		strcpy(password_in,password);
		forreturn = 1;
	}
	else {
		printf("dident have a passord at %s:%d\n",__FILE__,__LINE__);
		strcpy(password_in,"");
		forreturn = 0;
	}

	close(socketha);

	return forreturn;

}

int boithoad_sidToUser(char sid_in[], char username_in[]) {

	int socketha;
	int intresponse;
	char username[64];
	char sid[512];
     	int forreturn = 0;

	//ToDo: strscpy
	strncpy(sid,sid_in,sizeof(sid));

        if ((socketha = cconnect("localhost", BADPORT)) == 0) {
		return 0;
	}

        sendpacked(socketha,bad_sidToUser,BADPROTOCOLVERSION, 0, NULL,"");

	sendall(socketha, sid, sizeof(sid));
	
	//read respons
        if (!recvall(socketha,&intresponse,sizeof(intresponse))) {
                return 0;
        }

	if (intresponse == 1) {
		if (!recvall(socketha,username,sizeof(username))) {
                	return 0;
        	}
		strcpy(username_in, username);
		forreturn = 1;
	}
	else {
		printf("didn't have a user at %s:%d\n",__FILE__,__LINE__);
		strcpy(username_in,"");
		forreturn = 0;
	}

	close(socketha);

	return forreturn;

}
