
#ifndef _SRC_COMMON_NICE_H_
#define _SRC_COMMON_NICE_H_


#include <sys/syscall.h>
#include <asm/unistd.h>

#include <sys/types.h>
#include <unistd.h>


void ionice_benice(void);


#endif /* _SRC_COMMON_NICE_H_ */
