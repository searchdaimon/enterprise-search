
#include "bbdnclient.h"
#include "bbdn.h"
#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/error.h"
#include "../common/debug.h"

int bbdn_conect(int *socketha, char tkey[]) {

	int intrespons;
	int i;

	if (((*socketha) = cconnect("127.0.0.1", BLDPORT)) == 0) {
		bperror("Cant cconnect");
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
		printf("bbc authenticate ok");
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
		perror("close");
	}
}

int bbdn_docadd(int socketha,char subname[],char documenturi[],char documenttype[],char document[],
	int dokument_size,unsigned int lastmodified,char *acl, char title[],char doctype[]) {

	int len;


	//sender heder
        sendpacked(socketha,bbc_docadd,BLDPROTOCOLVERSION, 0, NULL,"");

/*
	//subname
	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == 0) {
		perror("sendall");
		exit(1);
	} 
	sendall(socketha,subname, len);

	//documenturi
	len = strlen(documenturi) +1;
        sendall(socketha,&len, sizeof(int));
	sendall(socketha,documenturi, len);

	//documenttype
	len = strlen(documenttype) +1;
        sendall(socketha,&len, sizeof(int));
	sendall(socketha,documenttype, len);

	//document
	//dokument_size
        sendall(socketha,&dokument_size, sizeof(int));
	sendall(socketha,document, dokument_size);

	//lastmodified
	sendall(socketha,&lastmodified, sizeof(int));

	//acl
	len = strlen(acl) +1;
        sendall(socketha,&len, sizeof(int));
	sendall(socketha,acl, len);

	//title
	len = strlen(title) +1;
        sendall(socketha,&len, sizeof(int));
	sendall(socketha,title, len);

	//doctype
	len = strlen(doctype) +1;
        sendall(socketha,&len, sizeof(int));
	sendall(socketha,doctype, len);

*/

        //subname
        len = strlen(subname) +1;
	debug("sending (len %i): \"%s\"",len,subname);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall subname len"); exit(1); }
        if(sendall(socketha,subname, len) == 0) { perror("sendall subname"); exit(1); }

        //documenturi
        len = strlen(documenturi) +1;
	debug("sending (len %i): \"%s\"",len,documenturi);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall documenturi len"); exit(1); }
        if(sendall(socketha,documenturi, len) == 0) { perror("sendall documenturi"); exit(1); }

        //documenttype
        len = strlen(documenttype) +1;
	debug("sending (len %i): \"%s\"",len,documenttype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall documenttype len"); exit(1); }
        if(sendall(socketha,documenttype, len) == 0) { perror("sendall documenttype"); exit(1); }

        //document
        //dokument_size
	debug("sending document(len %i)",dokument_size);
        if(sendall(socketha,&dokument_size, sizeof(int)) == 0) { perror("sendall dokument_size"); exit(1); }
	if (dokument_size != 0) {
        	if(sendall(socketha,document, dokument_size) == 0) { perror("sendall document"); exit(1); }
	}
        //lastmodified
        if(sendall(socketha,&lastmodified, sizeof(int)) == 0) { perror("sendall lastmodified"); exit(1); }

        //acl
        len = strlen(acl) +1;
	debug("sending (len %i): \"%s\"",len,acl);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall acl len"); exit(1); }
        if(sendall(socketha,acl, len) == 0) { perror("sendall acl"); exit(1); }

        //title
        len = strlen(title) +1;
	debug("sending (len %i): \"%s\"",len,title);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall title len"); exit(1); }
        if(sendall(socketha,title, len) == 0) { perror("sendall title"); exit(1); }

        //doctype
        len = strlen(doctype) +1;
	debug("sending (len %i): \"%s\"",len,doctype);
        if(sendall(socketha,&len, sizeof(int)) == 0) { perror("sendall doctype len"); exit(1); }
        if(sendall(socketha,doctype, len) == 0) { perror("sendall doctype"); exit(1); }

}

int bbdn_docexist(int socketha,char subname[],char documenturi[],unsigned int lastmodified) {
	return 0;
}

//stenger ned å kjører etterpehandler på en koleksin. Dette er ikke det samme som å stenge ned socketen
int bbdn_closecollection(int socketha, char subname[]) {

	int len;
	
	printf("bbdn_closecollection start\n");
	sendpacked(socketha,bbc_closecollection,BLDPROTOCOLVERSION, 0, NULL,"");

       	len = strlen(subname) +1;
        if(sendall(socketha,&len, sizeof(int)) == -1) { perror("sendall"); exit(1); }
        if(sendall(socketha,subname, len) == -1) { perror("sendall"); exit(1); }

	printf("bbdn_closecollection end\n");
		
}
