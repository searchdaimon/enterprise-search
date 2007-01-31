#ifndef _BBDHCLIENT__H_
#define _BBDHCLIENT__H_


int bbdn_conect(int *socketha, char tkey[]);


int bbdn_docadd(int socketha,char subname[],char documenturi[],char documenttype[],char document[],
        int dokument_size,unsigned int lastmodified,char *acl, char title[],char doctype[]);

int bbdn_docexist(int socketha, char subname[],char documenturi[],unsigned int lastmodified);

int bbdn_closecollection(int socketha, char subname[]);

#endif //_BBDHCLIENT__H_
