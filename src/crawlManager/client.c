#include <errno.h>
#include <netinet/tcp.h>

#include <sys/time.h>


#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/bstr.h"
#include "../common/timediff.h"
#include "../boithoadClientLib/boithoad.h"


int cmc_conect(int *socketha, char statusbuff[],int statusbufflen, int port) {

	//extern int errno;


        if (((*socketha) = cconnect("127.0.0.1", port)) == 0) {
		snprintf(statusbuff,statusbufflen,"Can't connect to crawler manager: %s. Is the crawler manager running?",strerror(errno));
		return 0;
	}
	else {
		int nodelayflag = 1;
		setsockopt(*socketha, IPPROTO_TCP, TCP_NODELAY, &nodelayflag, sizeof(int));
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

int cmc_recrawl(int socketha,char collection_inn[], int docsRemaining, char *extra_in) {
	char collection[64];
	char extrabuf[512];
	int intrespons;
	int i;

	//toDo bruk strSspy
	strncpy(collection,collection_inn,sizeof(collection));
	strncpy(extrabuf, extra_in, sizeof extrabuf);

	sendpacked(socketha,cm_recrawlcollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }
	if (sendall(socketha,&docsRemaining, sizeof docsRemaining) == 0) { perror("sendall"); exit(1); }
	if (sendall(socketha, &extrabuf, sizeof extrabuf) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }

	return 1;
}

int cmc_crawl(int socketha,char collection_inn[], char *extra_in) {
	char collection[64];
	char extrabuf[512];
	int intrespons;
	int i;

	//toDo bruk strSspy
	strncpy(collection,collection_inn,sizeof(collection));
	strncpy(extrabuf, extra_in, sizeof(extrabuf));

	sendpacked(socketha,cm_crawlcollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }
	if(sendall(socketha,&extrabuf, sizeof(extrabuf)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }

	return 1;
}

int
cmc_collectionislocked(int socketha, char *collection_in)
{
	char collection[64];
	int r, i;

	strncpy(collection, collection_in, sizeof(collection));
	sendpacked(socketha, cm_collectionislocked, BLDPROTOCOLVERSION, 0, NULL, "");
	sendall(socketha, &collection, sizeof(collection));

	if ((i=recv(socketha, &r, sizeof(r),MSG_WAITALL)) == -1) {
		return 1;
	}

	return r;
}

int cmc_deleteCollection(int socketha,char collection_in[], char **errormsgp) {
	char collection[64];
	int intrespons;
	int i, len;
	static char errormsg[512];

	//toDo bruk strSspy
	strncpy(collection,collection_in,sizeof(collection));

	sendpacked(socketha,cm_deleteCollection,BLDPROTOCOLVERSION, 0, NULL,"");

	if(sendall(socketha,&collection, sizeof(collection)) == 0) { perror("sendall"); exit(1); }

	if ((i=recv(socketha, &intrespons, sizeof(intrespons),MSG_WAITALL)) == -1) {
            	perror("Cant recv respons");
            	return 0;
        }
	if (intrespons == 0) {

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

	#ifdef DEBUG
	printf("cmc_pathaccess: intrespons %i\n",intrespons);
	#endif

	return intrespons;
}

int
cmc_rewrite_url(int socketha, char *collection_in, const char *url_in, size_t urlinlen, enum platform_type ptype, 
		enum browser_type btype, char *url_out, size_t url_out_len, char *uri_out, size_t uri_out_len, 
		char *fulluri_out, size_t fulluri_out_len)
{
	char url[1024], uri[1024], fulluri[1024];
	struct rewriteFormat rewrite;

	//#ifdef DEBUG
	printf("cmc_rewrite_url(collection_in=\"%s\", url_in=\"%s\")\n",collection_in,url_in);
	//#endif

	memset(&rewrite, '\0', sizeof(rewrite));

	/*
	if (gettimeofday(&start_time, NULL) != 0)
		printf("# ################################## # Error...\n");
	*/
				
	strscpy(rewrite.collection, collection_in, sizeof(rewrite.collection));
	strscpy(rewrite.url, url_in, sizeof(rewrite.url));
	rewrite.ptype = ptype;
	rewrite.btype = btype;

	sendpacked(socketha, cm_rewriteurl, BLDPROTOCOLVERSION, sizeof(rewrite), &rewrite, "");
	//sendall(socketha, &rewrite, sizeof(rewrite));

	if (recvall(socketha, url, sizeof(url)) == 0) {
		perror("recvall(url)");
		return 0;
	}
	if (recvall(socketha, uri, sizeof(uri)) == 0) {
		perror("recvall(uri)");
		return 0;
	}
	if (recvall(socketha, fulluri, sizeof(fulluri)) == 0) {
		perror("recvall(fulluri)");
		return 0;
	}
	strscpy(url_out, url, url_out_len);
	strscpy(uri_out, uri, uri_out_len);
	strscpy(fulluri_out, fulluri, fulluri_out_len);
	printf("~cmc_rewrite_url [uri_out=\"%s\", fulluri=\"%s\"]\n",uri_out,fulluri_out);
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

int cmc_killcrawl(int socketha, int port) {
    int resp;
    sendpacked(socketha, cm_killcrawl, BLDPROTOCOLVERSION, 0, NULL, "");
    if (sendall(socketha, &port, sizeof port) == 0) {
        perror("killcrawl sendall");
        exit(1);
    }

    if (recv(socketha, &resp, sizeof resp, MSG_WAITALL) == -1) {
        perror("killcrawl recv");
        return 0;
    }
    return resp;
}

int cmc_groupsforuserfromusersystem(int socketha, char *_user, unsigned int us, char ***_groups, char *extra_in) {
	char user[512];
	int i, n;
	char **groups;
	char extrabuf[512];

	strncpy(user, _user, sizeof(user));
	strncpy(extrabuf, extra_in, sizeof extrabuf);
	sendpacked(socketha, cm_groupsforuserfromusersystem, BLDPROTOCOLVERSION, sizeof(user), user, "");

	sendall(socketha, &us, sizeof(us));
	sendall(socketha, &extrabuf, sizeof extrabuf);

	if (recv(socketha, &n, sizeof n, MSG_WAITALL) == -1) {
		perror("groupsbyuserforcoll recv");
		return 0;
	}

	#ifdef DEBUG_TIME
		gettimeofday(&ts, NULL);
	#endif

	if (n > 0) {
		groups = calloc(n, MAX_LDAP_ATTR_LEN);
		if (recv(socketha, groups, n * MAX_LDAP_ATTR_LEN, MSG_WAITALL) == -1) {
			perror("groupsbyuserforcoll recv");
			return 0;
		}	
		*_groups = groups;
	} else {
		*_groups = NULL;
	}

	#ifdef DEBUG_TIME
		gettimeofday(&te, NULL);
		printf("grepme cmc_groupsforuserfromusersystem took: %f\n", getTimeDifference(&ts, &te));
	#endif

	return n;
}

int cmc_authuser(int sock, char *_user, char *_pass, unsigned int usersystem, char *extra_in)
{
	char user[512], pass[512];
	int n;
	char extrabuf[512];

	strncpy(user, _user, sizeof(user));
	strncpy(pass, _pass, sizeof(pass));
	strncpy(extrabuf, extra_in, sizeof extrabuf);

	sendpacked(sock, cm_authenticateuser, BLDPROTOCOLVERSION, 0, NULL, "");
	sendall(sock, &user, sizeof(user));
	sendall(sock, &pass, sizeof(pass));
	sendall(sock, &usersystem, sizeof(usersystem));
	sendall(sock, &extrabuf, sizeof extrabuf);

	recv(sock, &n, sizeof(int), 0);

	return n;
}

int
cmc_collectionsforuser(int sock, char *_user, char **_collections)
{
	char user[512];
	int n;
	char *collections;

	strncpy(user, _user, sizeof(user));
	sendpacked(sock, cm_collectionsforuser, BLDPROTOCOLVERSION, sizeof(user), user, "");

	recv(sock, &n, sizeof(n), MSG_WAITALL);
	if (n == 0) {
		collections = strdup("");
	} else {
		collections = calloc(n, maxSubnameLength+1);
		recv(sock, collections, (maxSubnameLength+1)*n, MSG_WAITALL);
	}
	*_collections = collections;
	
	return n;
}

int
cmc_usersystemfromcollection(int sock, char *collection)
{
	int n;

	sendpacked(sock, cm_usersystemfromcollection, BLDPROTOCOLVERSION, 0, NULL, collection);
	recv(sock, &n, sizeof(n), MSG_WAITALL);

	return n;
}

struct cm_listusers_h
cmc_listusersus(int sock, int usersystem, char ***users, char *extra_in)
{
	struct cm_listusers_h users_h;
	int i, n_users;
	char user[MAX_LDAP_ATTR_LEN];
	char extrabuf[512];

	strncpy(extrabuf, extra_in, sizeof extrabuf);

	sendpacked(sock, cm_listusersus, BLDPROTOCOLVERSION, 0, NULL, "");
	sendall(sock, &usersystem, sizeof(usersystem));
	sendall(sock, &extrabuf, sizeof extrabuf);

	recv(sock, &users_h, sizeof(users_h), 0);
	n_users = users_h.num_users;
	if (n_users >= 0) {
		*users = malloc((n_users+1) * sizeof(char *));
		// runarb 2 sept 2009: må finne en måte å sjekke om malloc fungerte her. 
        	//if (*users == NULL) {
                //	perror("malloc");
                //	return 0;
	        //}
		for (i = 0; i < n_users; i++) {
			recv(sock, user, sizeof(user), 0);
			(*users)[i] = strdup(user);
		}
		(*users)[n_users] = NULL;
	}
	return users_h;
}

int
cmc_addForeignUsers(int sock, char *collection, char *inuser, char *ingroup)
{
	int n;
	char group[512], user[512];

	strlcpy(group, ingroup, sizeof(group));
	strlcpy(user, inuser, sizeof(user));

	sendpacked(sock, cm_removeForeignUsers, BLDPROTOCOLVERSION, 0, NULL, collection);
	sendall(sock, user, sizeof(user));
	sendall(sock, group, sizeof(group));
	recv(sock, &n, sizeof(n), MSG_WAITALL);

	return n;
}

int
cmc_removeForeignUsers(int sock, char *collection)
{
	int n;

	sendpacked(sock, cm_removeForeignUsers, BLDPROTOCOLVERSION, 0, NULL, collection);
	recv(sock, &n, sizeof(n), MSG_WAITALL);

	return n;
}

void cmc_close(int socketha) {
	close(socketha);
}
