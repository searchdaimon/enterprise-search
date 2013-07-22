/*****************************************************************/
/* Output of unicode buffers with conversion into target encoding*/
/* And application of substitution maps                          */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 1996-2003, (c) Alex Ott 2003	             */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "catdoc.h"
/************************************************************************/
/* performs paragraph formatting if wrap_margin is non-zero             */
/* gets character sequence and appends it to buffer. If buffer is long  */
/* enough, prints its beginning out                                     */
/* Input parameter is pointer to the string which represents one unicode*/
/* character after character set translation. See convert_char ina      */
/* charset.c                                                            */
/************************************************************************/
static char outputbuffer[LINE_BUF_SIZE]="";
void out_char(const char *chunk) {
	static int bufpos=0;
	int eol_flag=0;
	const char *p; char *q;
	if (!wrap_margin) {
		fputs(chunk,stdout);
		return;
	}

	for (q=outputbuffer+bufpos,p=chunk;
			*p;
			*(q++)=*(p++),bufpos++) {
			if (*p=='\n') eol_flag=1;
	}		
	*q=0;
		 /* This strcat is safe. wrap margin setting
							 code in main.c ensures that wrap_margin is 
							 less than LINE_BUF_SIZE-strlen(largest chunk)
							 */  
	if (eol_flag) {
		/* End of paragraph */
		char *q = map_subst(spec_chars,'\n');
		fputs(outputbuffer,stdout);
		*outputbuffer=0;
		bufpos=0;
		if (q) {
			 fputs(q,stdout);
		} else {
			fputc('\n',stdout);
		}	
	} else if (bufpos>wrap_margin) {
		char *q=outputbuffer,*p=outputbuffer+wrap_margin;
		
		while (p>outputbuffer&&!isspace(*p)) p--;
		if (p==outputbuffer) {
			/*worst case - nowhere to wrap. Will use brute force */
			fwrite(outputbuffer,wrap_margin,1,stdout);
			fputc('\n',stdout);
			p=outputbuffer+wrap_margin;
		} else {
			*p=0;p++;
			fputs(outputbuffer,stdout);
			fputc('\n',stdout);
		}
		for(q=outputbuffer;*p;p++,q++) *q=*p;
		bufpos=q-outputbuffer;
		*q=0;
	}
}

/************************************************************************/
/* Main output function.
 * Programs which read word-processor files should accumulate paragraphs
 * in the buffer as array of unicode 16-bit codes and pass to this
 * function
 ************************************************************************/ 
void output_paragraph(unsigned short int *buffer) {
	unsigned short int *p;
	int countout=0;
	for (p=buffer;*p;p++) {
		out_char(convert_char(*p));
		countout++;
	}
}
