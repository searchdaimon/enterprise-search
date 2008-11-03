%{
// (C) Copyright SearchDaimon AS 2008, Magnus Galåen (mg@searchdaimon.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ds/dcontainer.h"
#include "../ds/dvector.h"

#include "attribute_descriptions.h"


// --- fra flex:
typedef void* yyscan_t;
typedef struct adf_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE adf_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct adf_yy_extra *adfget_extra( yyscan_t yyscanner );
// ---


static inline int adf_findc(container *V, char *str)
{
    int		i;

    for (i=0; i<vector_size(V); i++)
	if (!strcmp(vector_get(V,i).ptr, str))
	    return i;

    return -1;
}

static inline int adf_find(char **A, int start, int stop, char *str)
{
    int		i;

    if (str==NULL) return -1;

    for (i=start; i<stop; i++)
	if (!strcmp(A[i], str))
	    return i;

    return -1;
}


struct adf_yacc_data
{
    int		modus;
    int		lang_size, keys_size, values_size;
    container	*lang, *keys_icon, *values_icon, *keys2values;
    container	*key_attr, *value_attr;
    container	**keys, **values;
};

%}

%pure-parser
%parse-param { struct adf_yacc_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token EQUALS_ID STRING_ID BRACKET_BEGIN BRACKET_CLOSE COMMA_ID SEMICOLON_ID LANG_ID NAME_ID KEY_ID VALUE_ID ICON_ID

%%
doc	: lang list_of_keys
	;
lang	: lang_id EQUALS_ID string_list SEMICOLON_ID
	{
	    int		i;

	    data->lang_size = vector_size( data->lang );
	    data->modus++;

	    printf("lang = {");
	    for (i=0; i<data->lang_size; i++)
		{
		    if (i>0) printf(",");
		    printf("%s", vector_get( data->lang, i ).ptr);
		}
	    printf("}\n");

	    data->keys = malloc(sizeof(container*) * data->lang_size);
	    data->values = malloc(sizeof(container*) * data->lang_size);
	    data->keys2values = vector_container( int_container() );
	    data->key_attr = vector_container( string_container() );
	    data->value_attr = vector_container( string_container() );

	    for (i=0; i<data->lang_size; i++)
		{
		    data->keys[i] = vector_container( string_container() );
		    data->values[i] = vector_container( string_container() );
		}

	    data->keys_icon = vector_container( string_container() );
	    data->values_icon = vector_container( string_container() );

	    data->keys_size = 0;
	    data->values_size = 0;
	}
	;
lang_id	: LANG_ID
	{
	    data->modus = 1;
	    data->lang = vector_container( string_container() );
	}
	;
list_of_keys :
	| list_of_keys keys
	;
keys	: KEY_ID STRING_ID BRACKET_BEGIN keys_ids BRACKET_CLOSE
	{
	    int		i;

	    data->keys_size++;

	    vector_pushback(data->key_attr, $2);
	    vector_pushback(data->keys2values, data->values_size);

	    for (i=0; i<data->lang_size; i++)
		{
		    if (vector_size(data->keys[i]) < data->keys_size)
			vector_pushback(data->keys[i], NULL);
		}

	    if (vector_size(data->keys_icon) < data->keys_size)
		vector_pushback(data->keys_icon, NULL);
	}
	;
keys_ids :
	| keys_ids NAME_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
//	    printf("  group_name = \"%s\"\n", $4);
	    vector_pushback( data->keys[0], $4 );
	}
	| keys_ids NAME_ID STRING_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
	    int		lang_no = adf_findc(data->lang, (char*)$3);

	    if (lang_no<0) fprintf(stderr, "attribute_descrptions: Parse error! Invalid lang-specifier.\n");
	    else vector_pushback( data->keys[lang_no], $5 );
	}
	| keys_ids ICON_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
	    vector_pushback( data->keys_icon, $4 );
	}
	| keys_ids values
	;
values	: VALUE_ID STRING_ID BRACKET_BEGIN values_ids BRACKET_CLOSE
	{
	    int		i;

	    data->values_size++;
	    vector_pushback(data->value_attr, $2);

	    for (i=0; i<data->lang_size; i++)
		{
		    if (vector_size(data->values[i]) < data->values_size)
			vector_pushback(data->values[i], NULL);
		}

	    if (vector_size(data->values_icon) < data->values_size)
		vector_pushback(data->values_icon, NULL);
	}
	;
values_ids :
	| values_ids NAME_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
	    vector_pushback( data->values[0], $4 );
	}
	| values_ids NAME_ID STRING_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
	    int		lang_no = adf_findc(data->lang, (char*)$3);

	    if (lang_no<0) fprintf(stderr, "attribute_descrptions: Parse error! Invalid lang-specifier.\n");
	    else vector_pushback( data->values[lang_no], $5 );
	}
	| values_ids ICON_ID EQUALS_ID STRING_ID SEMICOLON_ID
	{
	    vector_pushback( data->values_icon, $4 );
	}
	;
string_list : STRING_ID
	{
	    if (data->modus == 1)
	        vector_pushback( data->lang, $1 );
	}
	| string_list COMMA_ID STRING_ID
	{
	    if (data->modus == 1)
	        vector_pushback( data->lang, $3 );
	}
	;
%%


struct adf_data* adf_init( char *conf_file )
{
    FILE	*fyyin = fopen(conf_file, "r");

    if (fyyin==NULL)
	{
    	    fprintf(stderr, "attribute_descriptions: Error! Could not open file '%s'.\n", conf_file);
	    return NULL;
	}

    struct adf_yacc_data	*data = malloc(sizeof(struct adf_yacc_data));

    yyscan_t		scanner;

    adflex_init(&scanner);
    adfset_extra(data, scanner);
    adfset_in(fyyin, scanner);

    printf("attribute_descriptions: Running scanner...\n");

    adfparse(data, scanner);

    printf("attribute_descriptions: Done.\n");

    adflex_destroy(scanner);
    fclose(fyyin);

    struct adf_data	*adata = malloc(sizeof(struct adf_data));
    int			i, j;

    printf("Values: "); println(data->value_attr);

    adata->lang_size = data->lang_size;
    adata->keys_size = data->keys_size;
    adata->values_size = data->values_size;

    adata->lang = malloc(sizeof(char*) * adata->lang_size);
    adata->keys2values = malloc(sizeof(int) * adata->keys_size);
    adata->keys_icon = malloc(sizeof(char*) * adata->keys_size);
    adata->values_icon = malloc(sizeof(char*) * adata->values_size);

    adata->key_attr = malloc(sizeof(char*) * adata->keys_size);
    adata->value_attr = malloc(sizeof(char*) * adata->values_size);

    adata->keys = malloc(sizeof(char**) * adata->lang_size);
    adata->values = malloc(sizeof(char**) * adata->lang_size);

    for (i=0; i<adata->lang_size; i++)
	{
	    adata->keys[i] = malloc(sizeof(char*) * adata->keys_size);
	    adata->values[i] = malloc(sizeof(char*) * adata->values_size);
	}

    for (i=0; i<adata->lang_size; i++)
	adata->lang[i] = strdup(vector_get(data->lang,i).ptr);

    for (j=0; j<data->keys_size; j++)
	{
	    adata->key_attr[j] = strdup(vector_get(data->key_attr,j).ptr);
	    adata->keys2values[j] = vector_get(data->keys2values,j).i;

	    char	*ptr = vector_get(data->keys_icon,j).ptr;
	    if (ptr!=NULL) adata->keys_icon[j] = strdup(ptr);
	    else adata->keys_icon[j] = NULL;
	}

    for (i=0; i<adata->values_size; i++)
	{
	    adata->value_attr[i] = strdup(vector_get(data->value_attr,i).ptr);

	    char	*ptr = vector_get(data->values_icon,i).ptr;
	    if (ptr!=NULL) adata->values_icon[i] = strdup(ptr);
	    else adata->values_icon[i] = NULL;
	}


    for (i=0; i<adata->lang_size; i++)
	{
    	    for (j=0; j<data->keys_size; j++)
		{
		    char	*ptr = vector_get(data->keys[i],j).ptr;
		    if (ptr!=NULL) adata->keys[i][j] = strdup(ptr);
		    else adata->keys[i][j] = NULL;
		}

    	    for (j=0; j<data->values_size; j++)
		{
		    char	*ptr = vector_get(data->values[i],j).ptr;
		    if (ptr!=NULL) adata->values[i][j] = strdup(ptr);
		    else adata->values[i][j] = NULL;
		}
	}

    // Deallocate internal memory used:
    destroy(data->lang);
    destroy(data->keys_icon);
    destroy(data->values_icon);

    for (i=0; i<data->lang_size; i++)
	{
	    destroy(data->keys[i]);
	    destroy(data->values[i]);
	}

    free(data->keys);
    free(data->values);

    free(data);

    return adata;
}

/*
struct adf_data
{
    int		lang_size, group_size, descr_size, ext_size;
    char	**lang, **ext, **version;
    int		*ext2descr, *ext2group;
    char	***group, ***descr;
};
*/
void adf_destroy(struct adf_data *adata)
{
    int		i, j;

    for (i=0; i<adata->lang_size; i++)
	free(adata->lang[i]);

    for (i=0; i<adata->keys_size; i++)
	free(adata->keys_icon[i]);

    for (i=0; i<adata->values_size; i++)
	free(adata->values_icon[i]);

    for (i=0; i<adata->keys_size; i++)
	free(adata->key_attr[i]);

    for (i=0; i<adata->values_size; i++)
	free(adata->value_attr[i]);

    for (i=0; i<adata->lang_size; i++)
	{
	    for (j=0; j<adata->keys_size; j++)
		free(adata->keys[i][j]);

	    for (j=0; j<adata->values_size; j++)
		free(adata->values[i][j]);

	    free(adata->keys[i]);
	    free(adata->values[i]);
	}

    free(adata->lang);
    free(adata->keys_icon);
    free(adata->values_icon);
    free(adata->keys2values);
    free(adata->key_attr);
    free(adata->value_attr);
    free(adata->keys);
    free(adata->values);
    free(adata);
}


int adf_get_val_descr(struct adf_data *adata, char *lang, char *key, char *value, char **description, char **icon)
{
    if (adata==NULL) return 0;
    int		lang_no = adf_find(adata->lang, 0, adata->lang_size, lang);

    printf("lang_no = %i\n", lang_no);
    if (lang_no<0)
	{
	    fprintf(stderr, "attribute_descriptions: Warning! Unknown language \"%s\". Using default language \"%s\" instead.\n", lang, adata->lang[0]);
	    lang_no = 0;
	}

    int		key_no = adf_find(adata->key_attr, 0, adata->keys_size, key);

    printf("key_no = %i\n", key_no);
    if (key_no<0) return 0;	// No such key

    int		start = 0;
    if (key_no > 0) start = adata->keys2values[key_no-1];

    printf("start=%i, stop=%i, max=%i\n", start, adata->keys2values[key_no], adata->values_size);
    int		value_no = adf_find(adata->value_attr, start, adata->keys2values[key_no], value);
    printf("value_no = %i\n", value_no);

    if (value_no<0) return 0;	// No such value for 'key'.

    if (adata->values[lang_no][value_no]!=NULL)
	*description = adata->values[lang_no][value_no];
    else
	*description = adata->values[0][value_no];

    if (adata->values_icon[value_no]!=NULL)
	*icon = adata->values_icon[value_no];
    else
	*icon = adata->keys_icon[key_no];

    return 1;	// Successful.
}


int adf_get_key_descr(struct adf_data *adata, char *lang, char *key, char **description, char **icon)
{
    if (adata==NULL) return 0;
    int		lang_no = adf_find(adata->lang, 0, adata->lang_size, lang);

    if (lang_no<0)
	{
	    fprintf(stderr, "attribute_descriptions: Warning! Unknown language \"%s\". Using default language \"%s\" instead.\n", lang, adata->lang[0]);
	    lang_no = 0;
	}

    int		key_no = adf_find(adata->key_attr, 0, adata->keys_size, key);

    if (key_no<0) return 0;	// No such key

    if (adata->keys[lang_no][key_no]!=NULL)
	*description = adata->keys[lang_no][key_no];
    else
	*description = adata->keys[0][key_no];

    *icon = adata->keys_icon[key_no];

    return 1;	// Successful.
}


adferror( struct adf_yacc_data *data, void *yyscan_t, char *s )
{
    fprintf(stderr, "attribute_descriptions: Parse error! %s\n", s);
}

