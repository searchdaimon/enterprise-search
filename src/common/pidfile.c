#include <sys/types.h>

#include <stdio.h>

#include "boithohome.h"

void
write_gpidfile(const char *name)
{
	pid_t gpid;
	FILE *fp;
	char path[2048];

	if ((gpid = setsid()) == -1) {
		if ((gpid = getpgrp()) == -1) {
			warn("getpgrp()");
			return;
		}
	}
	sprintf(path, "%s/%s.gpid", bfile("var/"), name);
	fp = fopen(path, "w");
	if (fp == NULL) {
		warn("fopen: %s", path);
		return;
	}
	fprintf(fp, "%d", gpid);
	fclose(fp);
}
