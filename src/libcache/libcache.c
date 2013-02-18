#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "libcache.h"
#include "../common/ht.h"
#include "../common/boithohome.h"
#include "../common/bfileutil.h"

#define _temp_path "var/cm_cache"

#define GC_SLEEP_TIME 360
#define GC_MIN_PURGE_TIME 180

typedef struct {
	time_t mtime;
	int size;
	#ifdef LIBCACHE_SHARE
		time_t atime;
		void *p;
	#else
		char p[];
	#endif
} cache_value_t;



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

#ifdef LIBCACHE_SHARE


int
cache_init(cache_t *c, void (*freevalue)(void *value))
{
	memset(c, '\0', sizeof(*c));
	c->c_data = create_hashtable(7, ht_stringhash, ht_stringcmp);
	
	if (c->c_data == NULL)
		return 0;
	pthread_mutex_init(&c->c_lock, NULL);
	c->c_freevalue = freevalue;

	return 1;
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
cache_fetch(cache_t *c, char *prefix, char *key, int timeout)
{



	char *k = gen_key(prefix, key);
	cache_value_t *v;
	time_t now;

	pthread_mutex_lock(&c->c_lock);

	v = hashtable_search(c->c_data, k);
	if (v == NULL) {
		goto nomatch;
	}

	now = time(NULL);
	if (now - v->mtime > timeout) {
		v = hashtable_remove(c->c_data, k);
		c->c_freevalue(v->p);
		free(v);
		printf("cache_fetch: removed timouted key %s\n",k);
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


void *
cache_free(cache_t *c)
{

    	cache_value_t *v;

    struct hashtable_itr *itr;
    if (hashtable_count(c->c_data) > 0)
    {
        itr = hashtable_iterator(c->c_data);
        do {
            v = hashtable_iterator_value(itr);

	    c->c_freevalue(v->p);
	    free(v);
        } while (hashtable_iterator_advance(itr));

     	free(itr);

    }

    hashtable_destroy(c->c_data,0); /* second arg indicates "free(value)" */

}

#else

void cache_delfiles() {

	// deleting db files
        DIR *dirp;
        struct dirent *dp;
	char path[PATH_MAX];

	bmkdir_p(bfile(_temp_path), 0755);

        if ((dirp = opendir(bfile(_temp_path))) == NULL) {
                printf("Cant open: %s\n", bfile(_temp_path) );
                exit(1);
        }

        while ((dp = readdir(dirp)) != NULL) {
                if (dp->d_name[0] == '.')
                        continue;
		snprintf(path,sizeof(path),"%s/%s",bfile(_temp_path),dp->d_name);
		printf("Unlinking %s\n",path);
		unlink(path);
        }

        closedir(dirp);

}

int
cache_init(cache_t *c, void (*freevalue)(void *value))
{
	memset(c, '\0', sizeof(*c));

	int ret;


	DB_ENV *dbenv;

  	ret = db_env_create(&dbenv, 0);
  	      dbenv->err(dbenv,ret,"err db_env_create ");

	dbenv->set_errfile(dbenv, stderr);
		dbenv->err(dbenv,ret,"err set_errfile ");

        if ((ret = dbenv->set_shm_key(dbenv, 664)) != 0) {
		dbenv->err(dbenv,ret,"err set_shm_key ");

		return 0;
        }


  	ret = dbenv->open(dbenv,bfile(_temp_path),DB_CREATE | DB_INIT_MPOOL | DB_INIT_CDB ,0);
  	     dbenv->err(dbenv,ret,"err db_env_open ");


        /* Create and initialize database object */
        if ((ret = db_create(&c->c_data, dbenv, 0)) != 0) {
                fprintf(stderr,
                     "%s: db_create: %s\n", "bbdocument", db_strerror(ret));
 		return 0;
        }


	#ifdef DEBUG
	dbenv->stat_print(dbenv, DB_STAT_ALL);
	#endif

        /* open the database. */
        if ((ret = c->c_data->open(c->c_data, NULL, "libcachedb", NULL, DB_BTREE, DB_CREATE, 0)) != 0) {
                        c->c_data->err(c->c_data, ret, "db open");
                        //goto err1;
                        //dette skjer nor collection mappen ikke er opprettet enda, typisk forde vi ikke har lagret et dokument der enda
                        #ifdef DEBUG
                        printf("can't dbp->open(), but db_create() was sucessful!\n");
                        #endif

                        return 0;
        }


	#ifdef DEBUG
	c->c_data->stat_print(c->c_data, DB_STAT_ALL);
	#endif

	pthread_mutex_init(&c->c_lock, NULL);
	c->c_freevalue = freevalue;

	return 1;
}


int
cache_add(cache_t *c, char *prefix, char *key, void *value, int size)
{
	char *k;
	int i;
	time_t now = time(NULL);
	cache_value_t *v;
printf("cache_add(key=\"%s\", size=%d)\n",key,size);
	/* Do we already have this cached? */
	k = gen_key(prefix, key);
	if (k == NULL)
		return 0;

	pthread_mutex_lock(&c->c_lock);

	int ret;
	DBT dbkey, dbdata;
        //resetter minne
        memset(&dbkey, 0, sizeof(DBT));
        memset(&dbdata, 0, sizeof(DBT));

        //legger inn nøkkelen i bdb strukturen
        dbkey.data = k;
        dbkey.size = strlen(k);


        dbdata.size = sizeof(cache_value_t) + size;
	v = calloc(1,dbdata.size);
	if (v == NULL) {
		pthread_mutex_unlock(&c->c_lock);
		free(k);
		return 0;
	}
	
	v->mtime = now;
	v->size  = size;
	memcpy(v->p,value,size);

	printf("######################################\n");
	printf("cache_add: ");
	for (i=0;i<v->size;i++) {
		//printf("%x",v->p[i]);
		printf("%c",v->p[i]);
	}
	printf("\n######################################\n");

        dbdata.data = v;


        //legger til i databasen
        if  ((ret = c->c_data->put(c->c_data, NULL, &dbkey, &dbdata, 0)) != 0) {
                c->c_data->err(c->c_data, ret, "DB->put");
        }
//        if  ((ret = c->c_data->sync(c->c_data, 0)) != 0) {
//                c->c_data->err(c->c_data, ret, "DB->sync");
//        }	

	printf("Added key %s (size %i) to db\n",dbkey.data, dbdata.size);

	pthread_mutex_unlock(&c->c_lock);

	return 1;
}

void *
cache_fetch(cache_t *c, char *prefix, char *key, int timeout)
{



	char *k = gen_key(prefix, key);
	cache_value_t *v;
	time_t now;
	int i;

	pthread_mutex_lock(&c->c_lock);

	int ret;
	DBT dbkey, dbdata;
        //resetter minne
        memset(&dbkey, 0, sizeof(DBT));
        memset(&dbdata, 0, sizeof(DBT));

        //legger inn key i bdb strukturen
        dbkey.data = k;
        dbkey.size = strlen(k);

	if ((ret = c->c_data->get(c->c_data, NULL, &dbkey, &dbdata, 0)) != 0) {
		printf("Diden't find key %s i db\n",dbkey.data);
		goto nomatch;
	}
	v = ((cache_value_t *)dbdata.data);

	now = time(NULL);
	if (now - v->mtime > timeout && timeout!=0) {
	        c->c_data->del(c->c_data, NULL, &dbkey, 0);
//		c->c_freevalue(v->p);
//		free(v);
		printf("cache_fetch: removed timouted key %s. No match\n",k);
		goto nomatch;
	}

	pthread_mutex_unlock(&c->c_lock);
	free(k);

	printf("######################################\n");
	printf("cache_fetch: ");
	for (i=0;i<v->size;i++) {
		//printf("%x",v->p[i]);
		printf("%c",v->p[i]);
	}
	printf("\n######################################\n");

	return v->p;
 nomatch:
	pthread_mutex_unlock(&c->c_lock);
	free(k);
	return NULL;
}


void *
cache_free(cache_t *c)
{

    	cache_value_t *v;

}


#endif
