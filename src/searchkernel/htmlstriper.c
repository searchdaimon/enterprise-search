#include <ctype.h>
#include <stdio.h>
#include <string.h>

void htmlstrip(char html[],char *text,int buffersize) {
	int i;
	int open;
	int textnr = 0;

	#ifdef DEBUG
	printf("htmlstrip(html=\"%s\", buffersize=%d, strlen(html)=%d)\n",html,buffersize,strlen(html));
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
			printf("ignoring format char c %c (uchar %d)\n",html[i],(unsigned char)html[i]);			
			#endif
		}
		else {
			#ifdef DEBUG
			printf("c %c into %d as uchar %d\n",html[i],textnr,(unsigned char)html[i]);
			#endif
			text[textnr] = html[i];
			textnr++;
		}
		//Runarb: 1 feb 2009: Hvorfor fjerner vi alle disse andre låvlige tegnene?
		// da vi ikke bruker denne funksjonen til noe gjør jeg bare om på den.
		//hvis dette skal brukes til noe annet en snipet må man også tilate utf-8 (tegn større en uchar  128)			
		//er alfanumerisk eller SPACE eller "," eller "."
		//else if ( isalnum((int)html[i]) || (html[i] == ' ') || (html[i] == ',') || (html[i] == '.') || (html[i] == '(') || (html[i] == ')') || (html[i] == ':') || (html[i] == '/') || (html[i] == '-')  ) {
		//	//printf("%c",html[i]);
		//	text[textnr] = tolower(html[i]);
		//	textnr++;
		//}
		//else {
		//	printf("ignoring unknown value %c (int %u)\n",html[i],(unsigned char)html[i]);
		//}
	} 

	text[textnr] = '\0';

	#ifdef DEBUG	
		printf("~htmlstrip(textnr=%d, i=%d)\n",textnr,i);	
	#endif

}
