#ifndef _HEADER_BBDOCDEFINE_
#define _HEADER_BBDOCDEFINE_

struct fileFilterFormat {
	char documentstype[12];
	char command[512];
	char outputtype[12];
	char outputformat[12];
	char comment[64];
	char format[12];
	char **attrwhitelist;
	int filtertype;
	char path[PATH_MAX];
};

#endif
