#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "../logger/logger.h"

void htmlstrip(char html[],char *text,int buffersize) {
	int i;
	int open;
	int textnr = 0;

	#ifdef DEBUG
		bblog(DEBUG, "htmlstrip(html=\"%s\", buffersize=%d, strlen(html)=%d)",html,buffersize,strlen(html));
	#endif

	for (i=0; ((html[i] != '\0') && (i < buffersize)); i++) {

		if (html[i] == '<') {
			open = 1;
		}
		else if (html[i] == '>') {
			open = 0;
			text[textnr] = ' ';
                        textnr++;

		}
		else if (open) {

		}
		else if ((unsigned char)html[i] < 11) {
			#ifdef DEBUG
				bblog(DEBUG, "ignoring format char c %c (uchar %d)",html[i],(unsigned char)html[i]);			
			#endif
		}
		else {
			#ifdef DEBUG
				bblog(DEBUG, "c %c into %d as uchar %d",html[i],textnr,(unsigned char)html[i]);
			#endif
			text[textnr] = html[i];
			textnr++;
		}
	} 

	text[textnr] = '\0';

	#ifdef DEBUG	
		bblog(DEBUG, "~htmlstrip(textnr=%d, i=%d)",textnr,i);	
	#endif

}
