#ifndef _PREOPEN_H_
#define _PREOPEN_H_

void preopen(void);

void cache_indexes_hup(int sig);
//size_t cache_indexex_walk(char *path, size_t len, size_t *cached);
void cache_indexes_empty(void);
void cache_indexes(void);
//void *cache_indexes_keepalive_thread(void *dummy);
void cache_indexes_keepalive(void);

//extern void *indexcaches[];
//extern size_t indexcachessize[];
extern size_t indexcachescached[];

void *cache_index_get(char *path);
void cache_fresh_lot_collection(void);

#endif /* _PREOPEN_H_ */
