#ifndef _BBDOCUMENT__H_
#define _BBDOCUMENT__H_

#ifdef BLACK_BOX

#include <db.h>
#include "../ds/dcontainer.h"
#include "../common/bprint.h"

struct uriindexFormat {
        unsigned int DocID;
        unsigned int lastmodified;
};


// routine for initialization.
int bbdocument_init(container **attrkeys);
void bbdocument_clean();

/*
 bbdocument_exist()

 routine to check whether a document exists. If it does not, use bbdocument_add to add it.
 subname	Which collection this is. As a general rule we have one collection per source, and per user. For example 
		for 24so search we have a user named per. He has the collection 24soper_mail for his mail.
		The collection name must be supplied on the command line.

 documenturi

 lastmodified

*/
int bbdocument_exist(char subname[],char documenturi[],unsigned int lastmodified, unsigned int no_lottery);

/*
 bbdocument_add()

 subname        Which collection this is. As a general rule we have one collection per source, and per user. For example
                for 24so search we have a user named per. He has the collection 24soper_mail for his mail.
                The collection name must be supplied on the command line. 

 documenturi

 documenttype	Which type. Set to NULL to let the system guess it based on the filename

 document	The document itself. dokument_size bytes long.

 dokument_size	The size of the document in bytes.

 lastmodified

 acl
*/
// routine to add a document
int bbdocument_add(char subname[],char documenturi[],char documenttype[],char document[],const int dokument_size,unsigned int 
	lastmodified,char *acl_allow, char *acl_denied, const char title[],char doctype[], char *attributes, container *attrkeys,
	char *image, int image_size, unsigned char PopRank);


struct hashtable; /* so everyone who include bbdocument.h does not have to include hashtable.h */

//int bbdocument_convert(char filetype[],char document[],const int dokument_size,char **documentfinishedbuf,int *documentfinishedbufsize
//	, const char titlefromadd[]);
int bbdocument_convert(
			char filetype[],
			char document[],
			const int dokument_size,
			buffer *documentfinishedbuf,
			const char titlefromadd[], 
			char *subname, 
			char *documenturi, 
			unsigned int lastmodified, 
			char *acl_allow, 
			char *acl_denied,
			struct hashtable **metahash
		);



int bbdocument_close (container *attrkeys);
int bbdocument_delete (char uri[], char subname[]);

int uriindex_add (char uri[], unsigned int DocID, unsigned int lastmodified, char subname[]);
int uriindex_get (char uri[], unsigned int *DocID, unsigned int *lastmodified, char subname[]);
int uriindex_close (DB **dbpp);
int uriindex_open(DB **dbpp, char subname[], u_int32_t flags);

unsigned int bbdocument_nrOfDocuments(char subname[]);
int bbdocument_deletecoll(char collection[]);

#endif
#endif
