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

#include "../3pLibs/keyValueHash/hashtable.h"

static struct hashtable *global_mountet_h = NULL;

int crawlNFS_myUmount (char localfs[]) {
	
	int pid,n,forkstatus;
        char *testaray[] = { "mount", localfs, NULL };

        //for aa kunne kalle et program og fortsat ha samme id kan vi ikke bruke systm(), men må bruke execv
        if ((pid = fork()) == 0) {
                //child process

                execv("/home/boitho/boithoTools/bin/umount",testaray);

        }
        else {
                printf("child has pid %i\n",pid);

                //venter på barnet skal gjøre seg ferdig
                n = wait(&forkstatus);
                printf("status %i, %i\n",forkstatus,n);

        }

}

int crawlNFS_mymount (char nfsfs[], char localfs[]) {

	int pid,n,forkstatus;
        char *testaray[] = { "mount", "-r",nfsfs, localfs, NULL };

	printf("mount %s %s\n",nfsfs,localfs);

        //for aa kunne kalle et program og fortsat ha samme id kan vi ikke bruke systm(), men må bruke execv
        if ((pid = fork()) == 0) {
                //child process

                execv("/home/boitho/boithoTools/bin/mount",testaray);

        }
        else {
                printf("child has pid %i\n",pid);

                //venter på barnet skal gjøre seg ferdig
                n = wait(&forkstatus);
                printf("status %i, %i\n",forkstatus,n);

        }

}

//hadde alg.dat l�reren min sette denne hash funksjonen hadde han blit leiseg
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
	//finner ut mappen vi vil ha.
	sprintf(localfsdir,"%s/mounted_XXXXXX",BOITHO_VAR_DIR);

	if ((localfs = mktemp(localfsdir)) == NULL) {
		perror("mktemp");
		return NULL;
	}



	//oppretter den hvis vi ikke har
	if (mkdir(localfs,0755) != 0) {
		perror("mkdir");
		return NULL;
	}

	//ask mont to mont
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


	//kj�rer gjenom alle filsystemer vi eventeult har mountet og unmunter og fjerner mappen
	if ((DIRH = opendir(BOITHO_VAR_DIR)) == NULL) {
                perror(BOITHO_VAR_DIR);
                return 0;
        }

        while ((dp = readdir(DIRH)) != NULL) {


                if (dp->d_name[0] == '.') {
                        //printf(". domain (\"%s\")\n",dp->d_name);
                }
                else if (dp->d_type == DT_DIR) {

                	sprintf(nextdirname,"%s/%s/",BOITHO_VAR_DIR,dp->d_name);

			printf("cleaning mountet fs %s\n",nextdirname);

			//forhindrer dobeltmount
			crawlNFS_myUmount(nextdirname);
			
			if (rmdir(nextdirname) != 0) {
				perror("unlink");
			}
		}
	}

	return 1;
}

int crawlcanconect(struct collectionFormat *collection) {
	//tester om vi kan koble til
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
	//opdate
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


