#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <limits.h>

#include "../crawl/crawl.h"
#include "../common/define.h"
#include "../crawl/crawlLocalFiles.h"
#include "../common/boithohome.h"

#include "../3pLibs/keyValueHash/hashtable.h"

static struct hashtable *global_mountet_h = NULL;

int crawlNFS_myUmount (char localfs[]) {
	
	int pid,n,forkstatus;
        char *testaray[] = { "mount", localfs, NULL };

        //to be able to call a program and still keep the same id we can't use system(), but must use execv
        if ((pid = fork()) == 0) {
                //child process

                execv("/home/boitho/boithoTools/bin/umount",testaray);

        }
        else {
                printf("child has pid %i\n",pid);

                //wait for the child to finish
                n = wait(&forkstatus);
                printf("status %i, %i\n",forkstatus,n);

        }

}

int crawlNFS_mymount (char nfsfs[], char localfs[]) {

	int pid,n,forkstatus;
        char *testaray[] = { "mount", "-r",nfsfs, localfs, NULL };

	printf("mount %s %s\n",nfsfs,localfs);

        //to be able to call a program and still keep the same id we can't use system(), but must use execv
        if ((pid = fork()) == 0) {
                //child process

                execv("/home/boitho/boithoTools/bin/mount",testaray);

        }
        else {
                printf("child has pid %i\n",pid);

                //wait for the child to finish
                n = wait(&forkstatus);
                printf("status %i, %i\n",forkstatus,n);

        }

}

//had my alg.dat teacher seen this hash function he would have been sad
static unsigned int crawlNFS_hash_from_key_fn(void *ky)
{
	int i;
	unsigned int key;

	char *k = (char *)ky;
	
	i =0;
	key = 0;
	while (k[i] != '\0') {
		key += k[i]; 
		++i;
	}


	return key;;
}

static int crawlNFS_keys_equal_fn(void *k1, void *k2)
{

	char *k1s = (char *)k1; 
	char *k2s = (char *)k2; 

    	return (0 == strcmp(k1s,k2s));
}

char *getfs(char nfsfs[]) {


	char *found, *v, *k;
	char *localfs;
	char localfsdir[PATH_MAX];

	k = strdup(nfsfs);

	if ( (found = hashtable_search(global_mountet_h,k)) != NULL) {
		printf("found!");
		return found;
	}
	//figure out the directory we want.
	sprintf(localfsdir,"%s/mounted_XXXXXX",bfile("var"));

	if ((localfs = mktemp(localfsdir)) == NULL) {
		perror("mktemp");
		return NULL;
	}



	//create it if we don't have it
	if (mkdir(localfs,0755) != 0) {
		perror("mkdir");
		return NULL;
	}

	//ask mount to mount
	crawlNFS_mymount(nfsfs,localfs);

	v = strdup(localfsdir);

	if (! hashtable_insert(global_mountet_h,k,v) ) {
		printf("can't hashtable_insert\n");
		return NULL;
	}

	return v;

}

int crawlinit() {

	char nextdirname[PATH_MAX];
	DIR *DIRH;
	struct dirent *dp;

	printf("crawlNFS crawlinit\n");


	global_mountet_h = create_hashtable(16, crawlNFS_hash_from_key_fn, crawlNFS_keys_equal_fn);
	if (global_mountet_h == NULL) {
        	printf("out of memory allocating mounted hashtable\n");
        	return 0;
    	}


	//go through all filesystems we may have mounted and unmount and remove the directory
	if ((DIRH = opendir(bfile("var"))) == NULL) {
                perror(bfile("var"));
                return 0;
        }

        while ((dp = readdir(DIRH)) != NULL) {


                if (dp->d_name[0] == '.') {
                        //printf(". domain (\"%s\")\n",dp->d_name);
                }
                else if (dp->d_type == DT_DIR) {

                	sprintf(nextdirname,"%s/%s/",bfile("var"),dp->d_name);

			printf("cleaning mountet fs %s\n",nextdirname);

			//prevent double mount
			crawlNFS_myUmount(nextdirname);
			
			if (rmdir(nextdirname) != 0) {
				perror("unlink");
			}
		}
	}

	return 1;
}

int crawlcanconect(struct collectionFormat *collection) {
	//test whether we can connect
	return 1;
}

int crawlfirst(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {

	struct crawldocumentExistFormat crawldocumentExist;
	struct crawldocumentAddFormat crawldocumentAdd;
	
	char *localfs;

	printf("crawlfirst: start\n");
	
	//mysql_recurse((*collection).host,(*collection).user,(*collection).password,(*collection).query1,collection,documentExist,documentAdd);

	localfs = getfs((*collection).resource);

	printf("fs: nfs \"%s\" -> \"%s\"\n",(*collection).resource,localfs);


	crawlLocalFiles(collection,documentExist,documentAdd,(*collection).resource,localfs);
	//crawlLocalFiles(collection,documentExist,documentAdd,(*collection).resource,"/tmp");

	return 1;
}


int crawlupdate(struct collectionFormat *collection,
	int (*documentExist)(struct collectionFormat *collection,struct crawldocumentExistFormat *crawldocumentExist), 
	int (*documentAdd)(struct collectionFormat *collection,struct crawldocumentAddFormat *crawldocumentAdd)) {
	//update
	printf("crawlupdate: start\n");
	
	
	return 1;
}


struct crawlLibInfoFormat crawlLibInfo = {
	crawlinit,
        crawlfirst,
	crawlfirst,
        crawlcanconect,
	NULL,
	NULL,
        crawl_security_acl,
        "NFS",
        strcrawlError
};


