/*****************************************************************/
/* Main program for parsing XLS files                            */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) David Rysdam  1998                                        */
/* (c) Victor Wagner 1998-2003, (c) Alex Ott 2003	             */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include "xltypes.h"
#include "catdoc.h"
#include <stdlib.h>
#include <unistd.h>
#include "catdoc.h"
#include "float.h"
#include "xls.h"

#ifdef __TURBOC__
#define strcasecmp(a,b) strcmpi(a,b)
#endif
extern char *forced_date_format; 
extern char number_format[];
extern char *sheet_separator;
/************************************************************************/
/* Displays  help message                                               */
/************************************************************************/
void help (void) {
	printf("Usage:\n xls2csv [-xlV] [-g number] [-f date-format] [-b string] [-s charset] [-d charset] [-c char] [ -q number] files\n");
}
/* Defines unicode chars which should be
   replaced by strings before UNICODE->target chatset
   mappigs are applied i.e. TeX special chars like %
   */
char *input_buffer, *output_buffer;
int main(int argc, char *argv[])
{
	FILE *input;
	FILE *new_file, *ole_file;
	char *filename =NULL;
	short int *tmp_charset;
	int c;
	int i;
	char *tempname;
	read_config_file(SYSTEMRC);
#ifdef USERRC
	tempname=find_file(strdup(USERRC),getenv("HOME"));
	if (tempname) {
		read_config_file(tempname);
		free(tempname);
	}
#endif
#ifdef HAVE_LANGINFO
	       get_locale_charset();
#endif
	
	check_charset(&dest_csname,dest_csname); 
		
	while ((c=getopt(argc,argv,"Vlf:s:d:xq:c:b:g:p:"))!=-1) {
		switch(c)  {
			case 'l':
				list_charsets(); exit(0);
			case 'x': 
				unknown_as_hex = 1; break;
			case 's':
				check_charset(&source_csname,optarg);
				source_charset=read_charset(source_csname);
				break;
			case 'b':
				sheet_separator= strdup(optarg);
				break;
			case 'd':
				check_charset(&dest_csname,optarg);
				break;
			case 'q':
				{   char *errptr;
					quote_mode = strtol(optarg,&errptr,0);
					if ((errptr && *errptr)||quote_mode<0||quote_mode>3) {
						fprintf(stderr,
								"argument of -q should be number from 0 to 3\n");
						exit(1);
					}
				}    
				break;
			case 'c':
				cell_separator = optarg[0];
				break;
			case 'f':
				forced_date_format = strdup(optarg);
				break;
			case 'g': 
					{ char *strend;
					  int digits = strtol(optarg,&strend,0);
					  if (*strend||digits<0||digits>DBL_DIG) {
						  fprintf(stderr,"value of -g option should be numbe between 0 and %d, not '%s'\n", DBL_DIG, optarg);
						  exit(1);
					  }
					  sprintf(number_format,"%%.%dg",digits);
					}
					break;
			case 'V': printf("Catdoc Version %s\n",CATDOC_VERSION);
					  exit(0);
			default:
				help();
				exit(1);
		}	
	}
/* If we are using system strftime, we need to  set LC_TIME locale
 * category unless choosen charset is not same as system locale
 */ 
#if defined(HAVE_LANGINFO) && defined(HAVE_STRFTIME) && !defined(__TURB0C__)
	set_time_locale();
#endif	
	/* charset conversion init*/
	input_buffer=malloc(FILE_BUFFER);
	if (strcmp(dest_csname,"utf-8")) {
		tmp_charset=read_charset(dest_csname);
		if (!tmp_charset) {
			fprintf(stderr,"Cannot load target charset %s\n",dest_csname);
			exit(1);
		}	
		target_charset=make_reverse_map(tmp_charset);
		free(tmp_charset);
	} else { 
		target_charset=NULL;
	} 
	spec_chars=read_substmap(stradd("ascii",SPEC_EXT));
	if (!spec_chars) {
		fprintf(stderr,"Cannod read substitution map ascii%s\n",
				SPEC_EXT);
		exit(1);
	}  
	replacements=read_substmap(stradd("ascii",REPL_EXT));
	if (!replacements) {
		fprintf(stderr,"Cannod read substitution map ascii%s\n",
				REPL_EXT);
		exit(1);
	}  
	if (optind>=argc) {
		if (isatty(fileno(stdin))) {
			help();
			exit(0);
		}    
		do_table(stdin,"STDIN");
		exit (0);
	}	
	for (i=optind;i<argc;i++) {
		filename = argv[i];
		input=fopen(filename,"rb");
		if (!input) {
			perror(filename);
			exit(1);
		}
		if ((new_file=ole_init(input, NULL, 0)) != NULL) {
			set_ole_func();
			while((ole_file=ole_readdir(new_file)) != NULL) {
				int res=ole_open(ole_file);
/* 				fprintf(stderr, "name = %s\n", ((oleEntry*)ole_file)->name); */
				if (res >= 0) {
					if (strcasecmp(((oleEntry*)ole_file)->name , "Workbook") == 0
						|| strcasecmp(((oleEntry*)ole_file)->name,"Book") == 0) {
						do_table(ole_file,filename);
					}
				} 
				ole_close(ole_file);
			}
			set_std_func();
			ole_finish();
			fclose(new_file);
		} else {
			fprintf(stderr, "%s is not OLE file or Error\n", filename);
		}
	}
	return 0;
}
