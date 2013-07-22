/*
  Copyright 1998-2003 Victor Wagner
  Copyright 2003 Alex Ott
  This file is released under the GPL.  Details can be
  found in the file COPYING accompanying this distribution.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_LANGINFO
#include <langinfo.h>
#ifndef __TURBOC__
#include <locale.h>
#endif
#endif
#include "catdoc.h"

char *format_name="ascii";
static int runtime_locale_check=1;
/********************************************************************/
/*  Reads configuration file                                        */
/*                                                                  */
/********************************************************************/
void read_config_file(const char* filename) { 
	FILE *f=fopen(add_exe_path(filename),"rb");
	char *name,*value,line[1024],*c;
	int lineno=0;
	if (!f) return;
	while (!feof(f)) {
		fgets(line,1024,f);
		if (feof(f)) break;
		lineno++;
		if ((c=strchr(line,'#'))) *c='\0';
		name=line;
		while (*name&&isspace(*name)) name++;
		if (!*name) continue;
		for (value=name;*value&&(isalnum(*value)||*value=='_'); value++);  
		if (*value=='=') {
			*value=0;value++;
		} else {
			*value=0;value++;
			while(*value&&isspace(*value)) value++;
			if (*value++ != '=' ) {
				fprintf(stderr,"Error %s(%d): name = value syntax expected\n",
						filename,lineno);
				exit(1);
			}
			while(*value&&isspace(*value)) value++;
		}
		for (c=value;*c&&!isspace(*c);c++);
		if (value==c) {
			fprintf(stderr,"Error %s(%d): name = value syntax expected\n",
					filename,lineno);
			exit(1);
		}
		*c=0;
		if (!strcmp(name,"source_charset")) {
			source_csname=strdup(value);
		} else if (!strcmp(name,"target_charset")) {
			dest_csname=strdup(value);
		} else if (!strcmp(name,"format")) {
			format_name=strdup(value);
		} else if (!strcmp(name,"charset_path")) {
			charset_path=strdup(value);
		} else if (!strcmp(name,"map_path")) {
			map_path = strdup(value);
		} else if (!strcmp(name,"unknown_char")) {
			if (*value=='"' && value[1] && value[2]=='"') value++;	
			if (*value=='\'' && value[1] && value[2]=='\'') value++;	
			bad_char[0] = *value;
		} else if (!strcmp(name,"use_locale")) {
			if (tolower(value[0])=='n') {
				runtime_locale_check=0;
			} else if (tolower(value[0])=='y') {
				runtime_locale_check=1;
			} else {
				fprintf(stderr,"Error %s(%d): use_locale requires 'yes' or 'no'\n",
						filename,lineno);
				exit(1);
			}	

		} else {
			fprintf(stderr,"Invalid configuration directive in %s(%d):,%s = %s\n",
					filename,lineno,name,value);		
			exit(1);
		}	
	}
	fclose(f);
}
#ifdef HAVE_LANGINFO
static char *locale_charset = NULL;
/*********************************************************************/
/*  Determines  output character set from current locale and puts it *
 *  into global variable dest_csname                                 *
 *********************************************************************/
void get_locale_charset() {
	char *codeset;
	if (!runtime_locale_check) return;
#ifndef __TURBOC__	
	if (!setlocale(LC_CTYPE,"")) return;
#endif
	codeset = nl_langinfo(CODESET);
	if (!strncmp(codeset,"ISO",3)||!strncmp(codeset,"iso",3)) {
		codeset+=3;
		if (*codeset=='-') codeset++;
		if (!strncmp(codeset,"646",3)) {
			/* ISO 646 is another name for us=ascii */
			check_charset(&dest_csname,"us-ascii") ;
		}	else {	 
			if (check_charset(&dest_csname,codeset)) {
				locale_charset = dest_csname;
			}
		}
	} else if (!strcmp(codeset,"ANSI_X3.4-1968")) {
		check_charset(&dest_csname,"us-ascii");
	} else if (!strncmp(codeset,"ANSI",4)||!strncmp(codeset,"ansi",4)) {
		char *newstr;
		if (*codeset=='-') {
			codeset++;
		}	
		newstr	= malloc(strlen(codeset)-4+2+1);
		strcpy(newstr,"cp");
		strcpy(newstr+2,codeset+4);
		if (check_charset(&dest_csname,newstr)) {
			locale_charset = dest_csname;
		}
		free(newstr);
	} else if (!strncmp(codeset,"IBM",3)) {
		char *newstr;
		codeset+=3;
		if (*codeset == '-') codeset++;
		newstr=malloc(strlen(codeset)+2+1);
		strcpy(newstr,"cp");
		strcpy(newstr+2,codeset);
		if (check_charset(&dest_csname, newstr)) {
			locale_charset=dest_csname;
		}
		free(newstr);
	} else {
		char *i,*newstr = strdup(codeset);
		for (i=newstr;*i;i++) {
			*i=tolower(*i);
		}	
		if (check_charset(&dest_csname,newstr)) {
			locale_charset = dest_csname;
		}	
	}	

}	
#ifndef __TURBOC__
void set_time_locale() {
	if (!runtime_locale_check) return;
	if (!locale_charset) return;
	if (strcmp(locale_charset,dest_csname)!=0) return;
	setlocale(LC_TIME,"");		
}	
#endif
#endif
#ifndef HAVE_STRDUP
/* Implementation of strdup for systems which don't have it */
char *strdup(const char *s) {
	int size=strlen(s);
	char *newstr;
	newstr=malloc(size+1);
	return strcpy(newstr,s);
}	
#endif
