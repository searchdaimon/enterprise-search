%{
// (C) Copyright Boitho 2005, Magnus Galåen (magnusga@idi.ntnu.no)
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config_parser.h"

// General functions for simple C-implementation of map<char*,char*>:
/************************************************/

// I choose for now not to make these public:
struct mapcc;
struct mapcc_iterator;

struct mapcc
{
    int				size;
    struct mapcc_iterator	*start;
};

struct mapcc_iterator
{
    char			*variable, *value;
    struct mapcc_iterator	*next;
};

struct mapcc * mapcc_new()
{
    struct mapcc	*M = (struct mapcc*)malloc( sizeof(struct mapcc) );

    M->size = 0;
    M->start = (struct mapcc_iterator*)malloc( sizeof(struct mapcc_iterator) );	// Dummy entry.
    M->start->variable = NULL;
    M->start->value = NULL;
    M->start->next = NULL;

    return M;
}

struct mapcc_iterator * mapcc_begin( struct mapcc* M )
{
    return M->start->next;
}

void mapcc_insert( struct mapcc* M, char *variable, char *value )
{
    // Insertion sort:
    struct mapcc_iterator	*it=mapcc_begin(M), *last=M->start;
    for (; it!=NULL; last=it, it=it->next)
	{
	    int		cmp = strcmp( it->variable, variable );

	    if (cmp>0)	// The new entry slips in here:
		{
		    M->size++;
		    last->next = (struct mapcc_iterator*)malloc( sizeof(struct mapcc_iterator) );
		    last->next->variable = strdup( variable );
		    last->next->value = strdup( value );
		    last->next->next = it;
		    return;
		}
	    else if (cmp==0)	// The new entry replaces an old entry:
		{
		    free(it->value);
		    it->value = strdup( value );
		    return;
		}
	}

    // The new entry will be added at the last position:
    M->size++;
    last->next = (struct mapcc_iterator*)malloc( sizeof(struct mapcc_iterator) );
    last->next->variable = strdup( variable );
    last->next->value = strdup( value );
    last->next->next = NULL;
}

char *mapcc_value( struct mapcc* M, char *variable )
{
    struct mapcc_iterator	*it=mapcc_begin(M);
    for (; it!=NULL; it=it->next)
	{
	    int		cmp = strcmp( it->variable, variable );
	    if (cmp>0)
		{
		    return NULL;
		}
	    else if (cmp==0)
		{
		    return it->value;
		}
	}

    return NULL;
}

/************************************************/
// End of general functions.

struct mapcc	*config=NULL;

%}

%token TEXT EQUALS EOLN

%%
doc		:
		| doc set_variable
		| doc empty_line
		;
set_variable	: TEXT EQUALS TEXT EOLN
		{
		    mapcc_insert( config, (char*)$1, (char*)$3 );
		}
		;
empty_line	: EOLN
		;
%%

extern FILE *yyin;
extern int lineno;

char	*config_filename;


int read_config( char *filename )
{
    if (config==NULL)
	{
	    config = mapcc_new();
	}

    config_filename = strdup( filename );
    yyin = fopen( config_filename, "r" );
    if (yyin==NULL) return -1;
    do
	{
	    yyparse();
	}
    while (!feof(yyin));
}

char *config_value( char *variable )
{
    return mapcc_value( config, variable );
}

int config_value_int( char *variable )
{
    return atoi( mapcc_value( config, variable ) );
}

yyerror( char *s )
{
    printf("Error reading config-file \"%s\": %s on line %i\n", config_filename, s, lineno);
    exit(-1);
}
