#include <ctype.h>

void htmlstrip(char html[],char text[],int buffersize) {
	int i;
	int open;
	int textnr = 0;

	for (i=0; ((html[i] != '\0') && (i < buffersize)); i++) {
		if (html[i] == '<') {
			open = 1;
		}
		else if (html[i] == '>') {
			open = 0;
			text[textnr] = ' ';
                        textnr++;

		}			
		//er alfanumerisk eller SPACE eller "," eller "."
		else if ( (!open) && (isalnum((int)html[i]) || (html[i] == ' ') || (html[i] == ',') || (html[i] == '.') || (html[i] == '(') || (html[i] == ')') || (html[i] == ':') || (html[i] == '/') || (html[i] == '-')  ) ) {
			//printf("%c",html[i]);
			text[textnr] = tolower(html[i]);
			textnr++;
		}
	} 
	text[textnr] = '\0';
	
}
