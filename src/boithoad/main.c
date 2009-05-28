#include <assert.h>
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
#include <err.h>
#include <mysql.h>

#include "../boithoadClientLib/boithoad.h"

#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/config.h"
#include "../common/bstr.h"
#include "../common/logs.h"
#include "../common/boithohome.h"
#include "../common/sid.h"
#include "../common/ht.h"
#include "../common/crc32.h"
#include "userobjekt.h"

#include "../libcache/libcache.h"
#include "../slicense/license.h"

#include "../common/list.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

#define RETURN_SUCCESS 1
#define RETURN_FAILURE 0

#define MAX_LDAP_ATTR_LEN 512

#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"
#define MYSQL_DB "boithobb"

/*
The sizelimit argument returns the number of matched entries specified for a search operation. When sizelimit 
is set to 50, for example, no more than 50 entries are returned. When sizelimit is set to 0, all matched 
entries are returned. The LDAP server can be configured to send a maximum number of entries, different from 
the size limit specified. If 5000 entries are matched in the database of a server configured to send a maximum 
number of 500 entries, no more than 500 entries are returned even when sizelimit is set to 0.

- http://docs.sun.com/app/docs/doc/816-5170/ldap-search-ext-3ldap?a=view
*/
#define ldap_sizelimit 0
//timout for ldap kall, i sekkunder
#define ldap_timeout 60

cache_t *cache;
pthread_mutex_t global_user_lock = PTHREAD_MUTEX_INITIALIZER;
static struct hashtable  *gloabal_user_h = NULL;

static unsigned int boithoad_hashfromkey(void *ky)
{
    char *k = (char *)ky;

    //printf("hashfromkey: \"%s\"\n",k);

    return (crc32boitho(k));
}

static int boithoad_equalkeys(void *k1, void *k2)
{
	//printf("equalkeys: \"%s\" ? \"%s\"\n",k1,k2);

	return (0 == strcmp(k1,k2));
}


int
user_enabled(char *user, const char *licensekey)
{
	MYSQL *db;
	char *query;
	char *query_isactive = "SELECT user FROM activeUsers where user ='%s'";
	char *query_enabled_users = "SELECT count(*) FROM activeUsers";
	MYSQL_RES *res;
	MYSQL_ROW row;
	int n;
	int maxusers;
	int enabled_users;

	if ((db = mysql_init(NULL)) == NULL)
		errx(1, "mysql_init, out of memory: %s", mysql_error(db));

	if (!mysql_real_connect(db, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 3306, NULL, 0))
		errx(1, "Mysql connect error: '%s'. Exiting.", mysql_error(db));

	//fiiner hvormange brukere vi har tilgjengelig.
	if (mysql_real_query(db, query_enabled_users, strlen(query_enabled_users)))
		errx(1, "Unable to get active users: %s", mysql_error(db));

	if ((res = mysql_store_result(db)) == NULL)
		errx(1, "mysql_store_result: %s", mysql_error(db));

	row = mysql_fetch_row(res);
	enabled_users = atoi(row[0]);

	printf("enabled_users: %i\n",enabled_users);

	maxusers = get_number_of_licenced_users(licensekey);
	printf("You have a license for %d users.\n", maxusers);

	if (maxusers == -1) {
		errx(1, "Invalide user count license.");
	}

	if (enabled_users > maxusers) {
		errx(1, "You have more user enabled then you have license fore.");
	}

	if (asprintf(&query,query_isactive,user) == -1) {
		errx(1, "Can't alloc memory for query.");
	}
	if (mysql_real_query(db, query, strlen(query)))
		errx(1, "Unable to get active users: %s", mysql_error(db));

	if ((res = mysql_store_result(db)) == NULL)
		errx(1, "mysql_store_result: %s", mysql_error(db));

	n = mysql_num_rows(res);

	free(query);
	if (n != 1) {
		return 0;
	}

	mysql_close(db);

	return 1; //har tilgang
}




void connectHandler(int socket);


int ldap_connect(LDAP **ld, const char ldap_host[] , int ldap_port,const char base[],const char distinguishedName[], const char password[]) {

   int  result;
   int  auth_method = LDAP_AUTH_SIMPLE;
   int desired_version = LDAP_VERSION3;
   //char root_dn[512]; 
	
   printf("ldap_connect(host=%s, user=%s, base=%s)\n",ldap_host,distinguishedName,base);

   #ifdef DEBUG
   //setter at ldap også skal vise debug info
   int debug = 1;
   if( ber_set_option( NULL, LBER_OPT_DEBUG_LEVEL, &debug ) != LBER_OPT_SUCCESS ) {
                fprintf( stderr, "Could not set LBER_OPT_DEBUG_LEVEL %d\n", debug );
		exit(-1);
   }
   if( ldap_set_option( NULL, LDAP_OPT_DEBUG_LEVEL, &debug )
                        != LDAP_OPT_SUCCESS )
   {
   	fprintf( stderr, "Could not set LDAP_OPT_DEBUG_LEVEL %d\n", debug );
	exit(-1);
   }
   #endif


   if (((*ld) = (LDAP *)ldap_init(ldap_host, ldap_port)) == NULL) {
      perror("ldap_init failed");
      return RETURN_FAILURE;
   }


   printf("~ldap_init\n");
   /* referrals */
   if( ldap_set_option( (*ld), LDAP_OPT_REFERRALS, LDAP_OPT_OFF ) != LDAP_OPT_SUCCESS )
   {
        fprintf( stderr, "Could not set LDAP_OPT_REFERRALS off\n" );
	exit( EXIT_FAILURE );
   }
   printf("~ldap_set_option\n");

   /* set the LDAP version to be 3 */
   if (ldap_set_option((*ld), LDAP_OPT_PROTOCOL_VERSION, &desired_version) != LDAP_OPT_SUCCESS)
   {
      ldap_perror((*ld), "ldap_set_option");
	return RETURN_FAILURE;
   }
   printf("~ldap_set_option\n");

   //hvis vi vil bruke sasl tilkobling. Tror det er en type secure layer tilkobling, men er ikke sikker.
   #if 0
   struct berval cred;
   cred.bv_val = password;
   cred.bv_len = strlen(password);
   struct berval *msgidp=NULL;


   if (ldap_sasl_bind_s((*ld), distinguishedName, LDAP_SASL_SIMPLE, &cred, NULL, NULL,&msgidp) != LDAP_SUCCESS ) {
	fprintf(stderr,"Can't conect. Tryed to bind ldap server at %s:%i\n",ldap_host,ldap_port);
      ldap_perror( (*ld), "ldap_bind" );
    	return RETURN_FAILURE;
   }
   #else
   if (ldap_bind_s((*ld), distinguishedName, password, auth_method) != LDAP_SUCCESS ) {
	fprintf(stderr,"Can't conect. Tryed to bind ldap server at %s:%i\n",ldap_host,ldap_port);
      ldap_perror( (*ld), "ldap_bind" );
    	return RETURN_FAILURE;
   }
   #endif
   printf("~ldap_bind_s\n");

   printf("~/ldap_connect\n");
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
  ldap_base[0] = '\0';
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

static void print_reference(LDAP *ld, LDAPMessage *reference)
{
        int rc;
        char **refs = NULL;
        LDAPControl **ctrls;

	/*
        if( ldif < 2 ) {
                printf(_("# search reference\n"));
        }
	*/
	rc = ldap_parse_reference( ld, reference, &refs, &ctrls, 0 );


        if( rc != LDAP_SUCCESS ) {
 //               tool_perror( "ldap_parse_reference", rc, NULL, NULL, NULL, NULL );
                exit( EXIT_FAILURE );
        }

        if(refs) {
#if 0
                int i;
                for( i=0; refs[i] != NULL; i++ ) {
                        tool_write_ldif( ldif ? LDIF_PUT_COMMENT : LDIF_PUT_VALUE,
                                "ref", refs[i], strlen(refs[i]) );
			printf("ref \"%s\", len %i\n",refs[i], strlen(refs[i]));
                }
#endif
                ber_memvfree( (void **) refs );
        }
	

	if( ctrls ) {
                //tool_print_ctrls( ld, ctrls );
                ldap_controls_free( ctrls );
        }
}

int compare_ldap_vals (const void *p1, const void *p2) {
	#ifdef DEBUG
	//printf("compare_ldap_vals (p1=\"%s\", p2\"%s\")\n",(*(char **)p1), (*(char **)p2));
	#endif
	return (strcmp((*(char **)p1),(*(char **)p2)) == 0);
}

/* 
 * ldap simple search flags
 */
#define LS_NONE
// Don't filter object sids
#define LS_WANT_OBJECTSID 0x1


int ldap_simple_search(LDAP **ld,char filter[],char vantattrs[],char **respons[],int *nrofresponses,const char ldap_base[]) {
	return ldap_simple_search_count(ld, filter, vantattrs, respons, nrofresponses, ldap_base, -1, NULL, 0);
}


int ldap_simple_search_count(LDAP **ld,char filter[],char vantattrs[],char **respons[],int *nrofresponses,const char ldap_base[], int maxcount, const char *valfilter, int flags) {

	printf("ldap_simple_search_count( filter=\"%s\", vantattrs=\"%s\", ldap_base=\"%s\", maxcount=%i ,valfilter=\"%s\" )\n",filter,vantattrs,ldap_base,maxcount,valfilter);

   	int  result;
	int i,len,count;
	List list;
	(*nrofresponses) = 0;

  	char **attrs;
  	int TokCount;

	#ifdef DEBUG
  	printf("Splitting: \"%s\" on \"%s\"\n", vantattrs, ",");
	#endif

  	TokCount = split(vantattrs, ",", &attrs);

	#ifdef DEBUG
  	printf("\tfound %d token(s):\n", TokCount);
	#endif

  	count = 0;
  	while( (attrs[count] != NULL) ) {
		#ifdef DEBUG
    		printf("\t\t%d\t\"%s\"\n", count, attrs[count]);
		#endif
		count++;
	}
	#ifdef DEBUG
	printf("\n");
	#endif

   	BerElement* ber;
   	LDAPMessage* msg = NULL;
   	//LDAPMessage* msg2;
   	LDAPMessage* entry;

	struct tempresultsFormat *tempresults;

   	char *dn = NULL;
   	char *attr;
   	char **vals;
   	//int msgid;
	ber_int_t             msgid;
   	//struct timeval tm;
	struct timeval ldap_time_out;
	ldap_time_out.tv_sec 	= ldap_timeout;
	ldap_time_out.tv_usec 	= 0;


   	int nrOfSearcResults;


	int rc;

	#ifdef DEBUG
	printf("trying to ldap_search_ext ...\n");
	#endif
   	// ldap_search() returns -1 if there is an error, otherwise the msgid 
   	if ((rc = ldap_search_ext((*ld), ldap_base, LDAP_SCOPE_SUBTREE, filter, attrs, 0, NULL , NULL,&ldap_time_out, ldap_sizelimit,&msgid)) == -1) {
   	   ldap_perror( ld, "ldap_search" );
   	   return RETURN_FAILURE;
   	}

	#ifdef DEBUG
	printf("... ldap_search_ext done\n");
	#endif

        if( rc != LDAP_SUCCESS ) {
                fprintf( stderr, "%s: ldap_search_ext: %s (%d)\n",
                        __FILE__, ldap_err2string( rc ), rc );
                return 0;
        }



	static char     *sortattr = NULL;
	LDAPMessage *res;

	//printf("LDAP_RES_SEARCH_ENTRY %i\nLDAP_RES_SEARCH_REFERENCE %i\nLDAP_RES_EXTENDED %i\nLDAP_RES_SEARCH_RESULT %i\nLDAP_RES_INTERMEDIATE %i\n\n",LDAP_RES_SEARCH_ENTRY,LDAP_RES_SEARCH_REFERENCE,LDAP_RES_EXTENDED,LDAP_RES_SEARCH_RESULT,LDAP_RES_INTERMEDIATE);

	nrOfSearcResults = 0;
	list_init(&list,free);

	res = NULL;
	while ((rc = ldap_result( (*ld), LDAP_RES_ANY,
                sortattr ? LDAP_MSG_ALL : LDAP_MSG_ONE,
                NULL, &res )) > 0 )
        {
		#ifdef DEBUG
		printf("result:\n");
		#endif
	
                for ( msg = ldap_first_message( (*ld), res );
                        msg != NULL;
                        msg = ldap_next_message( (*ld), msg ) )
                {
			#ifdef DEBUG
			printf("\ttype %i\n", ldap_msgtype( msg ) );
			#endif

			switch( ldap_msgtype( msg ) ) {
                        case LDAP_RES_SEARCH_ENTRY:
                                //nentries++;
                                //print_entry( (*ld), msg, 0 );

   				/* Iterate through the returned entries */
   				for(entry = ldap_first_entry((*ld), msg); entry != NULL; entry = ldap_next_entry((*ld), entry)) {
      					if((dn = ldap_get_dn((*ld), entry)) != NULL) {
						 printf("Returned dn: %s\n", dn);
						 ldap_memfree(dn);
      					}

      					for( attr = ldap_first_attribute((*ld), entry, &ber); attr != NULL; attr = ldap_next_attribute((*ld), entry, ber)) {
						int counter;

						printf("attr: %s\n",attr);

						//###########################

					 	if ((vals = (char **)ldap_get_values((*ld), entry, attr)) != NULL)  {
							counter = 0;

							printf("rbrb: got %i values\n",ldap_count_values(vals));
							//det ser ut som om ad sender verdiene i fårskjelige rekkefølge
							//sorterer de derfor
							qsort(vals,ldap_count_values(vals),sizeof(char *),compare_ldap_vals);
							

		    					for(i = 0; vals[i] != NULL; i++) {
								if (valfilter &&
								    !((strcmp(attr, "objectSid") == 0) && (flags & LS_WANT_OBJECTSID)) &&
								    strncmp(vals[i], valfilter, strlen(valfilter)) != 0) {
									printf("Skiping: value \"%s\" thats not in filter \"%s\" for att %s\n", vals[i], valfilter,attr);
									continue;
								}
								if (strcasecmp(attr, "objectSid") == 0) {
									char *p = sid_btotext(vals[i]);
									tempresults = malloc(sizeof(struct tempresultsFormat));
									strncpy((*tempresults).value,p,MAX_LDAP_ATTR_LEN);
									strncpy((*tempresults).key,attr,MAX_LDAP_ATTR_LEN);
									if (list_ins_next(&list, NULL, tempresults) != 0) {
										printf("can't insert objectSid into list\n");
										return 0;
									}
									free(p);
								} else {
									printf("attr: %s, vals %s\n", attr, vals[i]);

									tempresults = malloc(sizeof(struct tempresultsFormat));

									strncpy((*tempresults).value,vals[i],MAX_LDAP_ATTR_LEN);
									strncpy((*tempresults).key,attr,MAX_LDAP_ATTR_LEN);
									if (list_ins_next(&list,NULL,tempresults) != 0) {
										printf("cant insert into list\n");
										return 0;
									}	
								}
								++count;	
								counter++;

								if ( (maxcount != -1) && (counter >= maxcount) ) {/* We got what we want... */
									printf("Hit max count. counter=%i, maxcount=%i\n",counter,maxcount);
									break;
								}
							} //for

	

	    						ldap_value_free(vals);
	 					} //if
					//printf("attr adress %u\n",(unsigned int)attr);
					ldap_memfree(attr);

					//###########################
						++nrOfSearcResults;

					}
					ber_free(ber, 0);
				}

                                break;

                        case LDAP_RES_SEARCH_REFERENCE:
                                //nreferences++;
                                print_reference( (*ld), msg );
                                break;

			case LDAP_RES_SEARCH_RESULT:
                                /* pagedResults stuff is dealt with
                                 * in tool_print_ctrls(), called by
                                 * print_results(). */
                                //rc = print_result( ld, msg, 1 );
                                //if ( ldapsync == LDAP_SYNC_REFRESH_AND_PERSIST ) {
                                //        break;
                                //}
				printf("got LDAP_RES_SEARCH_RESULT!\n");
                                goto done;
				

			}
		}
		ldap_msgfree(res);
	}

done:
        if ( rc == -1 ) {
                //tool_perror( "ldap_result", rc, NULL, NULL, NULL, NULL );
		printf("rc == -1\n");
                return 0;
        }

	//clean up
	ldap_msgfree(msg);

  	FreeSplitList(attrs);


	printf("list size if %i, count %i, size %i\n",list_size(&list),count,( sizeof(char *) * (count +1) ));

	(*respons) = malloc(( sizeof(char *) * (count +1) ));

	tempresults = NULL;
	while(list_rem_next(&list,NULL,(void **)&tempresults) == 0) {
		//printf("aaa \"%s\":\"%s\"\n",(*tempresults).key,(*tempresults).value);

		len = strnlen((*tempresults).value,MAX_LDAP_ATTR_LEN);

		(*respons)[(*nrofresponses)] = malloc(len +1);

		strscpy((*respons)[(*nrofresponses)],(*tempresults).value,len +1);

		//printf("into \"%s\"\n",(*respons)[(*nrofresponses)]);

		free(tempresults);
		++(*nrofresponses);
	}
	(*respons)[(*nrofresponses)] = '\0';

	list_destroy(&list);

	printf("nr of results for return is %i\n",(*nrofresponses));

	/*
	if ((*nrofresponses) == 0) {
		printf("dident find any results, returning 0\n"); 
		return 0;
	}
	*/

	//printf("ldap_simple_search: end\n");

	return 1;
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
	#ifdef DEBUG
	printf("userid \"%s\", user_password \"%s\"\n",cn,user_password);
	#endif

   	if (!ldap_connect(&ldforuserconect,ldap_host,ldap_port,ldap_base,cn,user_password)) {
        	printf("cant connect to ldap server by userid \"%s\"\n",cn);
        	return RETURN_FAILURE;
   	}
   	printf("we cod connect. User is Authenticationated\n");

   	ldap_close(&ldforuserconect);


   	return RETURN_SUCCESS;
}
/**********************************************************************
  Henter ut gruppe fra en ldap distinguishedName "objectClass: group"
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
                printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
		Count++;
	}

        gropurecord = malloc(strlen(Data[0]) +1);
        strcpy(gropurecord,Data[0]);


        FreeSplitList(Data);

	printf("gropurecord \"%s\"\n",gropurecord);

        TokCount = split(gropurecord, "=", &Data);

	Count = 0;
	while( (Data[Count] != NULL) ) {
                printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
		Count++;
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
  Henter ut primær gruppe fra en cn. Den er på formatet
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
		printf("\t\t%d\t\"%s\"\n", Count, Data[Count]);
		Count++;
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

void
send_failure(int sock)
{
	int data = 0;
	sendall(sock, &data, sizeof(data));
}

int
insert_group(struct hashtable *grouphash, char *id)
{
	if (hashtable_search(grouphash, id) != NULL)
		return 0;
	hashtable_insert(grouphash, id, (void *)1);

	return 1;
}


void
gather_groups(struct hashtable *grouphash, LDAP **ld, const char *ldap_base, char *sid)
{
	char filter[128];
	char **response;
	int nrOfSearcResults;
	int i;

	printf("gather_groups(ldap_base=%s, sid=%s)\n",ldap_base,sid);


	/* XXX: hack */
	if (strncasecmp(sid, "S-", 2) == 0) {
		sprintf(filter, "(objectSid=%s)", sid);
		if (!ldap_simple_search(ld, filter, "sAMAccountName", &response, &nrOfSearcResults, ldap_base)) {
			printf("Unable to get sAMAccountName from objectSid\n");
			printf("Filter: %s, attributes: %s\n", filter, "sAMAccountName");
		} else {
			char *id;
			printf( "We got something like: %s from %s\n", response[0], sid);

			if (nrOfSearcResults > 0) {
				id = strdup(response[0]);
				if (insert_group(grouphash, id))
					gather_groups(grouphash, ld, ldap_base, id);
				else
					free(id);
				fprintf(stderr, "Foop: %s\n", sid);
			} else {
				fprintf(stderr, "Unable to get account name from: %s\n", sid);
			}
			ldap_simple_free(response);
		}
	} else  {
		sprintf(filter, "(sAMAccountName=%s)", sid);
	}
	if (!ldap_simple_search(ld, filter, "memberOf", &response, &nrOfSearcResults, ldap_base)) {
		printf("Unable to get memberOf\n");
		return;
	}
	for (i = 0; i < nrOfSearcResults; i++) {
		int n_results;
		char **response2;
		char *id;

		if (!ldap_simple_search(ld, NULL, "objectSid", &response2, &n_results, response[i])) {
			printf("Unable to get objectSid\n");
			return;
		}

		if (n_results > 0) {
			id = strdup(response2[0]);
			if (insert_group(grouphash, id))
				gather_groups(grouphash, ld, ldap_base, id);
			else
				free(id);
		} else {
			fprintf(stderr, "Unable to get sid from: %s\n", response[i]);
		}
		ldap_simple_free(response2);
	}
	ldap_simple_free(response);		

	printf("~gather_groups()\n");	
}



int getAllGroupsForUser(struct hashtable **grouphash, LDAP *ld, char *user_username, const char *msad_ldapgroupstring, const char *ldap_domain, const char *ldap_base ) {

	int nrOfSearcResults;
	char **respons;
	char filter[128];


	printf("getAllGroupsForUser(user_username=%s, msad_ldapgroupstrin=%s, ldap_domain=%s, ldap_base=%s)\n",user_username,msad_ldapgroupstring,ldap_domain,ldap_base);

	char primarygroup[64];
	char ldapbasegroup[128];
	char *sid;
	char *groupsid;
	char *id;

	/* We do not care about the specified ldap base when looking for groups */
	/* XXX: Is this correct? */
	if ((msad_ldapgroupstring == NULL) || (msad_ldapgroupstring[0] == '\0')) {
		ldap_genBaseName(ldapbasegroup, ldap_domain);
	}
	else {
		strscpy(ldapbasegroup, msad_ldapgroupstring,sizeof(ldapbasegroup));
	}

	printf("ldapbasegroup: %s\n",ldapbasegroup);
	printf("groupsForUser\n");
	printf("user_username %s\n",user_username);

	fprintf(stderr, "Writing the cache\n");

	/* Make hash table to temporarily hold all group info */
	*grouphash = create_hashtable(7, boithoad_hashfromkey, boithoad_equalkeys);

	/* First figure out the primary group */
	/* 1. Get User SID */
	sprintf(filter, "(sAMAccountName=%s)", user_username);
	printf("Filter: %s\n", filter);
	if (!ldap_simple_search(&ld, filter, "primaryGroupID,objectSid", &respons, &nrOfSearcResults, ldap_base)) {
		printf("Unable to get userSID and primaryGroup\n");
		printf("Filter: %s, attributes: %s\n", filter, "primaryGroupID,objectSid");
		return 0;
	}
	assert(nrOfSearcResults > 0);

	groupsid = NULL;
	if (nrOfSearcResults > 0) {
		sid = malloc(strlen(respons[*respons[0] == 'S' ? 0 : 1]) + 16);
		strcpy(sid, respons[*respons[0] == 'S' ? 0 : 1]);
		//printf("Sid: %s\n", sid);
		if (!insert_group(*grouphash, sid)) {
			free(sid);
		}
		else {
			printf("calling gather_groups() ldapbasegroup=%s, sid=%s\n",ldapbasegroup, sid);
			gather_groups(*grouphash, &ld, ldapbasegroup, sid);
		}
		/* 2. Replace last element */
		groupsid = strdup(sid);
		sid_replacelast(groupsid, respons[*respons[0] == 'S' ? 1 : 0]);
		if (!insert_group(*grouphash, groupsid)) {
			free(groupsid);
		}
		else {
			printf("calling gather_groups() ldapbasegroup=%s, groupsid=%s\n",ldapbasegroup, groupsid);
			gather_groups(*grouphash, &ld, ldapbasegroup, groupsid);
		}
		printf("Primary group: %s\n", groupsid);
		printf("%p\n", respons);
		ldap_simple_free(respons);			
	}

	/* 3. Get group name */
	if (groupsid != NULL) {
		sprintf(filter, "(objectSid=%s)", groupsid);
		if (!ldap_simple_search(&ld, filter, "sAMAccountName", &respons, &nrOfSearcResults, ldapbasegroup)) {
			printf("Unable to get userSID and primaryGroup");
			return 0;
		}
		if (nrOfSearcResults > 0) {
			id = strdup(respons[0]);
			if (!insert_group(*grouphash, id))
				free(id);
		} else {
			fprintf(stderr, "Could not resolve primaryGroup name: %s\n", groupsid);
		}
		ldap_simple_free(respons);			
	}

	/* Add Everyone and the username for the user */
	id = strdup("Everyone");
	if (!insert_group(*grouphash, id))
		free(id);
	id = strdup("S-1-1-0"); /* The SID for Everyone */
	if (!insert_group(*grouphash, id))
		free(id);
	id = strdup(user_username);
	if (!insert_group(*grouphash, id))
		free(id);


	return 1;
}

int userIsLogedIn (char *user_username, const char *user_password, int time_out) {

	int intresponse = 0;
	struct AuthenticatedUserFormat *userobjekt;
	time_t now = time(NULL);
	
	#ifdef DEBUG
		printf("userIsLogedIn(user_username=%s, user_password=%s)\n",user_username,user_password);
	#else
		printf("userIsLogedIn(user_username=%s )\n",user_username);
	#endif

	pthread_mutex_lock(&global_user_lock);
		if ( (userobjekt  = hashtable_search(gloabal_user_h,user_username)) == NULL) {    
			printf("not found!\n");
			intresponse = 0;

		}
		else if ((now - userobjekt->ctime) >= time_out) {
			printf("User objekt was to old. Was %u\n",now - userobjekt->ctime);
			intresponse = 0;
		}
		else if ((strcmp(userobjekt->username,user_username) != 0) || ((strcmp(userobjekt->password,user_password)))) {
			printf("user was already loged in, but pw og username was wrong!\n");
			intresponse = 0;
		}
		else {
	
			intresponse = 1;

		}                  
	pthread_mutex_unlock(&global_user_lock);

	printf("~userIsLogedIn, ret=%i\n",intresponse);

	return intresponse;
}

int
do_request(int socket,FILE *LOGACCESS, FILE *LOGERROR) {

//her driver vi og kaller return, men da blir ikke alt, som loggene, stenkt ned riktig. 
//Må heller bruke goto eller continue
	struct packedHedderFormat packedHedder;
	int intresponse;
	char user_username[64];
        char user_password[64];
	int i;
	unsigned int authentication_timeout;
	int license_system_active = 1;

	printf("got new connection\n");

	//temp: ser ikke ut til at Apache lukker sin ende riktig
	//while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) 
	if ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {
		#ifdef DEBUG
			printf( "size is: %i\nversion: %i\ncommand: %i",packedHedder.size,packedHedder.version,packedHedder.command);
		#endif

		packedHedder.size = packedHedder.size - sizeof(packedHedder);

		bconfig_flush(CONFIG_CACHE_IS_OK);

		const char *authenticatmetod = bconfig_getentrystr("authenticatmetod");

		//henter timeout. Hvis den er 0 så skal vi ikke cache.
		if (bconfig_getentryuint("authentication_timeout", &authentication_timeout) == 0) {
			authentication_timeout = 0;
		}
		//printf("resetting authentication timeout to %u\n",authentication_timeout);
		cache_settimeout(cache, authentication_timeout);

		if (strcmp(authenticatmetod,"msad") == 0) {
			LDAP *ld = NULL;
			char ldaprecord[MAX_LDAP_ATTR_LEN];
			struct AuthenticatedUserFormat *userobjekt;
			char *k;

			int nrOfSearcResults;
			char **respons;
			char filter[128];

			//nulstiller mine, slik at valgrind ikke klager når vi sender det.
			memset(&ldaprecord,'\0',MAX_LDAP_ATTR_LEN);

			//henter konfig verdier
			const char *admin_username 	= bconfig_getentrystr("user");
			const char *admin_password 	= bconfig_getentrystr("password");
			const char *ldap_host 		= bconfig_getentrystr("ip");
			const char *ldap_domain 	= bconfig_getentrystr("domain");
			const int   ldap_port = 0; // runarb: 12 des 2007: er 0 riktig initalisering??
			const char *msad_ldapstring 	= bconfig_getentrystr("ldapstring");
			const char *msad_ldapgroupstring = bconfig_getentrystr("ldapgroupstring");
			const char *msad_ldapbase 	= bconfig_getentrystr("ldapbase");
			const char *sudo		= bconfig_getentrystr("sudo");
			const char *active_usersystem   = bconfig_getentrystr("licensesystem");
			const char *licensekey		= bconfig_getentrystr("licensekey");
			char ldap_base[528] = "";

			if (active_usersystem != NULL && licensekey != NULL && strcmp(active_usersystem, "ohSh7oow") == 0 && licensekey[0] == '\0') {
				fprintf(stderr, "Disabling license system\n");
				license_system_active = 0;
			}

                        /* Flush config file and connect to ldap server */
                        int bad_ldap_connect(void) {

				char adminsdistinguishedName[512];

                                if ((msad_ldapbase == NULL) || (msad_ldapbase[0] == '\0')) {
                                        ldap_genBaseName(ldap_base,ldap_domain);
                                }
                                else {
                                        strscpy(ldap_base,msad_ldapbase,sizeof(ldap_base));
                                }
                                if ((msad_ldapstring == NULL) || (msad_ldapstring[0] == '\0')) {
                                        //bruker brukernavn på formen user@domain.ttl
                                        sprintf(adminsdistinguishedName,"%s@%s",admin_username,ldap_domain);
                                }
                                else {
                                        strscpy(adminsdistinguishedName,msad_ldapstring,sizeof(adminsdistinguishedName));
                                }

                                if (!ldap_connect(&ld,ldap_host,ldap_port,ldap_base,adminsdistinguishedName,admin_password)) {
                                        printf("can't connect to ldap server\n");
					blog(LOGERROR,1,"can't connect to ldap server. Useing server \"%s:%i\" (0 as port nr are OK), ldap_base \"%s\", adminsdistinguishedName \"%s\"",ldap_host,ldap_port,ldap_base,adminsdistinguishedName);
                                        return 0;
                                }

                                return 1;
                        }
			

			if ((packedHedder.command != bad_getPassword) && (packedHedder.command != bad_groupsForUser) && (packedHedder.command != bad_askToAuthenticate) && (!bad_ldap_connect()))
				return 0;

			if (packedHedder.command == bad_askToAuthenticate) {

				int firstOkLogin = 0;
				struct hashtable *grouphash;

				intresponse = ad_userauthenticated_ERROR;

				//lser brukernavn og passord
				recvall(socket,user_username,sizeof(user_username));
				recvall(socket,user_password,sizeof(user_password));

				#ifdef DEBUG
					printf("got username \"%s\", password \"%s\"\n",user_username,user_password);
					printf("sudo: \"%s\"\n",sudo);
				#endif


				if ((sudo != NULL) && (strcmp(user_password,sudo) == 0)) {
					printf("sudo!\n");
					blog(LOGACCESS,1,"sudo to user \"%s\".",user_username);
					intresponse = ad_userauthenticated_OK;
				}
				else if (userIsLogedIn(user_username, user_password, authentication_timeout)) {
					printf("user is already loged in. Returning ok\n");
					intresponse = ad_userauthenticated_OK;
				}
				
				//hvis vi her en ok respons allerede så trenger vi ikke å gjøre noe mer
				if (intresponse == ad_userauthenticated_OK) {
					sendall(socket,&intresponse, sizeof(intresponse));
					return 1;
				}
				else {

					//vi må spørre ldap. Kobler til
					if (!bad_ldap_connect()) {
						blog(LOGERROR,1,"can't connect to ldap server.");
						intresponse = ad_userauthenticated_ERROR;
					}
					else if (ldap_authenticat (&ld,user_username,user_password,ldap_base,ldap_host,ldap_port)) {
						printf("Main: user authenticated\n");
						printf("user_username: \"%s\"\n",user_username);
						blog(LOGACCESS,1,"user \"%s\" successfuly authenticated.",user_username);
						intresponse = ad_userauthenticated_OK;
						firstOkLogin = 1;
					}
					else {
						blog(LOGERROR,1,"could not authenticate user \"%s\".",user_username);
						intresponse = ad_userauthenticated_ERROR;
						printf("Main: user NOT authenticated\n");
					}

					if (intresponse == ad_userauthenticated_OK &&
					    license_system_active && !user_enabled(user_username,licensekey)) {
						printf("%s is not allowed to log in\n", user_username);
						intresponse = ad_userauthenticated_NOACCESS;
						firstOkLogin = 0;
					}


					sendall(socket,&intresponse, sizeof(intresponse));

					//leger brukeren til i logged inn oversikten
					if (firstOkLogin == 1) {
						k = strdup(user_username);
						userobjekt = malloc(sizeof(struct AuthenticatedUserFormat));
						strcpy(userobjekt->username,user_username);
						strcpy(userobjekt->password,user_password);
						userobjekt->ctime = time(NULL);

						printf("adding user \"%s\" to user hash\n",k);
						pthread_mutex_lock(&global_user_lock);
							//fjerner eventuelt gammelt brukerobjekt.
							struct AuthenticatedUserFormat *v;
							v = hashtable_remove(gloabal_user_h, k);
							if (v != NULL)
								free(v);
	
							//setter inn nyt btukerobjekt
							if (!hashtable_insert(gloabal_user_h,k,userobjekt)) {    
								pthread_mutex_unlock(&global_user_lock);
								printf("can't isert user in userobjekt\n"); 
								return 0;
							}
						pthread_mutex_unlock(&global_user_lock);
					}

					//finner brukerens grupper, og cacher disse
					if ((firstOkLogin == 1) && (authentication_timeout != 0)) {
						
						if (!getAllGroupsForUser(&grouphash, ld, user_username, msad_ldapgroupstring, ldap_domain, ldap_base)) {
							printf("can't find group info for user \"%s\"\n",user_username);
						}
						else {
							cache_add(cache, "groupsforuser", user_username, grouphash);
						}
					}
				}


			}
			else if (packedHedder.command == bad_listGroups) {

				sprintf(filter,"(objectClass=group)");			
				if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base)) {
					printf("can't ldap search\n");
					intresponse = 0;
					sendall(socket,&intresponse, sizeof(intresponse));
					//return;
				}
				else {

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

			}
			else if (packedHedder.command == bad_listUsers) {


				//sprintf(filter,"(objectClass=user)");			
				sprintf(filter,"(&(objectCategory=person)(objectClass=user))");			
				if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base)) {
					printf("can't ldap search\n");
					intresponse = 0;
					sendall(socket,&intresponse, sizeof(intresponse));
					//return;
				}
				else {
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
			}
			else if (packedHedder.command == bad_listMailUsers) {

				/*
					Lister mail brukere.
					Runarb: 22 aug 2008.
					Desverre her dette blitt ganske hårete. Vi må hente ut både proxyAddresses og objectSid 
					med det kan være mer en en proxyAddresses. Nå kommer objectSid som elemenrt 1, så 
					proxyAddresses som nr 2.
				*/
				sprintf(filter,"(&(objectClass=user)(mailNickname=*))");			
				if (!ldap_simple_search_count(&ld,filter,"proxyAddresses,objectSid",&respons,&nrOfSearcResults,ldap_base, 1, "SMTP:", LS_WANT_OBJECTSID)) {
					printf("can't ldap search\n");
					intresponse = 0;
					sendall(socket,&intresponse, sizeof(intresponse));
					//return;
				}
				else {
					intresponse = nrOfSearcResults / 2;
					//sender antall
					sendall(socket,&intresponse, sizeof(intresponse));

					printf("found %i mail users\n",nrOfSearcResults);
					for(i=0;i<nrOfSearcResults;i+=2) {
						char *objectSid		= respons[i];
						char *proxyAddresses 	= respons[i +1];

						printf("proxyAddresses=%s\nobjectSid=%s\n",proxyAddresses,objectSid);

						char *p;
						p = strchr(proxyAddresses, ':');
						if (p) {
							p++;

						} else {
							fprintf(stderr, "Invalid smtp address: %s\n", proxyAddresses);
							p=proxyAddresses;
						}

						strlcpy(ldaprecord, p, sizeof(ldaprecord));
						printf("mail adress \"%s\"\n",ldaprecord);
						
						strlcat(ldaprecord,":",sizeof(ldaprecord));
						strlcat(ldaprecord,objectSid,sizeof(ldaprecord));

						printf("mail user \"%s\"\n",ldaprecord);
						sendall(socket,ldaprecord, sizeof(ldaprecord));

					}

					ldap_simple_free(respons);
				}
			}
			else if (packedHedder.command == bad_sidToUser) {
				char objectSid[512];

				recvall(socket,objectSid,sizeof(objectSid));

				printf("runing bad_sidToUser(objectSid=\"%s\")\n",objectSid);

				sprintf(filter,"(&(objectSid=%s)(objectClass=user))", objectSid);			
				if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base) || nrOfSearcResults != 1) {
					printf("can't ldap search\n");
					intresponse = 0;
					sendall(socket,&intresponse, sizeof(intresponse));
					//return;
				}
				else {
					strscpy(user_username, *respons, sizeof(user_username));

					//sender antal
					intresponse = 1;
					sendall(socket,&intresponse, sizeof(intresponse));
					sendall(socket,user_username, sizeof(user_username));

					ldap_simple_free(respons);
				}
			}
			else if (packedHedder.command == bad_sidToGroup) {

				char objectSid[512];

				recvall(socket,objectSid,sizeof(objectSid));

				printf("runing bad_sidToGroup(objectSid=\"%s\")\n",objectSid);


				//sprintf(filter,"(&(objectSid=%s)(objectClass=group))", objectSid);			
				sprintf(filter,"(&(objectSid=%s))", objectSid);			
				if (!ldap_simple_search(&ld,filter,"sAMAccountName",&respons,&nrOfSearcResults,ldap_base) || nrOfSearcResults != 1) {
					printf("can't ldap search\n");
					intresponse = 0;
					sendall(socket,&intresponse, sizeof(intresponse));
					//return;
				}
				else {
					strscpy(user_username, *respons, sizeof(user_username));

					//sender antal
					intresponse = 1;
					sendall(socket,&intresponse, sizeof(intresponse));
					sendall(socket,user_username, sizeof(user_username));

					ldap_simple_free(respons);
				}
			}
			else if (packedHedder.command == bad_groupsForUser) {
				struct hashtable *grouphash;

				int sendgroups(struct hashtable *grouphash) {

					struct hashtable_itr *itr;

					/* Send all the groups */
					intresponse = hashtable_count(grouphash);
					if (!sendall(socket,&intresponse, sizeof(intresponse))) {
						perror("sendall() groups for users, count");
						return 0;
					}
					itr = hashtable_iterator(grouphash);
					do {
						//printf("Foo: %s %d\n", (char *)hashtable_iterator_key(itr),
						//    (int)hashtable_iterator_value(itr));
						strscpy(ldaprecord, hashtable_iterator_key(itr), sizeof(ldaprecord));
						if (!sendall(socket,ldaprecord, sizeof(ldaprecord))) {
							perror("sendall");
							return 0;
						}

					} while (hashtable_iterator_advance(itr));

					free(itr);

					return 1;
				}

				recvall(socket,user_username,sizeof(user_username));

				/* Look at the cache */
				grouphash = cache_fetch(cache, "groupsforuser", user_username);
				if (grouphash != NULL) {
					fprintf(stderr, "Using the cache\n");
					sendgroups(grouphash);
					goto ldap_end;
				}

				if (!bad_ldap_connect())
					return 0;


				if (!getAllGroupsForUser(&grouphash, ld, user_username, msad_ldapgroupstring, ldap_domain, ldap_base)) {
					send_failure(socket);
					goto ldap_end;
				}

				sendgroups(grouphash);
				cache_add(cache, "groupsforuser", user_username, grouphash);
				//hashtable_destroy(grouphash, 0);
			}
			else if (packedHedder.command == bad_getPassword) {
				printf("bad_getPassword: start\n");

				recvall(socket,user_username,sizeof(user_username));
				printf("user_username \"%s\"\n",user_username);

				//printf("hashtable_count %u\n",hashtable_count(gloabal_user_h));

				pthread_mutex_lock(&global_user_lock);
				if ( (userobjekt  = hashtable_search(gloabal_user_h,user_username)) == NULL)
				{    
					printf("not found!\n");
					intresponse = 0;

					if (!sendall(socket,&intresponse, sizeof(int))) {
						perror("sendall");
					}
				}
				else {
					#ifdef DEBUG
						printf("found\npassword \"%s\"\n", userobjekt->password);
					#endif

					intresponse = 1;
					if (!sendall(socket,&intresponse, sizeof(int))) {
						perror("sendall");
					}

					strscpy(user_password,(*userobjekt).password,sizeof(user_password));
					if (!sendall(socket,user_password, sizeof(user_password))) {
						perror("sendall");
					}

				}                  
				pthread_mutex_unlock(&global_user_lock);

				printf("bad_getPassword: end\n");
			}
			else {
				printf("unnown comand. %i at %s:%d\n", packedHedder.command,__FILE__,__LINE__);
			}
			printf("ldap_close\n");
 ldap_end:
 			if (ld != NULL)
				ldap_close(&ld);
		}
		else {
			printf("unknown authenticatmetod %s\n",authenticatmetod);
		}

		printf("end of while\n");
	}

	printf("connectHandler: end\n");

	return 1;
}

void connectHandler(int socket) {
	FILE *LOGACCESS, *LOGERROR;

        if (!openlogs(&LOGACCESS,&LOGERROR,"boithoad")) {
                fprintf(stderr,"can't open logfiles.\n");
        }

	blog(LOGACCESS, 1, "connectHandler: calling do_request");
	do_request(socket,LOGACCESS,LOGERROR);
	close(socket);
	blog(LOGACCESS, 1, "connectHandler: back from do_request");

	closelogs(LOGACCESS,LOGERROR);
	printf("closed logs\n");
}

void badldap_init(FILE *elog) {

   	if (bconfig_getentrystr("msad_user") == NULL) {
		blog(elog, 1, "cant read config for msad_user");
		exit(1);
	}
   	if (bconfig_getentrystr("msad_password") == NULL) {
		blog(elog, 1, "cant read config for msad_password");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_ip") == NULL) {
		blog(elog, 1, "cant read config for msad_ip");
		exit(1);
        }
   	if (bconfig_getentrystr("msad_domain") == NULL) {
		blog(elog, 1, "cant read config for msad_domain");
		exit(1);
        }

}

int
get_number_of_licenced_users(const char *licensekey)
{
	unsigned short int users;
	unsigned int serial;


	if (!get_licenseinfo(licensekey, &serial, &users)) {
		fprintf(stderr, "Invalid license key...");
		return -1;
	}

	return users;
}


void
usage(void)
{
	fprintf(stderr, "boithoad [-l]\n");
	exit(1);
}

#define STDOUTLOG "logs/boithoad_stdout"
#define STDERRLOG "logs/boithoad_stderr"


void
my_freevalue(void *p)
{
	free(p);
}

int
main(int argc, char **argv)
{
	int ch;
	int lflag;
	FILE *logaccess, *logerror;
	unsigned int authentication_timeout;

	lflag = 0;
	while ((ch = getopt(argc, argv, "l")) != -1) {
		switch (ch) {
		case 'l':
			lflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (lflag) {
		fprintf(stderr, "Opening new stdout log\n");
		if (freopen(bfile(STDOUTLOG), "a", stdout) == NULL)
			perror(bfile(STDOUTLOG));
                setvbuf(stdout, NULL, _IOLBF, 0);

		fprintf(stderr, "Opening new stderr log\n");
		if (freopen(bfile(STDERRLOG), "a", stderr) == NULL)
			perror(bfile(STDERRLOG));
		setvbuf(stderr, NULL, _IOLBF, 0);
	}

   	bconfig_flush(CONFIG_NO_CACHE);

	//henter timeout. Hvis den er 0 så skal vi ikke cache.
	if (bconfig_getentryuint("authentication_timeout", &authentication_timeout) == 0) {
		authentication_timeout = 0;
	}

	cache = malloc(sizeof(*cache));
	cache_init(cache, my_freevalue, authentication_timeout); 

	//bconfig_init();
	gloabal_user_h = create_hashtable(16, boithoad_hashfromkey, boithoad_equalkeys);


        if (!openlogs(&logaccess,&logerror,"boithoad")) {
		perror("unable to open logfiles for main boithoad");
	}

	badldap_init(logerror);
	sconnect_thread(connectHandler, BADPORT);
	printf("connect done\n");

	return(0);
}

