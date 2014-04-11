#ifndef _DP__H_
#define _DP__H_

#ifdef WITH_THREAD

void dp_init();

int dp_lock(int lot);

int dp_unlock(int lot);

#endif

#endif
