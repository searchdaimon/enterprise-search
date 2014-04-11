#include <stdio.h>                          
#include <string.h>                         

size_t _append(char *buff, size_t buffn, const char *str) {
	size_t i = 0;                                         
	for (; i < strlen(str); i++) {                     
		if (i >= buffn)                            
			return i - 1;
		buff[i] = str[i];
	}
	return i - 1;
}

size_t escapeHTML(char *buff, size_t buffn, const char *str) {

	size_t i = 0, pos = 0;
	size_t strl = strlen(str);
	for (; i < strl; i++, pos++) {
		if (pos >= buffn) {
			buff[pos] = '\0';
			return i;
		}

		switch(str[i]) {
			case '"':
				pos += _append(&(buff[pos]), buffn - pos, "&quot;");
				break;
			case '<':
				pos += _append(&(buff[pos]), buffn - pos, "&lt;");
				break;
			case '>':
				pos += _append(&(buff[pos]), buffn - pos, "&gt;");
				break;
			case '&':
				pos += _append(&(buff[pos]), buffn - pos, "&amp;");
				break;
				
			default: buff[pos] = str[i];
		}
	}
	buff[pos] = '\0';
	return pos;
}

