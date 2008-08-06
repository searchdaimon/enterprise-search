#ifndef _BASE32_H_
#define _BASE32_H_

int base32_encode(char *in, char **out, size_t len);
int base32_decode(char *in, char **out, size_t len);

#endif /* _BASE32_H_ */
