#ifndef _LICENSE_H_
#define _LICENSE_H_

#ifdef WITH_MAKE_LICENSE
char *make_license(unsigned short int users);
#endif
int get_licenseinfo(char *s, unsigned int *_serial, unsigned short int *_users);
char * human_readable_key(char *k);

#endif /* _LICENSE_H_ */
