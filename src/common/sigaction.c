#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>


#include <stdio.h>

#define _GNU_SOURCE
#include <string.h>

void
sigchild_handler(int sig, siginfo_t *sip, void *extra)
{
	pid_t child = sip->si_pid;

	if (sip->si_code == CLD_EXITED) {
		printf("Got signal '%s', si_code %d"
				" for child %d\n",
				strsignal(sip->si_signo), sip->si_code, child);
	} else {
		int status;

		printf("Got signal '%s', si_code %d"
				" for child %d: ",
				strsignal(sip->si_signo), sip->si_code, child);
		if (waitpid(-1, &status, 0) == -1) {
			perror("waitpid()");
		} else {
			if (WIFEXITED(status)) {
				printf("Exited normally\n");
			} else if (WIFSIGNALED(status)) {
				printf("Child was signaled: %s(%d)\n", strsignal(WTERMSIG(status)), WTERMSIG(status));
			} else {
				printf("Unknown exit\n");
			}
		}
	}
}

