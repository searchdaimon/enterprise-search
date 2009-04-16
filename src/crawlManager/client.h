int cmc_conect(int *socketha,char statusbuff[],int statusbufflen, int port);

int cmc_crawl(int socketha,char collection_inn[], char *);

int cmc_crawlcanconect (int socketha, char vantcollection[], char statusbuff[],int statusbufflen);

void cmc_close(int socketha);

int cmc_scan(int socketha,char **respons_list[],int *nrofresponses,char **errormsgp,char crawlertype_in[],
                char host_in[],char username_in[],char password_in[]);

int cmc_pathaccess(int socketha,char collection_in[], char uri_in[], char user_in[], char password_in[]);

int cmc_deleteCollection(int socketha,char collection_in[], char **errormsgp);

int cmc_collectionislocked(int socketha, char *collection_in);

int cmc_groupsforuserfromusersystem(int socketha, char *_user, unsigned int usersystem, char ***_groups);
int cmc_collectionsforuser(int sock, char *_user, char **groups);
int cmc_usersystemfromcollection(int sock, char *collection);
int cmc_listusersus(int sock, int usersystem, char ***users);


int cmc_removeForeignUsers(int sock, char *collection);
int cmc_addForeignUsers(int sock, char *collection, char *inuser, char *ingroup);
	
int cmc_rewrite_url(int socketha, char *collection_in, const char *url_in, size_t urlinlen, enum platform_type ptype,
                enum browser_type btype, char *url_out, size_t url_out_len, char *uri_out, size_t uri_out_len,
                char *fulluri_out, size_t fulluri_out_len);
