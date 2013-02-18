#ifndef _LIBCACHE_H_
#define _LIBCACHE_H_

#include <pthread.h>
#ifdef LIBCACHE_SHARE
	#include "../3pLibs/keyValueHash/hashtable.h"
	#include "../3pLibs/keyValueHash/hashtable_itr.h"
#else
	#include <db.h>
#endif


typedef struct {
	pthread_t c_gcthread;
	pthread_mutex_t c_lock;
	#ifdef LIBCACHE_SHARE
	struct hashtable *c_data;
	#else
	DB *c_data;
	#endif
	void (*c_freevalue)(void *value);
} cache_t;

int cache_init(cache_t *c, void (*freevalue)(void *value));
#ifdef LIBCACHE_SHARE
int cache_add(cache_t *c, char *prefix, char *key, void *value);
#else
int cache_add(cache_t *c, char *prefix, char *key, void *value, int size);
void cache_delfiles();
#endif
void *cache_fetch(cache_t *c, char *prefix, char *key, int timeout);
void *cache_free(cache_t *c);



#endif /* _LIBCACHE_H_ */
