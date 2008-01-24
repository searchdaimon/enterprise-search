/*
 *	(C) Boitho 2007-2008, Written by Magnus Galåen
 *
 */

#ifndef _READ_THESAURUS_H_
#define _READ_THESAURUS_H_


#include "../ds/dcontainer.h"
#include "query_parser.h"


/*
 *	Returns		map< vector<string>, vector<it.node> >
 */
container* read_thesaurus( char text[], int text_size );


/*
 *	Get synonyms
 *	Input 'text':	vector<string>
 *	Returns		vector< vector<string> >
 */
container* get_synonyms( container *C, container *text );

/*
 *	Clear synonym-datastructure
 */
void destroy_synonyms( container *C );


#endif	// _READ_THESAURUS_H_
