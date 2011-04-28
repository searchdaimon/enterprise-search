#ifndef _LIBCACHE_H_
#define _LIBCACHE_H_

#include <pthread.h>

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

typedef struct {
	pthread_t c_gcthread;
	pthread_mutex_t c_lock;
	struct hashtable *c_data;
	void (*c_freevalue)(void *value);
	unsigned int c_timeout;
} cache_t;

void cache_settimeout(cache_t *c, unsigned int timeout);
int cache_init(cache_t *c, void (*freevalue)(void *value), unsigned int timeout);
int cache_add(cache_t *c, char *prefix, char *key, void *value);
void *cache_fetch(cache_t *c, char *prefix, char *key);
void *cache_free(cache_t *c);



#endif /* _LIBCACHE_H_ */
