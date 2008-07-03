#ifndef _HT_H_
#define _HT_H_

unsigned int ht_stringhash(void *ky);
int ht_stringcmp(void *k1, void *k2);
/* wchar_t */
unsigned int ht_wstringhash(void *ky);
int ht_wstringcmp(void *k1, void *k2);
/* int */
unsigned int ht_integerhash(void *ky);
int ht_integercmp(void *k1, void *k2);

	
#endif /* _HT_H_ */
