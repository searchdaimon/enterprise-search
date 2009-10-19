#include <sys/syscall.h>
#include <asm/unistd.h>

#include <sys/types.h>
#include <unistd.h>

#include "../logger/logger.h"

#include "nice.h"

static inline int ioprio_set(int which, int who, int ioprio)
{
        return syscall(SYS_ioprio_set, which, who, ioprio);
}

static inline int ioprio_get(int which, int who)
{
        return syscall(SYS_ioprio_get, which, who);
}

enum {
        IOPRIO_CLASS_NONE,
        IOPRIO_CLASS_RT,
        IOPRIO_CLASS_BE,
        IOPRIO_CLASS_IDLE,
};

enum {
        IOPRIO_WHO_PROCESS = 1,
        IOPRIO_WHO_PGRP,
        IOPRIO_WHO_USER,
};

#define IOPRIO_CLASS_SHIFT      13

void
ionice_idle(void)
{
	int ioprio = 0;
	int ioclass = IOPRIO_CLASS_IDLE;

	if (ioprio_set(IOPRIO_WHO_PROCESS, getpid(), ioprio | (ioclass << IOPRIO_CLASS_SHIFT))) {
		bblog_errno(ERROR, "ioprio_set(): Unable to set idle io priority");
	}
}
