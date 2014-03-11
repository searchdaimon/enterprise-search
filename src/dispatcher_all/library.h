#ifndef _H_DISP_LIBRARHY 
#define _H_DISP_LIBRARHY 

#include <mysql.h>

#include "../common/define.h"


#define MAXDATASIZE 100 // max number of bytes we can get at once

//ikke sikert 1 vil være for altid, da vi blant annent snart vil støtte clustring, men trenger desperat
//og få ned stack støreslen, så valgrin fungerer igjen
#ifdef BLACK_BOX
	#define maxServers 30
#else
	#define maxServers 100
#endif


//i hundredels sekunder (100 = 1sec)
#define maxSocketWait_SiderHeder 6001
#define maxSocketWait_CanDo 6002
#define maxSocketWait_Connect 6003

#define maxerrors 5
#define maxerrorlen 201

#define _OUT_FOMRAT_SD 1
#define _OUT_FOMRAT_OPENSEARCH 2
#define _OUT_FOMRAT_SD_JSON 3
   
#define salt "sdjbjolQdfgkkf"

#define DefultMaxsHits 20


struct errorhaFormat {
	int nr;
	int errorcode[maxerrors];
	char errormessage[maxerrorlen][maxerrors];
};

struct dispconfigFormat {
	char usecashe;
	char useprequery;
	char writeprequery;
	const char *UrlToDocID;

	const char *webdb_host;
	const char *webdb_user;
	const char *webdb_password;
	const char *webdb_db;

	char **bannedwords;
	int bannedwordsnr;
};

void dumpQueryDataForamt(struct QueryDataForamt *d);

void die(int errorcode,char query[] ,const char *fmt, ...);
void bsConnectAndQuery(int *sockfd,int server_cnt, char **servers, 
	struct queryNodeHederFormat *header, 
	struct subnamesFormat *colls, int colls_cnt,
	int offset, int port);
	
void bsConectAndQueryOneServer(char server[], int searchport, char query[], char subname[], int maxHits, int start,
        struct SiderFormat **Sider, int *pageNr);
void addError(struct errorhaFormat *errorha, int errorcode,char errormessage[]);
void brGetPages(int *sockfd,int nrOfServers,struct SiderHederFormat *SiderHeder,struct SiderFormat *Sider, int *pageNr,int alreadynr);

void mysql_search_logg(MYSQL *demo_db, struct QueryDataForamt *QueryData,
        struct SiderHederFormat *FinalSiderHeder, int totlaAds,
        struct queryNodeHederFormat *queryNodeHeder, int nrOfServers, struct SiderFormat *Sider, int nrOfPiServers);

void dieLog(MYSQL *demo_db, struct QueryDataForamt *QueryData, int errorcode, char query[] ,const char *fmt, ...);

#ifdef WITH_CASHE

#define CACHE_STRUCT_VERSION "1.8"

enum cache_type {
        CACHE_PREQUERY,
        CACHE_SEARCH
};


unsigned int cache_hash(char *query, int start, char *country);

char *cache_path(char *path, size_t len, enum cache_type type, char *query, int start, char *country, int anonymous, char search_user[], struct subnamesFormat *collections, int num_colls);

int cache_read(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
           size_t sider_header_len, struct SiderFormat *sider, int cachetimeout, size_t max_sizer);

int cache_write(char *path, int *page_nr, struct SiderHederFormat *final_sider, struct SiderHederFormat *sider_header,
            size_t sider_header_len, struct SiderFormat *sider, size_t sider_len);
#endif
#endif
