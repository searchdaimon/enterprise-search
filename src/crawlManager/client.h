/* Include define.h to get enum platform_* */
#include "../common/define.h"

int cmc_conect(int *socketha,char statusbuff[],int statusbufflen, int port);

int cmc_crawl(int socketha,char collection_inn[], char *);

int cmc_crawlcanconect (int socketha, char vantcollection[], char statusbuff[],int statusbufflen);

void cmc_close(int socketha);

int cmc_scan(int socketha,char **respons_list[],int *nrofresponses,char **errormsgp,char crawlertype_in[],
                char host_in[],char username_in[],char password_in[]);

int cmc_pathaccess(int socketha,char collection_in[], char uri_in[], char user_in[], char password_in[]);

int cmc_deleteCollection(int socketha,char collection_in[], char **errormsgp);

int cmc_collectionislocked(int socketha, char *collection_in);

int cmc_groupsforuserfromusersystem(int socketha, char *_user, unsigned int usersystem, char ***_groups, char *extra_in);
int cmc_collectionsforuser(int sock, char *_user, char **groups);
int cmc_usersystemfromcollection(int sock, char *collection);
struct cm_listusers_h cmc_listusersus(int sock, int usersystem, char ***users, char *extra_in);
int cmc_authuser(int sock, const char *user, const char *pass, unsigned int usersystem, char *extra_in);
int cmc_getPassword(const char username_in[], char password[]);


