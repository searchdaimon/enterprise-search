/**
 * @file   ppt2text.c
 * @author Alex Ott <alexott@gmail.com>
 * @date   23 ‰≈À 2004
 * Version: $Id: catppt.c,v 1.1.1.1 2007/12/04 18:43:09 boitho Exp $
 * Copyright: Alex Ott 
 * 
 * @brief  main module for text extracting from .ppt
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "ppt.h"
#include "catdoc.h"
#include <stdlib.h>
#include "catdoc.h"
#include "float.h"

#ifdef __TURBOC__
#define strcasecmp(a,b) strcmpi(a,b)
#endif

/** 
 * Displays  help message
 * 
 */
void help (void) {
	printf("Usage:\n ppt2text [-lV] [-b string] [-s charset] [-d charset] files\n");
}


char *input_buffer, *output_buffer;

/** 
 * 
 * 
 * @param argc 
 * @param argv 
 * 
 * @return 
 */
int main(int argc, char *argv[]) {
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

	while ((c=getopt(argc,argv,"Vls:d:p:"))!=-1) {
		switch(c)  {
		case 'l':
			list_charsets(); exit(0);
		case 's':
			check_charset(&source_csname,optarg);
			source_charset=read_charset(source_csname);
			break;
		case 'd':
			check_charset(&dest_csname,optarg);
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
		do_ppt(stdin,"STDIN");
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
					if (strcasecmp(((oleEntry*)ole_file)->name , "PowerPoint Document") == 0) {
						do_ppt(ole_file,filename);
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
