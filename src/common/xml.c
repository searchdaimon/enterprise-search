#include <sys/types.h>

#include <string.h>
#include <stdio.h>

char *
xml_escape_attr(const char *str, char *buf, size_t len)
{
	size_t i, j;
	const char *p;

	p = str;
	j = 0;
	for (i = 0; *p; p++, i++) {
		switch (*p) {
		case '"':
			if (j+7 >= len)
				return NULL;
			strcpy(buf+j, "&quot;");
			j += 6;
			break;
		case '&':
			if (j+6 >= len)
				return NULL;
			strcpy(buf+j, "&amp;");
			j += 5;
			break;
		default:
			if (j+1 >= len)
				return NULL;
			buf[j++] = *p;
			break;
		}
	}
	buf[j] = '\0';
	
	return buf;
}

char *
xml_escape_uri(char *_arg, char *out, size_t len)
{ 
	int i; 
	unsigned char *arg = (unsigned char*)_arg;
	unsigned char c;
	char *p;

	p = out;
	for (i = 0; (c = arg[i]) != '\0'; i++) { 
		if ((c == 0x2D) || (c == 0x2E) || // Hyphen-Minus, Full Stop
		    ((0x30 <= c) && (c <= 0x39)) || // Digits [0-9]
		    ((0x41 <= c) && (c <= 0x5A)) || // Uppercase [A-Z]
		    ((0x61 <= c) && (c <= 0x7A))) { // Lowercase [a-z]
			if (len < 2)
				return NULL;
			len--;
			*p = c;	
			p++;
		} else {
			if (len < 4)
				return NULL;
			
			snprintf(p, len, "%%%02X", c);
			p += 3;
			len -= 3;
		}
	}
	*p = '\0';

	return out;
}


#ifdef XML_TEST_MAIN

#include <stdio.h>

int
main(int argc, char **argv)
{
	char buf[50];

	printf("Should be NULL: %p\n", xml_escape_attr("abcdeabcde", buf, 10));
	printf("Should be string: %s\n", xml_escape_attr("abcdeabcd", buf, 10));

	printf("Quote: %s\n", xml_escape_attr("\"", buf, sizeof(buf)));
	printf("Quote overflow: %s\n", xml_escape_attr("aaa\"", buf, sizeof(buf)));
	printf("Quote: %s\n", xml_escape_attr("a\"a", buf, sizeof(buf)));
	char	buf2[50];
	printf("Double-esc: %s\n", xml_escape_uri( xml_escape_uri("collection:sharepoint_udc", buf, sizeof(buf)), buf2, sizeof(buf2)));
	printf("√√∏√•: %s\n", xml_escape_uri("√¶√∏√• - √√√", buf, sizeof(buf)));

}
#endif
