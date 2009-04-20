#ifndef _SIGACTION_H_
#define _SIGACTION_H_

void sigchild_handler(int sig, siginfo_t *sip, void *extra);

#endif
