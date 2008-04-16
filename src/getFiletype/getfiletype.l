%{

#include <stdio.h>

#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"

#include "../common/search_automaton.h"

#include "getfiletype.h"


struct readconf_data
{
    container	*ext, *desc;
};

#define YY_EXTRA_TYPE	struct readconf_data*

%}

extchar		[a-z0-9_]
%option noyywrap reentrant
%x DESCR
%%
^{extchar}+		{
			    struct readconf_data	*data = yyget_extra(yyscanner);
			    vector_pushback( data->ext, yytext );
			    BEGIN(DESCR);
			}
<DESCR>[^\ \t][^\n]*	{
			    struct readconf_data	*data = yyget_extra(yyscanner);
			    vector_pushback( data->desc, yytext );
			    BEGIN(INITIAL);
			}
<*>.|\n			{}
%%


filetypes_info* getfiletype_init(char *lang_file)
{
    fprintf(stderr, "getfiletype: getfiletype_init()\n");

    FILE	*fyyin = fopen(lang_file, "r");

    if (fyyin==NULL)
	{
    	    fprintf(stderr, "Could not open file '%s'.\n", lang_file);
	    return NULL;
	}

    struct readconf_data	*data = malloc(sizeof(struct readconf_data));
    data->ext = vector_container( string_container() );
    data->desc = vector_container( string_container() );

    yyscan_t			scanner;

    yylex_init(&scanner);
    yyset_extra(data, scanner);
    yyset_in(fyyin, scanner);

    yylex(scanner);

    yylex_destroy(scanner);

    fclose(fyyin);

    int		size;

    if (vector_size(data->ext) != vector_size(data->desc))
	{
	    fprintf(stderr, "readconf.l: Warning! data->ext != data->desc; Parse error in '%s'.\n", lang_file);
	    if (vector_size(data->ext) < vector_size(data->desc))
		size = vector_size(data->ext);
	    else
		size = vector_size(data->desc);
	}
    else
	size = vector_size(data->ext);

    if (size==0)
	{
	    fprintf(stderr, "readconf.l: Warning! '%s' empty!\n", lang_file);
	    return NULL;
	}

    unsigned char	**A = malloc(sizeof(unsigned char*)*size);
    int			i;

    for (i=0; i<size; i++)
	A[i] = (unsigned char*)vector_get(data->ext, i).ptr;

    filetypes_info	*fti = malloc(sizeof(filetypes_info));
    fti->A = build_automaton(size, A);
    fti->desc = malloc(sizeof(char*)*size);
    fti->size = size;

    for (i=0; i<size; i++)
	fti->desc[i] = strdup(vector_get(data->desc, i).ptr);

    free(A);
    destroy(data->desc);
    destroy(data->ext);
    free(data);

    return fti;
}


char* getfiletype(filetypes_info *fti, char *extension)
{
    if (fti==NULL)
	return NULL;

    int	n = search_automaton(fti->A, extension);

    if (n>=0)
	return fti->desc[n];

    return NULL;
}


void getfiletype_destroy(filetypes_info *fti)
{
    fprintf(stderr, "getfiletype: getfiletype_destroy()\n");
    if (fti==NULL)
	return;

    int		i;
    for (i=0; i<fti->size; i++)
	free(fti->desc[i]);

    free_automaton(fti->A);
    free(fti->desc);
    free(fti);
}

