#ifndef _ATTRIBUTES_H_
#define _ATTRIBUTES_H_

/**
 *	Input: Kommaseparert liste med attributtpar.
 *	k <- key
 *	v <- value
 *	kv <- key=value
 */
int next_attribute(char *attributes, char **offset, char *k, char *v, char *kv);
int next_attribute_key(char *attributes, char **offset, char *k, char *v, char *kv, char *want);

//int next_attribute_kv(char *attributes, char **offset, char *kv);

#endif /* _ATTRIBUTES_H_ */
