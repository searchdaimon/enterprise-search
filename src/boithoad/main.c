#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lber.h>
#include <ldap.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>


#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/config.h"
#include "../common/bstr.h"
#include "userobjekt.h"

#include "../common/list.h"
#include "../3pLibs/keyValueHash/hashtable.h"

#define RETURN_SUCCESS 1
#define RETURN_FAILURE 0

#define MAX_LDAP_ATTR_LEN 512

static struct hashtable  *gloabal_user_h = NULL;


void connectHandler(int socket);


int ldap_connect(LDAP **ld, const char ldap_host[] , int ldap_port,const char base[],const char distinguishedName[], const char password[]) {

   int  result;
   int  auth_method = LDAP_AUTH_SIMPLE;
   int desired_version = LDAP_VERSION3;
   //char root_dn[512]; 
	

   printf("host %s, user %s, base %s\n",ldap_host,distinguishedName,base);

   if (((*ld) = (LDAP *)ldap_init(ldap_host, ldap_port)) == NULL ) {
      perror( "ldap_init failed" );
      return RETURN_FAILURE;
   }

   /* set the LDAP version to be 3 */
   if (ldap_set_option((*ld), LDAP_OPT_PROTOCOL_VERSION, &desired_version) != LDAP_OPT_SUCCESS)
   {
      ldap_perror((*ld), "ldap_set_option");
	return RETURN_FAILURE;
   }

   if (ldap_bind_s((*ld), distinguishedName, password, auth_method) != LDAP_SUCCESS ) {
	fprintf(stderr,"Can't conect. Tryed to bind ldap server at %s:%i\n",ldap_host,ldap_port);
      ldap_perror( (*ld), "ldap_bind" );
    	return RETURN_FAILURE;
   }

	//alt er vel. Ingen ting her feilet
	return RETURN_SUCCESS;

}

int ldap_close(LDAP **ld) {

   int  result;

   printf("ldap_close: start\n");

   result = ldap_unbind_s((*ld));

   if (result != 0) {
      fprintf(stderr, "ldap_unbind_s: %s\n", ldap_err2string(result));
      return RETURN_FAILURE;
   }

   printf("ldap_close: end\n");
}

//	Finner base name
int ldap_genBaseName(char ldap_base[],const char ldap_domain[]) {

  char **Data;
  int Count, TokCount;
  char buff[128];

  TokCount = split(ldap_domain, ".", &Data);
  printf("\tfound %d token(s):\n", TokCount);

  Count = 0;
  while( (Data[Count] != NULL) ) {
    printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
	sprintf(buff,"dc=%s,",Data[Count]);
	strcat(ldap_base,buff);
	++Count;
  }
  //fjerner , på slutten
  ldap_base[strlen(ldap_base) -1] = '\0';
  printf("\n");
  printf("base %s\n",ldap_base);
  FreeSplitList(Data);


}

void ldap_simple_free(char *respons[]) {
	int i = 0;
        while(respons[i] != '\0') {
		#ifdef DEBUG
		printf("freeing element %i\n",i);
		#endif
		free(respons[i]);
		i++;
	}
	free(respons);
}

struct tempresultsFormat {
	char key[MAX_LDAP_ATTR_LEN];
	char value[MAX_LDAP_ATTR_LEN];
};

int ldap_simple_search(LDAP **ld,char filter[],char vantattrs[],char **respons[],int *nrofresponses,const char ldap_base[]) {

	char *test = malloc(4);

	printf("ldap_simple_search: start\n");


	printf("ldap_base \"%s\", filter \"%s\", vantattrs \"%s\"\n",ldap_base,filter,vantattrs);

   	int  result;
	int i,len,count;
	List list;

/*
	//ldap kan returnere mere en en atributt. Skal ha inn en nulterminert liste med stringer. Siden dette er 
	//en enkel søk stytter vi bare å søke på en verdi  
	char **attrs;
	attrs = malloc(2 * sizeof(char *));
	attrs[0] = malloc(strlen(vantattrs) +1);
	strcpy(attrs[0],vantattrs);
	attrs[1] = '\0';


	printf("vantattrs search\n");
	i = 0;
	while(attrs[i] != '\0') {
		printf("search for \"%s\"\n",attrs[i]);
		++i;
	}
	printf("vantattrs search end\n");
*/
  	char **attrs;
  	int TokCount;

  	printf("Splitting: \"%s\" on \"%s\"\n", vantattrs, ",");

  	TokCount = split(vantattrs, ",", &attrs);
  	printf("\tfound %d token(s):\n", TokCount);

  	count = 0;
  	while( (attrs[count] != NULL) ) {
    		printf("\t\t%d\t\"%s\"\n", count, attrs[count++]);
	}
	printf("\n");


   	BerElement* ber;
   	LDAPMessage* msg;
   	LDAPMessage* entry;

	struct tempresultsFormat *tempresults;

   	char *errstring;
   	char *dn = NULL;
   	char *attr;
   	char **vals;
   	int msgid;
   	//struct timeval tm;


   	int nrOfSearcResults;

   

   	/* ldap_search() returns -1 if there is an error, otherwise the msgid */
   	if ((msgid = ldap_search((*ld), ldap_base, LDAP_SCOPE_SUBTREE, filter, attrs, 0)) == -1) {
   	   ldap_perror( ld, "ldap_search" );
   	   return RETURN_FAILURE;
   	}

   	/* block forever */
   	result = ldap_result((*ld), msgid, 1, NULL, &msg);

   switch(result)
   {
      case(-1):
	 ldap_perror((*ld), "ldap_result");
	 return RETURN_FAILURE;
	 break;
      case(0):
	 printf("Timeout exceeded in ldap_result()");
	 return RETURN_FAILURE;
	 break;
      case(LDAP_RES_SEARCH_RESULT):
	 printf("Search result returned\n");
	 break;
      default:
	 printf("Unknown result : %x\n", result);
	 return RETURN_FAILURE;
	 break;
   }

   nrOfSearcResults = (int)ldap_count_entries((*ld), msg);


   printf("The number of entries returned was %i\n\n", nrOfSearcResults);


	list_init(&list,free);
	
	count =0;	
   /* Iterate through the returned entries */
   for(entry = ldap_first_entry((*ld), msg); entry != NULL; entry = ldap_next_entry((*ld), entry)) {


      if((dn = ldap_get_dn((*ld), entry)) != NULL) {
	 printf("Returned dn: %s\n", dn);
	 ldap_memfree(dn);
      }

      for( attr = ldap_first_attribute((*ld), entry, &ber); attr != NULL; attr = ldap_next_attribute((*ld), entry, ber)) {


	printf("attr adress %u\n",(unsigned int)attr);

	 	if ((vals = (char **)ldap_get_values((*ld), entry, attr)) != NULL)  {


	    		for(i = 0; vals[i] != NULL; i++) {

    	        		printf("attr: %s, vals %s\n", attr, vals[i]);

				tempresults = malloc(sizeof(struct tempresultsFormat));
				printf("tempresults adress %u\n",(unsigned int)tempresults);

				strncpy((*tempresults).value,vals[i],MAX_LDAP_ATTR_LEN);

				printf("tempresults adress %u\n",(unsigned int)tempresults);
			
				if (list_ins_next(&list,NULL,tempresults) != 0) {
					printf("cant insert into list\n");
					return 0;
				}	

				++count;	
			} //for

	

	    		ldap_value_free(vals);
	 	} //if
	printf("attr adress %u\n",(unsigned int)attr);
	ldap_memfree(attr);
	
      } //for


      if (ber != NULL) {
	 ber_free(ber,0);
      }

      printf("\n");
   }

	//clean up
	ldap_msgfree(msg);

  	FreeSplitList(attrs);


	printf("list size if %i, count %i, size %i\n",list_size(&list),count,( sizeof(char *) * (count +1) ));

	(*respons) = malloc(( sizeof(char *) * (count +1) ));

	(*nrofresponses) = 0;
	tempresults = NULL;
	while(list_rem_next(&list,NULL,(void **)&tempresults) == 0) {
		printf("tempresults adress %u\n",(unsigned int)tempresults);
		printf("aaa \"%s\"\n",(*tempresults).value);

		len = strnlen((*tempresults).value,MAX_LDAP_ATTR_LEN);

		(*respons)[(*nrofresponses)] = malloc(len +1);

		strscpy((*respons)[(*nrofresponses)],(*tempresults).value,len +1);

		printf("into \"%s\"\n",(*respons)[(*nrofresponses)]);

		free(tempresults);
		++(*nrofresponses);
	}
	(*respons)[(*nrofresponses)] = '\0';

	list_destroy(&list);

/*
(*respons) = malloc((sizeof(char *) * nrOfSearcResults) +1);
(*nrofresponses) = 0;
for {
				(*respons)[(*nrofresponses)] = malloc(len +1);
				strscpy((*respons)[(*nrofresponses)],vals[i],len +1);

}

	(*respons)[(*nrofresponses)] = '\0';

*/


	printf("nr of results for return is %i\n",(*nrofresponses));

	printf("ldap_simple_search: end\n");

}

int ldap_getcnForUser(LDAP **ld,char cn[],char user_username[],const char ldap_base[]) {

	char **respons;
	int nrOfSearcResults;

	char filter[128];

	sprintf(filter,"(sAMAccountName=%s)",user_username);

	if (!ldap_simple_search(ld,filter,"distinguishedName",&respons,&nrOfSearcResults,ldap_base)) {
		printf("can't ldap search\n");
		return RETURN_FAILURE;	
	}

	printf("ldap_getcnForUser nrOfSearcResults %i\n",nrOfSearcResults);
   	if (nrOfSearcResults == 1) {
        	//ok
		strcpy(cn,respons[0]);
   	}
   	else if (nrOfSearcResults == 0) {
		printf("dident find user in ldap dir\n");
		return RETURN_FAILURE;
	}
	else if (nrOfSearcResults > 1) {
		printf("More then on results. Bad ldap?\n");
	        return RETURN_FAILURE;
	}
	else {
		printf("ldap search error\n");
	        return RETURN_FAILURE;
	}

	ldap_simple_free(respons);

	return RETURN_SUCCESS;	
}

/*
we uses the same tactic as mod_auth, se http://httpd.apache.org/docs/2.0/mod/mod_auth_ldap.html

From apache.org:

During the authentication phase, mod_auth_ldap searches for an entry in the directory that matches 
the username that the HTTP client passes. If a single unique match is found, then mod_auth_ldap 
attempts to bind to the directory server using the DN of the entry plus the password provided 
by the HTTP client. Because it does a search, then a bind, it is often referred to as the 
search/bind phase. Here are the steps taken during the search/bind phase.
*/

int ldap_authenticat (LDAP **ld,char user_username[],char user_password[],const char ldap_base[],const char ldap_host[],int ldap_port) {


   	LDAP *ldforuserconect;

	char cn[MAX_LDAP_ATTR_LEN];

	   //first we search for the user.
	if (!ldap_getcnForUser(ld,cn,user_username,ldap_base)) {
		printf("canr look up cn\n");
		return RETURN_FAILURE;
	}
   	//vi har en fungerende cn. Prøver og koble til
	printf("userid \"%s\", user_password \"%s\"\n",cn,user_password);

   	if (!ldap_connect(&ldforuserconect,ldap_host,ldap_port,ldap_base,cn,user_password)) {
        	printf("cant connect to ldap server by userid \"%s\"\n",cn);
        	return RETURN_FAILURE;
   	}
   	printf("we cod connect. User is Authenticationated\n");

   	ldap_close(&ldforuserconect);


   	return RETURN_SUCCESS;
}
/**********************************************************************
  Henter ut grippe fra en ldap distinguishedName "objectClass: group"
  Er på formatet "CN=Domain Admins,CN=Users,DC=bbtest,DC=searchdaimon,DC=com"
  der Domain Admins er verdien vi vil ha
***********************************************************************/
int getGroupFromDnGroup (char cn[],char groupname[], int sizeofgroupname) {
	char **Data;
        int TokCount,Count;
	char *gropurecord;

	printf("getGroupFromDnGroup: start\ncn: \"%s\"\n",cn);

	TokCount = split(cn, ",", &Data);

	if (TokCount < 2) {
                printf("dident mangae tp split cn. Bad cn? cn was \"%s\"\n",cn);
                return 0;
        }

	Count = 0;
        while( (Data[Count] != NULL) ) {
                printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
	}

        gropurecord = malloc(strlen(Data[0]) +1);
        strcpy(gropurecord,Data[0]);


        FreeSplitList(Data);

	printf("gropurecord \"%s\"\n",gropurecord);

        TokCount = split(gropurecord, "=", &Data);

	Count = 0;
	while( (Data[Count] != NULL) ) {
                printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
	}

	if (TokCount != 2) {
                printf("dident mangae to split group recodr. Bad cn? Group record  was \"%s\", nr of TokCount was %i\n",groupname,TokCount);
                return 0;
        }

	strscpy(groupname,Data[1],sizeofgroupname);

        free(gropurecord);
        FreeSplitList(Data);

	printf("getGroupFromDnGroup: end\n");

	return 1;
}

/**********************************************************************
  Henter ut primær grupe fra en cn. Den er på formatet
  CN=Runar Buvik,CN=Users,DC=bbtest,DC=searchdaimon,DC=com
  der "Runar Buvik" er brukeren og "Users" er primær gruppe.
***********************************************************************/
int getPrimaryGroupFromDnUsername (char cn[],char username[], int sizeofusername) {

	char **Data;
	int TokCount,Count;
	char *gropurecord;
	printf("getPrimaryGroupFromDnUsername: start\ncn : \"%s\"\n",cn);

	TokCount = split(cn, ",", &Data);

	if (TokCount < 2) {
		printf("dident mangae tp split cn. Bad cn? cn was \"%s\"\n",cn);
		return 0;
	}

	Count = 0;
	while( (Data[Count] != NULL) ) {
		printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
	}

	gropurecord = malloc(strlen(Data[1]) +1);
	strcpy(gropurecord,Data[1]);

	FreeSplitList(Data);

	TokCount = split(gropurecord, "=", &Data);

	if (TokCount != 2) {
                printf("dident mangae tp split group recodr. Bad cn? Group record  was \"%s\", nr of TokCount was %i\n",username,TokCount);
                return 0;
        }

	strscpy(username,Data[1],sizeofusername);

	free(gropurecord);
	FreeSplitList(Data);

	printf("getPrimaryGroupFromDnUsername: end\n");

	return 1;

}



void connectHandler(int socket) {
	struct packedHedderFormat packedHedder;
	int intresponse;
	char user_username[64];
        char user_password[64];
	int i;

	printf("got new connection\n");

//temp: ser ikke ut til at apcahe lokker sin enne riktig
//while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {
if ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

	printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
        packedHedder.size = packedHedder.size - sizeof(packedHedder);

	const char *authenticatmetod = bconfig_getentrystr("authenticatmetod");

	if (strcmp(authenticatmetod,"msad") == 0) {

	   	LDAP *ld;
	   	char adminsdistinguishedName[512]; 
		char cn[MAX_LDAP_ATTR_LEN];
		char ldaprecord[MAX_LDAP_ATTR_LEN];
		struct AuthenticatedUserFormat *userobjekt;
		char *k;

		int nrOfSearcResults;
		char **respons;
		char filter[128];
		
		//henter konfi verdier
		const char *msad_domain 	= bconfig_getentrystr("msad_domain");
   		const char *admin_username 	= bconfig_getentrystr("msad_user");
   		const char *admin_password 	= bconfig_getentrystr("msad_password");
   		const char *ldap_host 		= bconfig_getentrystr("msad_ip");
   		const char *ldap_domain 	= bconfig_getentrystr("msad_domain");
   		const int   ldap_port 		= bconfig_getentryint("msad_port");
		const char *ldap_group 		= bconfig_getentrystr("msad_group");
		const char *msad_ldapstring 	= bconfig_getentrystr("msad_ldapstring");
		const char *msad_ldapbase 	= bconfig_getentrystr("msad_ldapbase");
		const char *sudo		= bconfig_getentrystr("sudo");

		char ldap_base[528] = "";

		if (msad_ldapbase == NULL) {
			ldap_genBaseName(ldap_base,ldap_domain);
		}
		else {
			strscpy(ldap_base,msad_ldapbase,sizeof(ldap_base));
		}
		if (msad_ldapstring == NULL) {
			sprintf(adminsdistinguishedName,"cn=%s,cn=%s,%s",admin_username,ldap_group,ldap_base);
		}
		else {
			strscpy(adminsdistinguishedName,msad_ldapstring,sizeof(adminsdistinguishedName));
		}

   		if (!ldap_connect(&ld,ldap_host,ldap_port,ldap_base,adminsdistinguishedName,admin_password)) {
			printf("can't connect to ldap server\n");
			return;
   		}

	        if (packedHedder.command == bad_askToAuthenticate) {



			//lser brukernavn og passord
			recvall(socket,user_username,sizeof(user_username));
			recvall(socket,user_password,sizeof(user_password));

			printf("got username \"%s\", password \"%s\"\n",user_username,user_password);

		   	//struct AuthenticatedUserFormat *AuthenticatedUser;
		   	//AuthenticatedUser = malloc(sizeof(struct AuthenticatedUserFormat));
			

		   	printf("aa %i %i\n",RETURN_SUCCESS,EXIT_FAILURE);
			printf("sudo: \"%s\"\n",sudo);

			if ((sudo != NULL) && (strcmp(user_password,sudo) == 0)) {
				printf("sudo!\n");
				intresponse = ad_userauthenticated_OK;
			}
			else if (ldap_authenticat (&ld,user_username,user_password,ldap_base,ldap_host,ldap_port)) {
				printf("Main: user authenticated\n");
				intresponse = ad_userauthenticated_OK;
			}
			else {
				intresponse = ad_userauthenticated_ERROR;
				printf("Main: user NOT authenticated\n");
			}
	
			sendall(socket,&intresponse, sizeof(intresponse));

			//leger brukeren til i logged inn oversikten
			
			if (intresponse == ad_userauthenticated_OK) {
				printf("*****************************************\n");

				k=strdup(user_username);

				userobjekt = malloc(sizeof(struct AuthenticatedUserFormat));


				strcpy((*userobjekt).username,user_username);
				strcpy((*userobjekt).password,user_password);

				printf("adding user \"%s\" to user hash\n",k);
				if (! hashtable_insert(gloabal_user_h,k,userobjekt) )
				{    
					printf("can't isert\n"); 
					exit(-1);
		               	}

				printf("hashtable_count %u\n",hashtable_count(gloabal_user_h));

				printf("*****************************************\n");


				
			}
	
		}
		else if (packedHedder.command == bad_listGroups) {

			sprintf(filter,"(objectClass=group)");			
			if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base)) {
                                printf("can't ldap search\n");
                                intresponse = 0;
                                sendall(socket,&intresponse, sizeof(intresponse));
                                return;
                        }

			//sender antal
                        sendall(socket,&nrOfSearcResults, sizeof(nrOfSearcResults));

                        printf("found %i gruups\n",nrOfSearcResults);
                        for(i=0;i<nrOfSearcResults;i++) {
                                printf("user \"%s\"\n",respons[i]);
                                strscpy(ldaprecord,respons[i],sizeof(ldaprecord));
                                sendall(socket,ldaprecord, sizeof(ldaprecord));

                        }

			ldap_simple_free(respons);

		}
		else if (packedHedder.command == bad_listUsers) {


			sprintf(filter,"(objectClass=user)");			
			if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base)) {
                                printf("can't ldap search\n");
                                intresponse = 0;
                                sendall(socket,&intresponse, sizeof(intresponse));
                                return;
                        }

			//sender antal
                        sendall(socket,&nrOfSearcResults, sizeof(nrOfSearcResults));

                        printf("found %i gruups\n",nrOfSearcResults);
                        for(i=0;i<nrOfSearcResults;i++) {
                                printf("user \"%s\"\n",respons[i]);
                                strscpy(ldaprecord,respons[i],sizeof(ldaprecord));
                                sendall(socket,ldaprecord, sizeof(ldaprecord));

                        }

			ldap_simple_free(respons);
		}
		else if (packedHedder.command == bad_groupsForUser) {
			printf("listUsers\n");
			char primarygroup[64];

			recvall(socket,user_username,sizeof(user_username));

			printf("user_username %s\n",user_username);

			if (!ldap_getcnForUser(&ld,cn,user_username,ldap_base)) {
                		printf("canr look up cn\n");
				intresponse = 0;
				sendall(socket,&intresponse, sizeof(intresponse));
				return;
        		}

			//finner hovedgruppe fra cn
			if (!getPrimaryGroupFromDnUsername (cn,primarygroup,sizeof(primarygroup))) {
				printf("cnat fins group name.\n");
			}
			printf("group %s\n",primarygroup);

		        sprintf(filter,"(distinguishedName=%s)",cn);

			if (!ldap_simple_search(&ld,filter,"memberOf",&respons,&nrOfSearcResults,ldap_base)) {
                		printf("can't ldap search\n");
				intresponse = 0;
				sendall(socket,&intresponse, sizeof(intresponse));
				return;
		        }
			printf("ldap_simple_search done. Found %i groups\n",nrOfSearcResults);
			
			// +2:
			// +1 for "Everyone" gruppen som alle er medlem av, men ikke finne. Windows :(
			// +1 for primær gruppe
			intresponse = nrOfSearcResults +2;

			//sender antall
			if (!sendall(socket,&intresponse, sizeof(intresponse))) {
				perror("sendall");
			}

			//sender primær gruppe
			strscpy(ldaprecord,primarygroup,sizeof(ldaprecord));
			if (!sendall(socket,ldaprecord, sizeof(ldaprecord))) {
                                perror("sendall");
                        }

			//sender Everyone
			strscpy(ldaprecord,"Everyone",sizeof(ldaprecord));

			if (!sendall(socket,ldaprecord, sizeof(ldaprecord))) {
				perror("sendall");
			}


			printf("found %i gruups\n",nrOfSearcResults);
			for(i=0;i<nrOfSearcResults;i++) {
				printf("gruup \"%s\"\n",respons[i]);
				strscpy(ldaprecord,respons[i],sizeof(ldaprecord));
				//gjør om til gruppe navn, ikke ldap navn
				getGroupFromDnGroup(ldaprecord,ldaprecord,sizeof(ldaprecord));
				if (!sendall(socket,ldaprecord, sizeof(ldaprecord))) {
					perror("sendall");
				}

			}
			ldap_simple_free(respons);			

		}
		else if (packedHedder.command == bad_getPassword) {
			printf("bad_getPassword: start\n");

			recvall(socket,user_username,sizeof(user_username));
			printf("user_username \"%s\"\n",user_username);

			printf("hashtable_count %u\n",hashtable_count(gloabal_user_h));

			if ( (userobjekt  = hashtable_search(gloabal_user_h,user_username)) == NULL)
		      	{    
				printf("not found!\n");
				intresponse = 0;

				if (!sendall(socket,&intresponse, sizeof(int))) {
                                        perror("sendall");
                                }

				
			}
			else {
				printf("found\n");
				printf("password \"%s\"\n",(*userobjekt).password);

				intresponse = 1;
				if (!sendall(socket,&intresponse, sizeof(int))) {
                                        perror("sendall");
                                }

				strscpy(user_password,(*userobjekt).password,sizeof(user_password));
				if (!sendall(socket,user_password, sizeof(user_password))) {
					perror("sendall");
				}

			}                  

			printf("bad_getPassword: end\n");
//printf("exiting to show pd\n");
//exit(1);
		}
		else {
			printf("unnown comand. %i at %s:%d\n", packedHedder.command,__FILE__,__LINE__);
		}

		printf("ldap_close\n");
   		ldap_close(&ld);

	}
	else {
		printf("unknown authenticatmetod %s\n",authenticatmetod);

	}

	printf("end off while\n");
}

printf("connectHandler: end\n");
}

void badldap_init() {
   	if (bconfig_getentrystr("msad_user") == NULL) {
		printf("cant read config for msad_user\n");
		exit(1);
	}
   	if (bconfig_getentrystr("msad_password") == NULL) {
		printf("cant read config for msad_password\n");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_ip") == NULL) {
		printf("cant read config for msad_ip\n");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_domain") == NULL) {
		printf("cant read config for msad_domain\n");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_port") == NULL) {
		printf("cant read config for msad_port\n");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_group") == NULL) {
		printf("cant read config for msad_group\n");
		exit(1);
        }

}

static unsigned int boithoad_hashfromkey(void *ky)
{
    char *k = (char *)ky;

    printf("hashfromkey: \"%s\"\n",k);

    return (crc32boitho(k));
}

static int boithoad_equalkeys(void *k1, void *k2)
{
	printf("equalkeys: \"%s\" ? \"%s\"\n",k1,k2);

	return (0 == strcmp(k1,k2));
}


int main( int argc, char *argv[] ) {

   bconfig_init();

   gloabal_user_h = create_hashtable(16, boithoad_hashfromkey, boithoad_equalkeys);


   badldap_init();

        sconnect(connectHandler, BADPORT);

        printf("conek ferdig \n");

        exit(0);




}



