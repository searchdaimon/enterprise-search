#include <errno.h>


#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/bstr.h"


int cmc_conect(int *socketha, char statusbuff[],int statusbufflen, int port) {

	//extern int errno;


        if (((*socketha) = cconnect("127.0.0.1", port)) == 0) {
		snprintf(statusbuff,statusbufflen,"cconnect: %s",strerror(errno));
		return 0;
	}
	else {
		return 1;
	}
}


int cmc_scan(int socketha,char **respons_list[],int *nrofresponses,char **errormsgp,char crawlertype_in[],
		char host_in[],char username_in[],char password_in[]) {
                       char crawlertype[64];
                        char host[64];
                        char username[64];
                        char password[64];
	int i,len;
	static char errormsg[512];

	strscpy(crawlertype,crawlertype_in,sizeof(crawlertype));
	strscpy(host,host_in,sizeof(host));
	strscpy(username,username_in,sizeof(username));
	strscpy(password,password_in,sizeof(password));

	sendpacked(socketha,cm_scan,BLDPROTOCOLVERSION, 0, NULL,"");


	if(sendall(socketha,&crawlertype, sizeof(crawlertype)) == 0) { perror("sendall"); exit(1); }
	if(sendall(socketha,&host, sizeof(host)) == 0) { perror("sendall"); exit(1); }
	if(sendall(socketha,&username, sizeof(username)) == 0) { perror("sendall"); exit(1); }
	if(sendall(socketha,&password, sizeof(password)) == 0) { perror("sendall"); exit(1); }


	socketgetsaa(socketha,respons_list,nrofresponses);	

	if ((*nrofresponses) == 0) {

		if ((i=recv(socketha, &len, sizeof(len),MSG_WAITALL)) == -1) {
        	        perror("Cant recv respons");
        	        return 0;
        	}

        	if ((i=recv(socketha, errormsg, len,MSG_WAITALL)) == -1) {
                	perror("Cant read respons");
                	return 0;
        	}
		
		(*errormsgp) = errormsg;

		return 0;
	}
	
	return 1;
}

int cmc_recrawl(int socketha,char collection_inn[]) {
	char collection[64];
	int intrespons;
	int i;

	//toDo bruk strSspy
	strncpy(collection,collection_inn,sizeof(collection));

	sendpacked(socketha,cm_recrawlcollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }

	return 1;
}

int cmc_crawl(int socketha,char collection_inn[]) {
	char collection[64];
	int intrespons;
	int i;

	//toDo bruk strSspy
	strncpy(collection,collection_inn,sizeof(collection));

	sendpacked(socketha,cm_crawlcollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }

	return 1;
}

int cmc_deleteCollection(int socketha,char collection_in[]) {
	char collection[64];
	int intrespons;
	int i;

	//toDo bruk strSspy
	strncpy(collection,collection_in,sizeof(collection));

	sendpacked(socketha,cm_deleteCollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }

	return 1;

}

int
cmc_pathaccess(int socketha,char collection_in[], char uri_in[], char user_in[], char password_in[])
{
	char collection[64];
	char uri[512];
	char user[64];
	char password[64];
        int intrespons;
        int i;
	char all[64+512+64+64];

	memset(collection,'\0',sizeof(collection));

	//toDo bruk strSspy
	strncpy(all,collection_in,sizeof(collection));
	strncpy(all+64,uri_in,sizeof(uri));

	strncpy(all+512+64,user_in,sizeof(user));
	strncpy(all+512+64+64,password_in,sizeof(password));

	sendpacked(socketha,cm_pathaccess,BLDPROTOCOLVERSION, sizeof(all), all, collection);

#if 0
	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if(sendall(socketha,&uri, sizeof(uri)) == 0) { perror("sendall"); exit(1); }

	if(sendall(socketha,&user, sizeof(user)) == 0) { perror("sendall"); exit(1); }
	if(sendall(socketha,&password, sizeof(password)) == 0) { perror("sendall"); exit(1); }
#else 
	//if(sendall(socketha,&all, sizeof(all)) == 0) { perror("sendall"); exit(1); }
#endif

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
                perror("Cant recv respons");
                return 0;
        }

	printf("cmc_pathaccess: intrespons %i\n",intrespons);
	return intrespons;
}

int
cmc_rewrite_url(int socketha, char *collection_in, char *uri_in, size_t inlen, enum platform_type ptype, enum browser_type btype,
                char *uri_out, size_t len)
{
	char uri[512];
	struct rewriteFormat rewrite;

	//memset(&rewrite, '\0', sizeof(rewrite));

	/*
	if (gettimeofday(&start_time, NULL) != 0)
		printf("# ################################## # Error...\n");
	*/
				
	strscpy(rewrite.collection, collection_in, sizeof(rewrite.collection));
	strscpy(rewrite.uri, uri_in, sizeof(rewrite.uri));
	rewrite.ptype = ptype;
	rewrite.btype = btype;

	sendpacked(socketha, cm_rewriteurl, BLDPROTOCOLVERSION, sizeof(rewrite), &rewrite, "");
	//sendall(socketha, &rewrite, sizeof(rewrite));

	if (recvall(socketha, uri, sizeof(uri)) == 0) {
		perror("recvall(uri)");
		exit(1);
	}
	strscpy(uri_out, uri, len);
	/*
	if (gettimeofday(&end_time, NULL) != 0)
		printf("# ################################## # Error...\n");
	printf("####### ttt time %.50f\n",getTimeDifference(&start_time,&end_time));
	*/


	return 1;
}

int cmc_crawlcanconect (int socketha, char vantcollection[], char statusbuff[],int statusbufflen) {
	int i;
	int intrespons;
	char *respons;
	char collection[64];

	//extern int errno;


	sendpacked(socketha,cm_crawlcanconect,BLDPROTOCOLVERSION, 0, NULL,"");

	//sending coll
	strscpy(collection,vantcollection,sizeof(collection));
	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
	    	snprintf(statusbuff,statusbufflen,"recv: %s",strerror(errno));
            	return 0;
        }

	respons = malloc(intrespons +1);
        if ((i=recv(socketha, respons, intrespons,MSG_WAITALL)) == -1) {
                perror("Cant read respons");
	    	snprintf(statusbuff,statusbufflen,"recv: %s",strerror(errno));
        	return 0;
        }

	if (strcmp(respons,"ok") != 0) {
		strscpy(statusbuff,respons,statusbufflen);
		return 0;
	}

	return 1;

}
void cmc_close(int socketha) {
	close(socketha);
}
