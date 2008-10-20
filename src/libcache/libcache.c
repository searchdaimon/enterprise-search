#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "libcache.h"

#include "../common/ht.h"

#define GC_SLEEP_TIME 360
#define GC_MIN_PURGE_TIME 180

typedef struct {
	void *p;
	time_t atime, mtime;
} cache_value_t;

int
cache_init(cache_t *c, void (*freevalue)(void *value), unsigned int timeout)
{
	memset(c, '\0', sizeof(*c));
	c->c_data = create_hashtable(7, ht_stringhash, ht_stringcmp);
	if (c->c_data == NULL)
		return 0;
	pthread_mutex_init(&c->c_lock, NULL);
	c->c_freevalue = freevalue;
	c->c_timeout = timeout;
	
	return 1;
}

void cache_settimeout(cache_t *c, unsigned int timeout) {
	c->c_timeout = timeout;
}

#define KEY_SEPARATOR 0x10

static char *
gen_key(char *p, char *k)
{
	char *key;
	size_t i, plen;

	if (p != NULL)
		plen = strlen(p);
	else
		plen = 0;
	key = malloc((p == NULL ? 0 : (plen+1)) + strlen(k) + 1);
	if (key == NULL)
		return NULL;
	if (p != NULL) {
		strcpy(key, p);
		i = plen;
		key[i] = KEY_SEPARATOR;
		i++;
	} else {
		i = 0;
	}
	strcpy(key+i, k);

	return key;
}

int
cache_add(cache_t *c, char *prefix, char *key, void *value)
{
	char *k;
	time_t now = time(NULL);
	cache_value_t *v;

	/* Do we already have this cached? */
	k = gen_key(prefix, key);
	if (k == NULL)
		return 0;
	pthread_mutex_lock(&c->c_lock);
	if ((v = hashtable_search(c->c_data, k)) != NULL) {
		v->mtime = now;
		v->atime = now;
		pthread_mutex_unlock(&c->c_lock);
		free(k);
		return 0;
	}
	v = malloc(sizeof(*v));
	if (v == NULL) {
		pthread_mutex_unlock(&c->c_lock);
		free(k);
		return 0;
	}
	v->atime = now;
	v->mtime = now;
	v->p = value;

	hashtable_insert(c->c_data, k, v);
	pthread_mutex_unlock(&c->c_lock);

	return 1;
}

void *
cache_fetch(cache_t *c, char *prefix, char *key)
{

	if (c->c_timeout == 0) {
		return NULL;
	}

	char *k = gen_key(prefix, key);
	cache_value_t *v;
	time_t now;


	pthread_mutex_lock(&c->c_lock);
	v = hashtable_search(c->c_data, k);
	if (v == NULL) {
		goto nomatch;
	}
	now = time(NULL);
	if (now - v->mtime > c->c_timeout) {
		v = hashtable_remove(c->c_data, k);
		c->c_freevalue(v->p);
		free(v);
		goto nomatch;
	}
	v->atime = time(NULL);
	pthread_mutex_unlock(&c->c_lock);
	free(k);
	return v->p;
 nomatch:
	pthread_mutex_unlock(&c->c_lock);
	free(k);
	return NULL;
}
