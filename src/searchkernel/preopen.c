/*

	runarb: 15 ja. Dette var en test av om å preåpne re filer ville øke ytelsen. Det ble ingen forskjell.
*/

#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include <pthread.h>

#include "../common/lot.h"
#include "../common/re.h"
#include "../common/boithohome.h"
#include "../common/ht.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

void
preopen(void)
{
	int i;
        DIR *dirh;
	
	if ((dirh = listAllColl_start()) == NULL)
                err(1, "listAllColl_start()");

        char * subname;
        while ((subname = listAllColl_next(dirh)) != NULL) {
                printf("subname: %s\n", subname);
		for(i=1;i<12;i++) {
			reopen_cache(i,4, "filtypes",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE);
			reopen_cache(i,sizeof(int), "dates",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE);
			reopen_cache(i,sizeof(unsigned int), "crc32map",subname,RE_READ_ONLY|RE_POPULATE);
		}	
	}
        listAllColl_close(dirh);
}

pthread_mutex_t index_cache_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t index_cache_cv = PTHREAD_COND_INITIALIZER;

void
cache_indexes_hup(int sig)
{
	pthread_mutex_lock(&index_cache_lock);
	pthread_cond_signal(&index_cache_cv);
	pthread_mutex_unlock(&index_cache_lock);
}

#define MAX_INDEX_CACHE (1024*1024*256)

typedef struct {
	void *ptr;
	size_t size;
} indexcache_t;

struct hashtable *indexcachehash;
size_t indexcachescached[2];

void *
cache_index_get(char *path)
{
	indexcache_t *ic;

	if (indexcachehash == NULL)
		return NULL;
	
	ic = hashtable_search(indexcachehash, path);

	if (ic == NULL)
		return NULL;
	if (ic->ptr == NULL)
		return MAP_FAILED;

	return ic->ptr;
}


void
cache_indexes_empty(void)
{
	size_t *cached;
	struct hashtable_itr *itr;

	cached = indexcachescached;
	itr = hashtable_iterator(indexcachehash);
	if (hashtable_count(indexcachehash) > 0) {
		do {
			indexcache_t *ic;

			ic = hashtable_iterator_value(itr);
			if (ic == NULL)
				continue;
			munmap(ic->ptr, ic->size);
		} while (hashtable_iterator_advance(itr));
	}

	hashtable_destroy(indexcachehash, 1);
}

void
cache_fresh_lot_collection(void)
{
	DIR *colls;
	char *coll;
	char path[2048];
	size_t len;
	int i;

	/* We only look at the first 5 lots */
	colls = listAllColl_start();
	while ((coll = listAllColl_next(colls))) {
		for (i = 1; i < 6; i++) {
			DIR *dirp;
			struct dirent *de;

			GetFilPathForLot(path, i, coll);
			len = strlen(path);

			dirp = opendir(path);
			if (dirp == NULL)
				continue;
			while ((de = readdir(dirp))) {
				int fd;
				int dw;

				if (de->d_name[0] == '.')
					continue;

				sprintf(path+len, "/%s", de->d_name);
				printf("Found file: %s\n", path);
				fd = open(path, O_RDONLY);
				if (fd == -1)
					continue;
				read(fd, &dw, sizeof(dw));
				close(fd);
			}
			closedir(dirp);
		}
	}
}

static size_t
cache_indexes_handle(char *path, size_t *cached)
{
	int fd;
	indexcache_t *ic;
	void *ptr;
	char *p;
	size_t len;
	struct stat st;

	if (stat(path, &st) == -1)
		return 0;

	if (cached[0] + st.st_size > MAX_INDEX_CACHE && 0)
		return 0;
	printf("Found index: %s\n", path);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		warn("open(%s)", path);
		return 0;
	}
	if (st.st_size == 0) {
#if 1
		/* XXX: Not sure about this, will empty iindexes ever be opnened during search? */
		ic = malloc(sizeof(*ic));
		ic->size = 0;
		ic->ptr = NULL;

		len = strlen(path);
		p = malloc(len+1);
		memcpy(p, path, len);
		p[len] = '\0';
		hashtable_insert(indexcachehash, p, ic);
#endif
		close(fd);
		return 0;
	}
	if ((ptr = mmap(0, st.st_size, PROT_READ, MAP_SHARED|MAP_LOCKED, fd, 0)) == MAP_FAILED) {
		warn("mmap(indexcache)");
		close(fd);
		return 0;
	}
	ic = malloc(sizeof(*ic));
	ic->size = st.st_size;
	ic->ptr = ptr;
	
	close(fd);
	
	cached[0] += st.st_size;
	cached[1]++;
	len = strlen(path);
	p = malloc(len+1);
	memcpy(p, path, len);
	p[len] = '\0';

	hashtable_insert(indexcachehash, p, ic);

	return 1;
}

void
cache_indexes(void)
{
	DIR *dirp;
	size_t len;
	int i;
	size_t *cached;
	DIR *colls;
	char *coll;

	cached = indexcachescached;
	indexcachehash = create_hashtable(1023, ht_stringhash, ht_stringcmp);

	cached[0] = cached[1] = 0;

	colls = listAllColl_start();
	while ((coll = listAllColl_next(colls))) {
		for (i = 0; i <= NrOfDataDirectorys; i++) {
			char *types[] = {
				"Main",
				"acl_allow",
				"acl_denied",
				"attributes",
				NULL,
			};
			char path[2048];
			char name[2048];
			int j;

			for (j = 0; types[j] != NULL; j++) {
				GetFilePathForIindex(path, name, i, types[j], "aa", coll);
				cache_indexes_handle(name, cached);
				GetFilePathForIDictionary(path, name, i, types[j], "aa", coll);
				cache_indexes_handle(name, cached);
			}
		}
	}
	listAllColl_close(colls);
}

static void *
cache_indexes_keepalive_thread(void *dummy)
{
	for (;;) {
		// Hush little baby
		pthread_mutex_lock(&index_cache_lock);
		pthread_cond_wait(&index_cache_cv, &index_cache_lock);

		// Refresh cache
		//printf("Refreshing index cache...\n");
		cache_indexes_empty();
		cache_indexes();
		// Preopen some other files
		preopen();
		// Lot directories
		cache_fresh_lot_collection();
		pthread_mutex_unlock(&index_cache_lock);

	}
}

void
cache_indexes_keepalive(void)
{
	pthread_t td;
	int rc;

	if ((rc = pthread_create(&td, NULL, cache_indexes_keepalive_thread, NULL)))
		err(1, "Unable to start cache index keepalive thread");
}

