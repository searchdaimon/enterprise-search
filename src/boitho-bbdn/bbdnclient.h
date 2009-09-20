#ifndef _BBDHCLIENT__H_
#define _BBDHCLIENT__H_

#include "../common/gcwhisper.h"


int bbdn_conect(int *socketha, char tkey[], int PORT);


int bbdn_docadd(int socketha,char subname[],char documenturi[],char documenttype[],char document[],
        int dokument_size,unsigned int lastmodified,char *acl_allow, char *acl_denied, char title[],char doctype[], char *attributes);

int bbdn_docexist(int socketha, char subname[],char documenturi[],unsigned int lastmodified);

int bbdn_closecollection(int socketha, char subname[]);
int bbdn_deleteuri(int socketha, char subname[], char *uri);

int bbdn_addwhisper(int sock, char *subname, whisper_t whisper);
int bbdn_HasSufficientSpace(int socketha, char subname[]);
void bbdn_opencollection(int sock, char *subname);


#endif //_BBDHCLIENT__H_
