#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <err.h>

#define linelen 512

/*
usage:

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/common/exeoc.h"

int main () {

        char exeocbuf[1024];
        int exeocbuflen;


        char *shargs[] = {"/bin/sh","-c","ls -1",'\0'};

        exeocbuflen = sizeof(exeocbuf);
        if (!exeoc(shargs,exeocbuf,&exeocbuflen)) {
                printf("can't run program\n");

                return 0;

        }

        printf("exeoc out:\n\"%s\"\n",exeocbuf);
}

*/

static void
setrlimits(void)
{
        struct rlimit rlim;

	// don't dump core
        rlim.rlim_cur = 0;
        rlim.rlim_max = 0;
        if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
                perror("setrlimit");
        }
        if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
                perror("getrlimit");
                return;
        }
        printf("%d %d\n", rlim.rlim_cur, rlim.rlim_max);
}



int
exeoc_stdselect(char *exeargv[],char documentfinishedbuf[],int *documentfinishedbufsize, pid_t *ret,int alsostderr, struct timeval *timeout)
{

	pid_t pid;
	int     pipefd[2];
	int i,n;
	pid_t waitstatus;

	if ((*documentfinishedbufsize) <= linelen) {
		fprintf(stderr,"Error: buffer must be larger then %i. Did get only %i\n",linelen,*documentfinishedbufsize);
	}	

	//printf("documentfinishedbufsize %i\n",(*documentfinishedbufsize));
	#ifdef DEBUG
		printf("exeoc_stdselect: runing program \"%s\"\n",exeargv[0]);
		i=0;
		while(exeargv[i] != '\0') {
			printf("arg \"%s\"\n",exeargv[i]);

			++i;
		}
	#endif

	pipe(pipefd);

	if ((pid = fork()) == 0) {
		#ifdef DEBUG
			printf("child\n");
		#endif

		/* Child process closes up input side of pipe */
                close(pipefd[0]);


		if (dup2(pipefd[1],fileno(stdout)) == -1) {
			perror("dup2");
		}

		//ogaå fange stderr
		if (alsostderr) {
			if (dup2(pipefd[1],fileno(stderr)) == -1) {
				perror("dup2");
			}
		}

		// lukker alle andre åpne filer
        	int         fi, fmax = getdtablesize();
	        for (fi=3; fi<fmax; fi++) close(fi);

		// skrur av core domping
		setrlimits();

		//sleep(1);
		//char *execvarg[] = {"/bin/cat","/tmp/test.txt", '\0'};
		//execv("/bin/cat",execvarg);
		if (execv(exeargv[0],exeargv) == -1) {
			fprintf(stderr,"Error: can't execv at %s:%d\n",__FILE__,__LINE__);
			perror(exeargv[0]);
			//exit(EXIT_FAILURE);
		}
		#ifdef DEBUG
			fprintf(stderr,"Eror: This can't happend. exexcv returned!\n");
		#endif

		/*******************************************
		BUGFIKS: runarb
		Et eller annet skjer hvis vi kaller execv med
		et program som ikke finnes. Vi får da segfeil i indeksereren
		kaller derfor /bin/false for å dø på en verdig måte
				
		*******************************************/		
		char *falesaar[] = {"/bin/false",'\0'}; 
		if (execv("/bin/false",falesaar) == -1) {

		}
		/*******************************************/
		exit(1);
		
	}
	else {
		int	read_error = 0;
		fd_set readfds;
		fd_set tmp;
		//printf("perent. Child pid %i\n",(int)pid);
		// Parent process closes up output side of pipe
                close(pipefd[1]);

		FD_ZERO(&readfds);
		FD_SET(pipefd[0], &readfds);

		i = 0;

		while (1) {
			int numfds; 

			tmp = readfds;

			numfds = select(pipefd[0]+1, &tmp, NULL, NULL, timeout);

			if (numfds < 0) {
				warn("error in select, exeoc");
				return 0;
			} else if (numfds == 0) {
				warn("timeout in select, exeoc. kiling pid %u", (unsigned int)pid);
				kill(pid, SIGKILL);
				return 0;
			}

			if (!FD_ISSET(pipefd[0], &tmp)) {
				warn("no wanted fd set in select, exeoc");
				continue;
			}

			if (((n = read(pipefd[0], &documentfinishedbuf[i], linelen)) >= 1) 
				&& (i + linelen < (*documentfinishedbufsize))) {
				i += n;
			} else {
				if (n == 0)
					break;
				warn("error in read, exeoc");
				read_error = 1;
				break;
			}

		}
		
		close(pipefd[0]);

		(*documentfinishedbufsize) = i;
		documentfinishedbuf[(*documentfinishedbufsize)] = '\0';

		#ifdef DEBUG
			printf("exeoc_stdselect: documentfinishedbufsize \"%i\" at %s:%d\n",(*documentfinishedbufsize),__FILE__,__LINE__);
		#endif

		if (i==0) {
			printf("exeoc_stdselect: Error: dident manage to read back any data\n");
			//return 0;
		}


		/*
		Ser ikke ut til å fungere :(
		//venter på prossesen
		if (waitpid(pid,&waitstatus,0) == -1) {
			printf("error: can't get exit status for %i pid at %s:%d\n",pid,__FILE__,__LINE__);
			perror("waitpid");
		}
		printf("waitpid finished. waitstatus %i\n",waitstatus);

		//hvis vi ikke fikk til å kjøre programet, skal vi returnere med EXIT_FAILURE, som blir waitstatus 256
		if (waitstatus == 256) {
			return 0;
		}
		*/

#ifdef DEBUG
		printf("waitng for pid \"%i\"\n",pid);
#endif
		//waitpid(pid,&waitstatus,0);
		if (waitpid(pid, &waitstatus, WUNTRACED) <= 0) {
			perror("waitpid()");
			*ret = 1;
		} else {
	//		#ifdef DEBUG
			if (WIFEXITED(waitstatus)==0)
			    {
				printf("Child did not exit normally.\n");
				if (WIFSIGNALED(waitstatus))
				    {
					printf("TERMSIG: %i (\"%s\")\n", WTERMSIG(waitstatus),strsignal(WTERMSIG(waitstatus)));
				    }

				if (WIFSTOPPED(waitstatus))
				    {
					printf("STOPSIG: %i\n", WSTOPSIG(waitstatus));
				    }
			    }
	//		#endif

			(*ret) = WEXITSTATUS(waitstatus);

			if (WIFSIGNALED(waitstatus) && (WTERMSIG(waitstatus) == SIGKILL)) {
				printf("TERMSIG: %i (\"%s\")\n", WTERMSIG(waitstatus),strsignal(WTERMSIG(waitstatus)));
				(*documentfinishedbufsize) = 0;
				documentfinishedbuf[0] = '\0';

				return 0;
			}
			else if (read_error) {
				printf("Error: Read error.\n");
				(*documentfinishedbufsize) = 0;
				documentfinishedbuf[0] = '\0';

				return 0;

			}
#ifdef DEBUG
			printf("waitpid finished. waitstatus %i\n",waitstatus);
			printf("dokument: \"%s\"\n",documentfinishedbuf);
#endif 

		}

		//printf("WIFEXITED(): %i\nWIFSIGNALED %i\nWIFSTOPPED %i\n\nread_error %i\n40 %i\n",WIFEXITED(waitstatus),WIFSIGNALED(waitstatus),WIFSTOPPED(waitstatus),read_error,SIGRTMIN + 7);


		//vi får ofte at vi fikk signal 40, men uten at noe var feil. Ignorerer det for nå.
		//bdw: hardkoding av signalnavnet er ikke lurt.		

		return 1;
	}

}

int exeoc_stdall(char *exeargv[],char documentfinishedbuf[],int *documentfinishedbufsize, pid_t *ret) {
	return exeoc_stdselect(exeargv,documentfinishedbuf,documentfinishedbufsize,ret,1, NULL);
}
int exeoc(char *exeargv[],char documentfinishedbuf[],int *documentfinishedbufsize, pid_t *ret) {
	return exeoc_stdselect(exeargv,documentfinishedbuf,documentfinishedbufsize,ret,0, NULL);
}

int exeoc_timeout(char *exeargv[],char documentfinishedbuf[],int *documentfinishedbufsize, pid_t *ret, int timeout) {
	struct timeval tv;
	int n;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	n = exeoc_stdselect(exeargv,documentfinishedbuf,documentfinishedbufsize, ret, 0, &tv);

	//#ifdef DEBUG
		printf("exeoc_stdselect returned %i\n",n);
	//#endif

	if (tv.tv_sec == 0 && tv.tv_usec == 0) { // Timeout
              	printf("exeoc_timeout did time out.\n");
		return 0;
        }
	else {
		return n;
	}
}

