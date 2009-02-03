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

static size_t
cache_indexex_walk(char *path, size_t len, size_t *cached)
{
	DIR *dirp;
	size_t len2;
	struct dirent *ent;

	dirp = opendir(path);
	if (dirp == NULL) {
		//warn("opendir(%s)", path);
		return 0;
	}

	while ((ent = readdir(dirp))) {
		size_t len2;
		char *p;
		struct stat st;

		if (ent->d_name[0] == '.')
			continue;
		len2 = sprintf(path+len, "%s", ent->d_name);
		if (stat(path, &st) == -1) {
			warn("stat(%s)", path);
			continue;
		}
		if (S_ISREG(st.st_mode) && (p = strrchr(ent->d_name, '.')) && strcmp(p, ".txt") == 0) {
			int fd;
			indexcache_t *ic;
			void *ptr;
			char *p;

			if (cached[0] + st.st_size > MAX_INDEX_CACHE && 0)
				continue;
			printf("Found index: %s\n", path);
			fd = open(path, O_RDONLY);
			if (fd == -1) {
				warn("open(%s)", path);
				continue;
			}
#if 1
			if (st.st_size == 0) {
				ic = malloc(sizeof(*ic));
				ic->size = 0;
				ic->ptr = NULL;

				p = malloc(len+len2+1);
				memcpy(p, path, len+len2);
				p[len+len2] = '\0';
				hashtable_insert(indexcachehash, strdup(path), ic);

				close(fd);
				continue;
			}
#endif
			if ((ptr = mmap(0, st.st_size, PROT_READ, MAP_SHARED|MAP_LOCKED, fd, 0)) == MAP_FAILED) {
				warn("mmap(indexcache)");
				close(fd);
				continue;
			}
			ic = malloc(sizeof(*ic));
			ic->size = st.st_size;
			ic->ptr = ptr;
			
			close(fd);
			
			cached[0] += st.st_size;
			cached[1]++;
			p = malloc(len+len2+1);
			memcpy(p, path, len+len2);
			p[len+len2] = '\0';
			hashtable_insert(indexcachehash, strdup(path), ic);
		} else if (S_ISDIR(st.st_mode)) {
			len2++;
			(path+len+len2)[0] = '/';
			(path+len+len2)[1] = '\0';
			len2 = sprintf(path+len, "%s/", ent->d_name);
			cache_indexex_walk(path, len+len2, cached);
		}
	}

	closedir(dirp);

	return 0;
}


void
cache_indexes_empty()
{
	size_t *cached;

	cached = indexcachescached;

#if 0
	int i;

	for (i = 0; i < cached[1]; i++) {
		munmap(indexcaches[i], indexcachessize[i]);
	}
	cached[0] = cached[1] = 0;
#endif

	hashtable_destroy(indexcachehash, 0);
}

void
cache_fresh_lot_collection(void)
{
	DIR *dirp;
	char path[2048];
	size_t len;
	int i;

	/* We only look at the first 5 lots */
	for (i = 1; i < 6; i++) {
		struct dirent *de;

		len = snprintf(path, sizeof(path), "%s/%d/%d", bfile("lot"), i, i);

		dirp = opendir(path);
		if (dirp == NULL)
			continue;
		
		while ((de = readdir(dirp))) {
			size_t len2;
			DIR *dirp2;
			struct dirent *de2;

			if (de->d_name[0] == '.')
				continue;

			len2 = sprintf(path+len, "/%s", de->d_name);

			dirp2 = opendir(path);
			if (dirp2 == NULL)
				continue;
			while ((de2 = readdir(dirp2))) {
				int fd;
				int dw;

				if (de2->d_name[0] == '.')
					continue;

				sprintf(path+len+len2, "/%s", de2->d_name);
				printf("Found file: %s\n", path);
				fd = open(path, O_RDONLY);
				if (fd == -1)
					continue;
				read(fd, &dw, sizeof(dw));
				close(fd);
			}
			closedir(dirp2);
		}

		closedir(dirp);
	}
}

void
cache_indexes(void)
{
	DIR *dirp;
	size_t len;
	int i;
	char path[2048];
	size_t *cached;

	cached = indexcachescached;
	indexcachehash = create_hashtable(1023, ht_stringhash, ht_stringcmp);

	cached[0] = cached[1] = 0;
	for (i = 0; i < 65; i++) {
		struct dirent *ent;

		len = sprintf(path, "%s/%d/iindex/", bfile("lot"), i);
		cache_indexex_walk(path, len, cached);
	}
}

static void *
cache_indexes_keepalive_thread(void *dummy)
{
	for (;;) {
		// Hush little baby
		pthread_mutex_lock(&index_cache_lock);
		pthread_cond_wait(&index_cache_cv, &index_cache_lock);

		// Refresh cache
#if defined DEBUG || 1
		printf("Refreshing index cache...\n");
#endif
		cache_indexes_empty();
		cache_indexes();
		// Preopen some other files
		preopen();
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

