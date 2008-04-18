
#ifndef _STEMMER_H_
#define _STEMMER_H_


#include "../ds/dcontainer.h"
#include "../ds/dvector.h"
#include "../ds/dset.h"
#include "query_parser.h"


typedef struct
{
    int		id;
    char	*text;
    char	flags;
} tword;


typedef struct
{
    tword	*W, *Id;
    int		W_size, Id_size;
} thesaurus;



// Initialize thesaurus:
thesaurus* thesaurus_init( char *fname_text, char *fname_id );

// Destroy thesaurus:
void thesaurus_destroy( thesaurus* T );

void thesaurus_expand_query( thesaurus *T, query_array *qa );

// Get synonyms.
// Returns vector<string>
container* thesaurus_get_synonyms(thesaurus *T, char *word);

// Returns set<string>
container* thesaurus_get_words_from_id(thesaurus *T, int id);



#endif	// _STEMMER_H_
