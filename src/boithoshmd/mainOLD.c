#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#include "../common/poprank.h"

#define SHMSZ     27

main()
{
	char c;
	int shmid;
	key_t key;
	char *shm, *s;
	FILE *POPFILE;
	int popMemorySize;
	int Rank;
	unsigned int DocID;
	char *RankArray;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5678;

popopen ();
//popclose();

	popMemorySize = NrOfElementInPopIndex();

	printf("mem size %i\n",popMemorySize);
    /*
     * Create the shared memory segment.
     */
	//ikke glem å regne ut størelsen hvis man ikke bruker char som enhet her
	if ((shmid = shmget(key, popMemorySize, IPC_CREAT | 0666)) < 0) {
        	perror("shmget");
        	exit(1);
    	}

    /*
     * Now we attach the segment to our data space.
     */
  	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        	perror("shmat");
        	exit(1);
    	}


	while(popGetNext(&Rank,&DocID)) {

		printf("DocID %i, Rank %u\n",DocID,Rank);
	
		RankArray[DocID] = log(Rank);		

	}

	exit(1);




    /*
     * Now put some things into the memory for the
     * other process to read.
     */
    s = shm;

    for (c = 'a'; c <= 'z'; c++)
        *s++ = c;
    *s = '\0';
    //*s = NULL;

    /*
     * Finally, we wait until the other process 
     * changes the first character of our memory
     * to '*', indicating that it has read what 
     * we put there.
     */
    while (*shm != '*')
        sleep(1);

    exit(0);
}


