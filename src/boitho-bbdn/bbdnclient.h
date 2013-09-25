#ifndef _BBDHCLIENT__H_
#define _BBDHCLIENT__H_

#include "../common/gcwhisper.h"


int bbdn_conect(int *socketha, char tkey[], int PORT);


int bbdn_docadd(int socketha, const char subname[], const char documenturi[], const char documenttype[], const char document[],
        const int dokument_size, const unsigned int lastmodified, const char *acl_allow, const char *acl_denied, const char title[],
        const char doctype[], const char *attributes);

int bbdn_docexist(int socketha, char subname[],char documenturi[],unsigned int lastmodified);

int bbdn_closecollection(int socketha, char subname[]);
int bbdn_deleteuri(int socketha, const char subname[], const char *uri);
int bbdn_deletecollection(int socketha, const char subname[]);
int bbdn_addwhisper(int sock, char *subname, whisper_t whisper);
int bbdn_HasSufficientSpace(int socketha, char subname[]);
void bbdn_opencollection(int sock, char *subname);


int bbdn_close(int socketha);

#endif //_BBDHCLIENT__H_
