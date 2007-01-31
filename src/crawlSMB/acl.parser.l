%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acl.parser.h"

//#define	ACL_ALLOWED	1
//#define ACL_DENIED	2
/*
REVISION:1,
OWNER:BUILTIN\Administrators,
GROUP:AD-BOITHO-COM\Domain Users,
ACL:AD-BOITHO-COM\Administrator:0/0/0x001200a9,
ACL:NT AUTHORITY\SYSTEM:0/0/0x001f01ff,
ACL:BUILTIN\Users:0/0/0x001200a9
*/
/*

doc := statement[, statement]
statement := REVISION:<tall>
statement := OWNER:<id>
statement := GROUP:<id>
statement := ACL:<id>:<mask>
id := <domain>\<user>
mask := <cmd>/0/<rights>

<MASK_RIGHTS>0x00100116	{ printf("write\n"); BEGIN(INITIAL); }
<MASK_RIGHTS>0x00120089	{ printf("read\n"); BEGIN(INITIAL); }
<MASK_RIGHTS>0x001200a9	{ printf("read, read & execute\n"); BEGIN(INITIAL); }
<MASK_RIGHTS>0x001201bf	{ printf("read, read & execute, write\n"); BEGIN(INITIAL); }
<MASK_RIGHTS>0x001301bf	{ printf("read, read & execute, write, modify\n"); BEGIN(INITIAL); }
<MASK_RIGHTS>0x001f01ff	{ printf("read, read & execute, write, modify, full control\n"); BEGIN(INITIAL); }

*/
const long int	mask_read = 0x00120089;
int	is_acl, read_everyone;
char	*user;
int	allowed;
char	buf[1024];
int	buf_pos;


%}

%option nomain noyywrap
%x	NUMBER DOMAIN USER MASK_CMD MASK_RIGHTS
%%
REVISION:		{ /*printf("Revision: ");*/		is_acl = 0;	BEGIN(NUMBER); }
OWNER:			{ /*printf("Owner:\n");*/		is_acl = 0;	BEGIN(DOMAIN); }
GROUP:			{ /*printf("Group:\n");*/		is_acl = 0;	BEGIN(DOMAIN); }
ACL:			{ /*printf("ACL:\n");*/			is_acl = 1;	BEGIN(DOMAIN); }
<NUMBER>[0-9a-zA-Z]*	{ /*printf("%s\n", yytext);*/		BEGIN(INITIAL); }
<DOMAIN>[^\\]*\\	{ /*printf("\tDomain: %s\n", yytext);*/	BEGIN(USER); }
<USER>[^:,]*		{
			    /*printf("\tUser/Group: %s\n", yytext);*/
			    if (user!=NULL) free(user);
			    user = strdup(yytext);

			    if (is_acl) BEGIN(MASK_CMD);
			    else BEGIN(INITIAL);
			}
<MASK_CMD>0\/[0-9]+\/	{ /*printf("\tALLOWED ");*/ allowed = 1;	BEGIN(MASK_RIGHTS); }
<MASK_CMD>1\/[0-9]+\/	{ /*printf("\tDENIED ");*/ allowed = 0;		BEGIN(MASK_RIGHTS); }
<MASK_RIGHTS>0x[0-9a-fA-F]+	{
			    /*printf("%s\n", ((strtol(yytext, NULL, 0) & mask_read) == mask_read ? "read_access" : "none"));*/

			    if ((strtol(yytext, NULL, 0) & mask_read) == mask_read && allowed)
				{
				    if (user!=NULL && !strcmp(user, "Everyone"))
					{
					    snprintf(buf, 1023, "Everyone");
					    read_everyone = 1;
					}

				    if (!read_everyone)
					{
					    if (buf_pos>0)
						buf_pos+= snprintf(buf + buf_pos, 1023-buf_pos, ",%s", user);
					    else
						buf_pos+= snprintf(buf + buf_pos, 1023-buf_pos, "%s", user);
					}
				}

			    BEGIN(INITIAL);
			}
,			{ /*printf("---\n");*/ }
<*>.|\n			{}
%%


char* parseacl_read_access( char *acl_code )
{
//    printf("acl: %s\n\n", acl_code);
    user = NULL;
    buf_pos = 0;
    read_everyone = 0;

    // Vi skiller ikke mellom brukere over domener. Derfor dropper vi domenenavnet fra brukernavnet.
//    yylex_init();
    YY_BUFFER_STATE	bs;
    bs = yy_scan_bytes( acl_code, strlen(acl_code) );
    yylex();
    yy_delete_buffer( bs );
//    yylex_destroy();
    if (user!=NULL) free(user);

    buf[buf_pos] = '\0';
    return strdup(buf);
}

/*
int main()
{
    char	*textdump = "REVISION:1,OWNER:SELJE\\magnusga,GROUP:SELJE\\fall_s,ACL:SELJE\\magnusga:0/0/0x001f01ff,ACL:SELJE\\fall_s:0/0/0x00120089,ACL:\\Everyone:0/0/0x00120089";
    vector 	*V = getl_read_access(textdump);
    int		i;

    for (i=0; i<V->size; i++)
//	printf("%i\n", V->elem[i]);
	printf("%s (%s)\n", ((struct strint*)V->elem[i].ptr)->str, (((struct strint*)V->elem[i].ptr)->i == ACL_ALLOWED ? "true" : "false"));
}
*/
