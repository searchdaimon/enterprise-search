#include <stdio.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/sem.h>

int main() {

key_t key; /* key to pass to semget() */ 
int semflg; /* semflg to pass tosemget() */ 
int nsems; /* nsems to pass to semget() */ 
int semid; /* return value from semget() */ 

key = 1234;
nsems = 1;
semflg = IPC_CREAT | 0666; /* semflg to pass to semget() */
struct sembuf *sops; /* ptr to operations to perform */ 

if ((semid = semget(key, nsems, semflg)) == -1) {
		perror("semget: semget failed"); 
		exit(1); 
} 

if ((semop(semid, sops, 1,) == -1) {
	perror("semop");
}


}
