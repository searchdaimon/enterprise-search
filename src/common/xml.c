#include <sys/types.h>

#include <string.h>

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

#ifdef XML_TEST_MAIN

#include <stdio.h>

int
main(int argc, char **argv)
{
	char buf[10];

	printf("Should be NULL: %p\n", xml_escape_attr("abcdeabcde", buf, sizeof(buf)));
	printf("Should be string: %s\n", xml_escape_attr("abcdeabcd", buf, sizeof(buf)));

	printf("Quote: %s\n", xml_escape_attr("\"", buf, sizeof(buf)));
	printf("Quote overflow: %s\n", xml_escape_attr("aaa\"", buf, sizeof(buf)));
	printf("Quote : %s\n", xml_escape_attr("a\"a", buf, sizeof(buf)));

	
}
#endif
