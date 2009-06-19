%{

#include <stdio.h>
#include <string.h>

#include "rac.tab.h"
#include "show_attributes_common.h"

#define rac_warning(s) printf("Warning at line %i: %s\n", 5, s);

%}

blank		[\ \t\r\f]
id		[a-z_]
%option nomain noyywrap reentrant bison-bridge
%x LIST
%%
\#[^\n]*\n		{ /* Ignore comments. */ racget_extra(yyscanner)->line++; }
expanded		{ return EXPANDED_ID; }
group			{ return GROUP_ID; }
import			{ return IMPORT_ID; }
name			{ return NAME_ID; }
select			{ return SELECT_ID; }
hide			{ return HIDE_ID; }
show.duplicates		{ /* ignore */ }
show.empty		{ return SHOW_EMPTY_ID; }
show.max		{ return SHOW_MAX_ID; }
sort.reverse		{ return SORT_REVERSE_ID; }
build_groups[^\n]*	{ /* ignore */ }
sort			{ return SORT_ID; }
from			{ /* ignore */ }
\;{blank}*\n		{ /* Ignore */ racget_extra(yyscanner)->line++; }
\=			{ return EQUALS_ID; }
\(			{ return PARANTES_BEGIN; }
\)			{ return PARANTES_CLOSE; }
\{			{ return BRACKET_BEGIN; }
\}			{ return BRACKET_CLOSE; }
\"			{
			    struct rac_yy_extra	*re = racget_extra(yyscanner);

			    re->space = 0;
			    re->next = 1;

			    BEGIN LIST;
			}
<LIST>(\"{blank}*\,{blank}*\"|\,|\=)	{
			    struct rac_yy_extra	*re = racget_extra(yyscanner);

			    if (!re->next)
				{
				    re->buf[re->ptr++] = '\0';
				    *racget_lval(yyscanner) = (int)&(re->buf[re->last_ptr]);
				    re->last_ptr = re->ptr;
				    if (re->last_ptr > 3072) re->ptr = re->last_ptr = 0;
				}
			    re->next = 1;
			    return STRING_ID;
			}
<LIST>{blank}+		{ racget_extra(yyscanner)->space = 1; }
<LIST>[^\"\,\=\ \t\r\f\n]+	{
			    struct rac_yy_extra	*re = racget_extra(yyscanner);

			    if (re->space && !re->next) re->buf[re->ptr++] = ' ';

			    re->ptr+= snprintf(&(re->buf[re->ptr]), 4095 - re->ptr, "%s", yytext);
			    re->next = 0;
			    re->space = 0;
			}
<LIST>\"		{
			    struct rac_yy_extra	*re = racget_extra(yyscanner);

			    if (!re->next)
				{
				    re->buf[re->ptr++] = '\0';
				    *racget_lval(yyscanner) = (int)&(re->buf[re->last_ptr]);
				    re->last_ptr = re->ptr;
				    if (re->last_ptr > 3072) re->ptr = re->last_ptr = 0;
				}

			    BEGIN INITIAL;
			    return STRING_ID;
			}
\,			{ bprintf(racget_extra(yyscanner)->Bwarnings, "line %i: Misplaced comma.\n", racget_extra(yyscanner)->line); }
\n			{ racget_extra(yyscanner)->line++; }
.			{}
%%
