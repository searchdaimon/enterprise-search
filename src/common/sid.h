#ifndef _SID_H_
#define _SID_H_

#define MAX_SID_LEN 1024

char *sid_b64totext(char *, size_t);
char *sid_btotext(char *);
int sid_replacelast(char *, char *);

#endif /* _SID_H_ */
