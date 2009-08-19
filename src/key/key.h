#ifndef _COMMON_KEY_H_
#define _COMMON_KEY_H_


#define KEY_LEN 32
#define KEY_STR_LEN (KEY_LEN+1)

int key_get_existingconn(void *db, char *keyout);
int key_get(char *keyout);
int key_equal(char *k1, char *k2);

#endif
