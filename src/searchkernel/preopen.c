/*
	runarb: 15 ja. Dette var en test av om å preåpne re filer ville øke ytelsen. Det ble ingen forskjell.
*/

#include <sys/file.h>

#include <stdio.h>
#include <dirent.h>
#include <err.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "../logger/logger.h"
#include "../common/lot.h"
#include "../common/re.h"
#include "../common/boithohome.h"
#include "../common/ht.h"
#include "verbose.h"

#include "../3pLibs/keyValueHash/hashtable.h"
#include "../3pLibs/keyValueHash/hashtable_itr.h"

#ifdef WITH_SPELLING
        #include "../newspelling/spelling.h"
#endif

#define MAX_PREOPEM_FILE 300

// maks minne vi kan bruke på å cache i indekser
#define MAX_INDEX_CACHE (1024*1024*256)

void preopen(void) {
	int i;
        DIR *dirh;
	FILE *FH;
	int count = 0;

	reclose_cache();
	
	if ((dirh = listAllColl_start()) == NULL) {
		bblog(ERROR, "Can't listAllColl_start()");
		return;
	}
 
        char * subname;
	while (((subname = listAllColl_next(dirh)) != NULL) && (count < MAX_PREOPEM_FILE)) {
                bblog(DEBUGINFO, "subname: %s", subname);
		for(i=1;i<maxLots;i++) {
			// vi åpner kun lotter som har DocumentIndex. Dette er spesielt viktig da vi oppretter 
			// filene hvis de ikke finnes.
			if ((FH = lotOpenFileNoCasheByLotNr(i,"DocumentIndex","rb", 'r', subname)) == NULL) {
				continue;
			}
			reopen_cache(i,4, "filtypes",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE|RE_CREATE_AND_STRETCH);
			reopen_cache(i,sizeof(int), "dates",subname,RE_READ_ONLY|RE_STARTS_AT_0|RE_POPULATE|RE_CREATE_AND_STRETCH);
			reopen_cache(i,sizeof(unsigned int), "crc32map",subname,RE_READ_ONLY|RE_POPULATE|RE_CREATE_AND_STRETCH);

			fclose(FH);

			if (count > MAX_PREOPEM_FILE) {
				break;
			}
			// +3 da vi øker med filtypes, dates, og crc32map
			count += 3;
		}	
	}
        listAllColl_close(dirh);


	if (count >= MAX_PREOPEM_FILE) {
		bblog(WARN, "can't preopen any more. Did hit MAX_PREOPEM limit of %d files", MAX_PREOPEM_FILE);
	}
}

pthread_mutex_t index_cache_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t index_cache_cv = PTHREAD_COND_INITIALIZER;

#ifdef WITH_SPELLING
	pthread_mutex_t spelling_cache_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t spelling_cache_cv = PTHREAD_COND_INITIALIZER;
#endif

void cache_indexes_hup(int sig) {
	/*
	 * If it is locked we are either in the signal handler somewhere else,
	 * or the indexer cache in running.
	 */
	if (pthread_mutex_trylock(&index_cache_lock) != 0)
		return;
	pthread_cond_signal(&index_cache_cv);
	pthread_mutex_unlock(&index_cache_lock);
}

#ifdef WITH_SPELLING
void cache_spelling_hup(int sig) {
	/*
	 * If it is locked we are either in the signal handler somewhere else,
	 * or the indexer cache in running.
	 */
	if (pthread_mutex_trylock(&spelling_cache_lock) != 0)
		return;
	pthread_cond_signal(&spelling_cache_cv);
	pthread_mutex_unlock(&spelling_cache_lock);
}
#endif

#define MAX_INDEX_CACHE (1024*1024*256)

typedef struct {
	void *ptr;
	size_t size;
} indexcache_t;

struct hashtable *indexcachehash;
size_t indexcachescached[2];

void * cache_index_get(char *path, size_t *size) {
	indexcache_t *ic;

	if (indexcachehash == NULL)
		return NULL;
	
	ic = hashtable_search(indexcachehash, path);

	if (ic == NULL)
		return NULL;
	if (ic->ptr == NULL)
		return MAP_FAILED;
	if (size != NULL)
		*size = ic->size;

	return ic->ptr;
}


void cache_indexes_empty(void) {
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

void cache_fresh_lot_collection(void) {
	DIR *colls;
	char *coll;
	char path[2048];
	size_t len;
	int i;

	/* We only look at the first 5 lots */
        if ((colls = listAllColl_start()) == NULL) {
		bblog(ERROR, "Can't listAllColl_start()");
                return;
	}

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
				bblog(DEBUGINFO, "Found file: %s", path);
				fd = open(path, O_RDONLY);
				if (fd == -1)
					continue;
				read(fd, &dw, sizeof(dw));
				close(fd);
			}
			closedir(dirp);
		}
	}
	listAllColl_close(colls);
}

static size_t cache_indexes_handle(char *path, size_t *cached) {
	int fd;
	indexcache_t *ic, *icold;
	void *ptr;
	char *p;
	size_t len;
	struct stat st;

	if (stat(path, &st) == -1)
		return 0;

	if (cached[0] + st.st_size > MAX_INDEX_CACHE)
		return 0;
	bblog(DEBUGINFO, "Found index: %s", path);
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		bblog_errno(ERROR, "open(%s)", path);
		return 0;
	}
	if (st.st_size == 0) {
#if 0
		/* XXX: Not sure about this, will empty iindexes ever be opened during search? */
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
		bblog_errno(INFO, "mmap(indexcache)");
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

	icold = hashtable_search(indexcachehash, p);
	if (icold != NULL) {
		munmap(icold->ptr, icold->size);
		icold->ptr = ic->ptr;
		icold->size = ic->size;
		free(ic);
		free(p);
	} else {
		hashtable_insert(indexcachehash, p, ic);
	}

	return 1;
}

void cache_indexes_collection(char *coll) {
	int i;
	size_t *cached;

	cached = indexcachescached;
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

void cache_indexes_all(void) {
	size_t *cached;
	DIR *colls;
	char *coll;
	FILE *fp;
	char collpath[2048];

	lot_get_closed_collections_file(collpath);
	if ((fp = fopen(collpath, "r+")) == NULL) {
		bblog_errno(WARN, "Unable to open collection list: fopen(%s)", collpath);
	} else {
		flock(fileno(fp), LOCK_EX);
		ftruncate(fileno(fp), 0);
		flock(fileno(fp), LOCK_UN);
		/*
		 * Release the lock, so that indexes updated while running the
		 * cache step will not block. We can recache it later.
		 */ 
		fclose(fp);
	}

	cached = indexcachescached;
	indexcachehash = create_hashtable(1023, ht_stringhash, ht_stringcmp);

	cached[0] = cached[1] = 0;

        if ((colls = listAllColl_start()) == NULL) {
		bblog(ERROR, "Can't listAllColl_start()");
                return;
	}

	while ((coll = listAllColl_next(colls))) {
		cache_indexes_collection(coll);
	}

	listAllColl_close(colls);
}

void cache_indexes(int action) {
	if (action == 0) { /* All collections */
		cache_indexes_all();
	} else if (action == 1) {
		if (indexcachehash == NULL) {
			bblog(WARN, "Unable to run an incremental index cache when there has not been done a full one");
		} else {
			FILE *fp;
			char collpath[2048];

			lot_get_closed_collections_file(collpath);
			if ((fp = fopen(collpath, "r+")) == NULL) {
				bblog_errno(ERROR, "Unable to open collection list: fopen(%s)", collpath);
			} else {
				char line[2048];

				flock(fileno(fp), LOCK_EX);

				while (fgets(line, sizeof(line), fp) != NULL) {
					line[strlen(line)-1] = '\0'; /* Remove trailing newline */
					bblog(INFO, "Got updated collection: %s", line);
					cache_indexes_collection(line);
				}
				
				ftruncate(fileno(fp), 0);
				flock(fileno(fp), LOCK_UN);
				fclose(fp);
				bblog(INFO, "done");
			}
		}
	} else {
		bblog(WARN, "Unknown cache index action: %d", action);
	}
}

static void * cache_indexes_keepalive_thread(void *dummy) {
	for (;;) {
		// Hush little baby
		pthread_mutex_lock(&index_cache_lock);
		pthread_cond_wait(&index_cache_cv, &index_cache_lock);

		// Refresh cache
		bblog(INFO, "Refreshing index cache...");
		/* We do incremental caching now */
		cache_indexes(1);
		// Preopen some other files
		preopen();
		// Lot directories
		pthread_mutex_unlock(&index_cache_lock);

	}
}

#ifdef WITH_SPELLING
static void * cache_spelling_keepalive_thread(spelling_t **spelling) {


	for (;;) {
		// Hush little baby
		pthread_mutex_lock(&spelling_cache_lock);
		pthread_cond_wait(&spelling_cache_cv, &spelling_cache_lock);

		// Refresh cache
		bblog(INFO, "Refreshing spelling cache...");
		// do it

		if (*spelling != NULL) {
        		untrain(*spelling);
		}

        	if ((*spelling = train(bfile("var/dictionarywords"))) == NULL) {
        	        bblog(WARN, "Can't init spelling.");
	        }


		pthread_mutex_unlock(&spelling_cache_lock);

		bblog(INFO, "~Refreshing spelling cache");
	}
}
#endif

void cache_indexes_keepalive(void) {
	pthread_t td;
	int rc;

	if ((rc = pthread_create(&td, NULL, cache_indexes_keepalive_thread, NULL))) {
		bblog_errno(ERROR, "Unable to start cache index keepalive thread");
		exit(1);
	}
}

#ifdef WITH_SPELLING
void cache_spelling_keepalive(spelling_t *spelling) {
	pthread_t td;
	int rc;

	if ((rc = pthread_create(&td, NULL, cache_spelling_keepalive_thread, spelling))) {
		bblog_errno(ERROR, "Unable to start spelling keepalive thread");
		exit(1);
	}
	
}
#endif
