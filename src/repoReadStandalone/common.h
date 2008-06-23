#ifndef _COMMON_H_
#define _COMMON_H_

typedef struct {
	unsigned int DocID;
	char url[200];
	char content_type[4];
	unsigned int IPAddress;
	unsigned short response;
	unsigned short htmlSize;
	unsigned short imageSize;
	unsigned int time;
	unsigned short userID;
	double clientVersion;
} RepositoryHeader_t;


#endif /* _COMMON_H_ */
