
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
		goto bbdn_conect_err;
	}


        
        if ((i=recv((*socketha), &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	bperror("Cant recv respons");
  		goto bbdn_conect_err;
        }
	
	if (intrespons == bbc_authenticate_ok) {
		debug("bbc authenticate ok\n");
		return 1;
	}
	else if (intrespons == bbc_authenticate_feiled) {
		berror("bbc authenticate feiled");
		goto bbdn_conect_err;
	}
	else {
		berror("respons from server was nider bbc_authenticate_feiled or bbc_authenticate_ok!\n");
		goto bbdn_conect_err;
	}

	bbdn_conect_err:
		close(*socketha);
		return 0;
	
}

int bbdn_close(int socketha) {
	if(!close(socketha)) {
		printf("Error: bbdn_close can't close\n");
		perror("close");
	}

	return 1;
}

int bbdn_docadd(int socketha, const char subname[], const char documenturi[], const char documenttype[], const char document[],
	const int dokument_size, const unsigned int lastmodified, const char *acl_allow, const char *acl_denied, const char title[], 
	const char doctype[], const char *attributes) {

	int len, intrespons, i;
	char *blank = "";

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
	if (documenttype==NULL) {
		documenttype = blank;
	}
        len = strlen(documenttype) +1;
	debug("sending (len %i): \"%s\"",len,documenttype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall documenttype len"); return 0; }
        if(sendall(socketha,documenttype, len) == 0) { perror("sendall documenttype"); return 0; }

        //document
        //dokument_size
	debug("sending document(len %i)",dokument_size);
	debug("sending1 %d\n", dokument_size);
        if(sendall(socketha,&dokument_size, sizeof(int)) == 0) { perror("sendall dokument_size"); return 0; }
	debug("sending2 %d\n", dokument_size);
	if (dokument_size != 0) {
        	if(sendall(socketha,document, dokument_size) == 0) { perror("sendall document"); return 0; }
	}
	debug("sending3 %d\n", dokument_size);
        //lastmodified
        if(sendall(socketha,&lastmodified, sizeof(int)) == 0) { perror("sendall lastmodified"); return 0; }

        //acl_allow
	if (acl_allow==NULL) {
		acl_allow = blank;
	}
        len = strlen(acl_allow) +1;
	debug("sending (len %i): \"%s\"",len,acl_allow);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall acl_allow len"); return 0; }
        if(sendall(socketha,acl_allow, len) == 0) { perror("sendall acl_allow"); return 0; }

        //acl_denied
	if (acl_denied==NULL) {
		acl_denied = blank;
	}
        len = strlen(acl_denied) +1;
	debug("sending (len %i): \"%s\"",len,acl_denied);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall acl_denied len"); return 0; }
        if(sendall(socketha,acl_denied, len) == 0) { perror("sendall acl_denied"); return 0; }

        //title
	if (title==NULL) {
		title = blank;
	}
        len = strlen(title) +1;
	debug("sending (len %i): \"%s\"",len,title);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall title len"); return 0; }
        if(sendall(socketha,title, len) == 0) { perror("sendall title"); return 0; }

        //doctype
	if (doctype==NULL) {
		doctype = blank;
	}
        len = strlen(doctype) +1;
	debug("sending (len %i): \"%s\"",len,doctype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall doctype len"); return 0; }
        if(sendall(socketha,doctype, len) == 0) { perror("sendall doctype"); return 0; }

	// Attributes
	if (attributes==NULL) {
		attributes = blank;
	}
        len = strlen(attributes) +1;
	debug("sending (len %i): \"%s\"",len,attributes);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall attributes len"); return 0; }
        if(sendall(socketha,attributes, len) == 0) { perror("sendall attributes"); return 0; }


        if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                bperror("Cant recv respons");
		intrespons = 0;
        }


	return intrespons;
}

int bbdn_docexist(int socketha,char subname[],char documenturi[],unsigned int lastmodified) {
	return 0;
}

//stenger ned å kjører etterpehandler på en koleksin. Dette er ikke det samme som å stenge ned socketen
int bbdn_closecollection(int socketha, const char subname[]) {

	int len, intrespons, i;
	
	debug("bbdn_closecollection start");
	sendpacked(socketha,bbc_closecollection,BLDPROTOCOLVERSION, 0, NULL,"");

       	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,subname, len) == -1) { perror("sendall"); exit(1); }


        if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                bperror("Cant recv respons");
		intrespons = 0;
        }

	debug("bbdn_closecollection end");
		
	return intrespons;
}

/* Delete an uri from the system */
int bbdn_deleteuri(int socketha, const char subname[], const char *uri) {

	int len, intrespons, i;
	
	debug("bbdn_deleteuri start");
	sendpacked(socketha,bbc_deleteuri,BLDPROTOCOLVERSION, 0, NULL,"");

       	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,subname, len) == -1) { perror("sendall"); exit(1); }

       	len = strlen(uri) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,uri, len) == -1) { perror("sendall"); exit(1); }


        if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                bperror("Cant recv respons");
		intrespons = 0;
        }

	debug("bbdn_deleteuri end");
		
	return intrespons;
}

/* Delete an collection from the system */
int bbdn_deletecollection(int socketha, const char subname[]) {
	int len, i;
	int intrespons = 0;
	
	debug("bbdn_deletecollection start");
	sendpacked(socketha,bbc_deletecollection,BLDPROTOCOLVERSION, 0, NULL,"");

       	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,subname, len) == -1) { perror("sendall"); exit(1); }

        if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            bperror("Cant recv respons");
            return 0;
        }

	debug("bbdn_deletecollection end");

	return intrespons;

}

int bbdn_addwhisper(int sock, char *subname, whisper_t whisper) {
	int len;

	debug("bbdn_addwhisper: start");
	sendpacked(sock, bbc_addwhisper, BLDPROTOCOLVERSION, 0, NULL, "");

	len = strlen(subname);
	if (sendall(sock, &len, sizeof(len)) == -1)
		err(1, "sendall(len)");
	if (sendall(sock, subname, len) == -1)
		err(1, "sendall(subname)");
	if (sendall(sock, &whisper, sizeof(whisper)) == -1)
		err(1, "sendall(whisper)");
	debug("bbdn_addwhisper: end");

	return 1;
}

int bbdn_HasSufficientSpace(int socketha, char subname[]) {

	int intrespons = 0;
	int len, i;

	//sender heder
        sendpacked(socketha,bbc_HasSufficientSpace,BLDPROTOCOLVERSION, 0, NULL,"");

        //subname
        len = strlen(subname) +1;
	debug("sending (len %i): \"%s\"",len,subname);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall subname len"); return 1; }
        if(sendall(socketha,subname, len) == 0) { perror("sendall subname"); return 1; }

        

        
        if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            bperror("Cant recv respons");
            return 1;
        }

	return intrespons;
}

void
bbdn_opencollection(int sock, char *subname)
{
	int len;

	sendpacked(sock, bbc_opencollection, BLDPROTOCOLVERSION, 0, NULL, "");

	len = strlen(subname);
	if (sendall(sock, &len, sizeof(len)) == -1)
		err(1, "sendall(len)");
	if (sendall(sock, subname, len) == -1)
		err(1, "sendall(subname)");
}
