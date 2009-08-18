#ifndef _PREOPEN_H_
#define _PREOPEN_H_

#ifdef WITH_SPELLING
        #include "../newspelling/spelling.h"
#endif

void preopen(void);

void cache_indexes_hup(int sig);
//size_t cache_indexex_walk(char *path, size_t len, size_t *cached);
void cache_indexes_empty(void);
void cache_indexes(int);
//void *cache_indexes_keepalive_thread(void *dummy);
void cache_indexes_keepalive(void);

//extern void *indexcaches[];
//extern size_t indexcachessize[];
extern size_t indexcachescached[];

void *cache_index_get(char *path, size_t *size);
void cache_fresh_lot_collection(void);

#ifdef WITH_SPELLING
void cache_spelling_keepalive(spelling_t *spelling);
static void *cache_spelling_keepalive_thread(void *dummy);
void cache_spelling_hup(int sig);
#endif

#endif /* _PREOPEN_H_ */
