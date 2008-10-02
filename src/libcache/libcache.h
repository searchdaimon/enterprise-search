#ifndef _LIBCACHE_H_
#define _LIBCACHE_H_

#include <pthread.h>

#include "../3pLibs/keyValueHash/hashtable.h"

typedef struct {
	pthread_t c_gcthread;
	pthread_mutex_t c_lock;
	struct hashtable *c_data;
	void (*c_freevalue)(void *value);
	unsigned int c_timeout;
} cache_t;

int cache_init(cache_t *c, void (*freevalue)(void *value), unsigned int timeout);
int cache_add(cache_t *c, char *prefix, char *key, void *value);
void *cache_fetch(cache_t *c, char *prefix, char *key);




#endif /* _LIBCACHE_H_ */
