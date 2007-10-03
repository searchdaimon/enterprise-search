

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <stdlib.h>

#include "../common/exeoc.h"

int
main(void)
{
	char buf[1024*2*2];
	int buflen;
	pid_t pid;
	char *cmd[] = {
		"execo-timeout.sh",
		NULL
	};

	buflen = sizeof(buf);
	if (!exeoc_timeout(cmd, buf, &buflen, &pid, 2))
		err(1, "Unable to run prog: %s", cmd[0]);


	printf("Got buffer:\n===\n%s\n===\n", buf);

	return 0;
}
