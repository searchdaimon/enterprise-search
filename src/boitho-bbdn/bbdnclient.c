
#include "bbdnclient.h"
#include "bbdn.h"
#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/error.h"
#include "../common/debug.h"

int bbdn_conect(int *socketha, char tkey[], int PORT) {

	int intrespons;
	int i;

	if (((*socketha) = cconnect("127.0.0.1", PORT)) == 0) {
		bperror("Can't connect to back-end document manager");
		return 0;
	}

	//sender heder
        sendpacked((*socketha),bbc_askToAuthenticate,BLDPROTOCOLVERSION, 0, NULL,"");

        
        if (!sendall((*socketha),tkey, 32)) {
		bperror("sendall");
		return 0;
	}


        
        if ((i=recv((*socketha), &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            bperror("Cant recv respons");
            return 0;
        }
	
	if (intrespons == bbc_authenticate_ok) {
		debug("bbc authenticate ok\n");
		return 1;
	}
	else if (intrespons == bbc_authenticate_feiled) {
		berror("bbc authenticate feiled");
		return 0;
	}
	else {
		berror("respons from server was nider bbc_authenticate_feiled or bbc_authenticate_ok!\n");
		return 0;
	}
	
}

int bbdn_close(int socketha) {
	if(!close(socketha)) {
		printf("Error: bbdn_close can't close\n");
		perror("close");
	}

	return 1;
}

int bbdn_docadd(int socketha,char subname[],char documenturi[],char documenttype[],char document[],
	int dokument_size,unsigned int lastmodified,char *acl_allow, char *acl_denied, char title[],char doctype[], char *attributes) {

	int len;

	//sender heder
        sendpacked(socketha,bbc_docadd,BLDPROTOCOLVERSION, 0, NULL,"");

        //subname
        len = strlen(subname) +1;
	debug("sending (len %i): \"%s\"",len,subname);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall subname len"); return 0; }
        if(sendall(socketha,subname, len) == 0) { perror("sendall subname"); return 0; }

        //documenturi
        len = strlen(documenturi) +1;
	debug("sending (len %i): \"%s\"",len,documenturi);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall documenturi len"); return 0; }
        if(sendall(socketha,documenturi, len) == 0) { perror("sendall documenturi"); return 0; }

        //documenttype
        len = strlen(documenttype) +1;
	debug("sending (len %i): \"%s\"",len,documenttype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall documenttype len"); return 0; }
        if(sendall(socketha,documenttype, len) == 0) { perror("sendall documenttype"); return 0; }

        //document
        //dokument_size
	debug("sending document(len %i)",dokument_size);
        if(sendall(socketha,&dokument_size, sizeof(int)) == 0) { perror("sendall dokument_size"); return 0; }
	if (dokument_size != 0) {
        	if(sendall(socketha,document, dokument_size) == 0) { perror("sendall document"); return 0; }
	}
        //lastmodified
        if(sendall(socketha,&lastmodified, sizeof(int)) == 0) { perror("sendall lastmodified"); return 0; }

        //acl_allow
        len = strlen(acl_allow) +1;
	debug("sending (len %i): \"%s\"",len,acl_allow);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall acl_allow len"); return 0; }
        if(sendall(socketha,acl_allow, len) == 0) { perror("sendall acl_allow"); return 0; }

        //acl_denied
        len = strlen(acl_denied) +1;
	debug("sending (len %i): \"%s\"",len,acl_denied);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall acl_denied len"); return 0; }
        if(sendall(socketha,acl_denied, len) == 0) { perror("sendall acl_denied"); return 0; }

        //title
        len = strlen(title) +1;
	debug("sending (len %i): \"%s\"",len,title);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall title len"); return 0; }
        if(sendall(socketha,title, len) == 0) { perror("sendall title"); return 0; }

        //doctype
        len = strlen(doctype) +1;
	debug("sending (len %i): \"%s\"",len,doctype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall doctype len"); return 0; }
        if(sendall(socketha,doctype, len) == 0) { perror("sendall doctype"); return 0; }

	// Attributes
        len = strlen(attributes) +1;
	debug("sending (len %i): \"%s\"",len,attributes);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall attributes len"); return 0; }
        if(sendall(socketha,attributes, len) == 0) { perror("sendall attributes"); return 0; }

	return 1;
}

int bbdn_docexist(int socketha,char subname[],char documenturi[],unsigned int lastmodified) {
	return 0;
}

//stenger ned å kjører etterpehandler på en koleksin. Dette er ikke det samme som å stenge ned socketen
int bbdn_closecollection(int socketha, char subname[]) {

	int len;
	
	debug("bbdn_closecollection start");
	sendpacked(socketha,bbc_closecollection,BLDPROTOCOLVERSION, 0, NULL,"");

       	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,subname, len) == -1) { perror("sendall"); exit(1); }

	debug("bbdn_closecollection end");
		
	return 1;
}
