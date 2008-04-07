 #include "../common/define.h"

    #define MAXDATASIZE 100 // max number of bytes we can get at once


        //ikke sikert 1 vil være for altid, da vi blant annent snart vil støtte clustring, men trenger desperat
        //og få ned stack støreslen, så valgrin fungerer igjen
        #ifdef BLACK_BOKS
            #define maxServers 30
        #else
            #define maxServers 100
        #endif

    //extern int errno;

    //i hundredels sekunder (100 = 1sec)
    #define maxSocketWait_SiderHeder 1000
    #define maxSocketWait_CanDo 100

    #define maxerrors 5
    #define maxerrorlen 201


#define QUERY_LOG_FILE "/home/boitho/logs/query.log"

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
        };


void die(int errorcode,const char *fmt, ...);
void bsConectAndQuery(int *sockfd,int nrOfServers, char *servers[],struct queryNodeHederFormat *queryNodeHeder,int alreadynr, int port);
int bsConectAndQueryOneServer(char server[], int searchport, char query[], char subname[], int maxHits, int start,
        struct SiderFormat **Sider, int *pageNr);
