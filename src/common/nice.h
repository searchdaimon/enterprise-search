
#ifndef _SRC_COMMON_NICE_H_
#define _SRC_COMMON_NICE_H_

#include <sys/syscall.h>
#include <asm/unistd.h>

#include <sys/types.h>
#include <unistd.h>


// hvis vi ikke kjener SYS_ioprio_get har vi en gammel gcc/glibc (for eks bbh-001) gjømmer hele nice systemet
#ifdef SYS_ioprio_get

void ionice_benice(void);

#endif

#endif /* _SRC_COMMON_NICE_H_ */
