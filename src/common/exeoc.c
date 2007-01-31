#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define linelen 512

int exeoc(char *exeargv[],char documentfinishedbuf[],int *documentfinishedbufsize) {
	pid_t pid;
	int     pipefd[2];
	int i,n;

	//printf("documentfinishedbufsize %i\n",(*documentfinishedbufsize));
	#ifdef DEBUG
		printf("runing program \"%s\"\n",exeargv[0]);
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

		//sleep(1);
		//char *execvarg[] = {"/bin/cat","/tmp/test.txt", '\0'};
		//execv("/bin/cat",execvarg);
		//printf("program is \"%s\"\n",exeargv[0]);
		if (execv(exeargv[0],exeargv) == -1) {
			perror(exeargv[0]);
		}
		printf("did return from execv. Somthing is wrong\n");
		return 0;
	}
	else {
		//printf("perent. Child pid %i\n",pid);
		/* Parent process closes up output side of pipe */
                close(pipefd[1]);

		/*
		trenger ikke noe wait her. Vi blokker jo ved read
		#ifdef DEBUG
			printf("waitng for pid \"%i\"\n",pid);
		#endif

		//waitpid(pid,&waitstatus,0);
		//waitpid(pid,&waitstatus,WUNTRACED);

		#ifdef DEBUG
			printf("waitpid finished. waitstatus %i\n",waitstatus);
		#endif 
		*/
		i=0;
		while (((n = read(pipefd[0], &documentfinishedbuf[i], linelen)) > 1) 
			&& (i + linelen < (*documentfinishedbufsize))) {
			i += n;			
			//printf("did read %ib, tot %i\n",n,i);
		}
		(*documentfinishedbufsize) = i;
		documentfinishedbuf[(*documentfinishedbufsize) +1] = '\0';
		#ifdef DEBUG
		printf("documentfinishedbufsize %i\n",(*documentfinishedbufsize));
		#endif
		close(pipefd[0]);


	}

}
