#include <stdio.h>
#include <string.h>

#include "gcwhisper.h"
#include "lot.h"


struct {
	whisper_t flag;
	const char *str;
} whispers[] = {
	{ .flag = GCWHISPER_NOTOLD, "notold" },
	{ .flag = 0, .str = NULL },
};

whisper_t
gcwhisper_read(char *subname)
{
	FILE *fp;
	whisper_t w;
	char buf[2048];

	fp = lotOpenFileNoCasheByLotNr(1, "gcwhisper", "r", 's', subname);
	if (fp == NULL)
		return 0;

	w = 0;
	while (fgets(buf, sizeof(buf), fp)) {
		int i;
		char *p;

		/* remove trailing newline */
		if ((p = strrchr(buf, '\n')))
			*p = '\0';
			
		for (i = 0; whispers[i].str != NULL; i++) {
			if (strcmp(whispers[i].str, buf) == 0)
				w |= whispers[i].flag;
		}
		/* We did not find a match */
		if (whispers[i].str == NULL)
			warnx("Trying to read unknown whisper string: %s", buf);
	}

	fclose(fp);
	
	return w;
}

void
gcwhisper_write(char *subname, whisper_t whisper)
{
	FILE *fp;
	whisper_t has;
	int i;

	has = gcwhisper_read(subname);

	fp = lotOpenFileNoCasheByLotNr(1, "gcwhisper", ">>", 'e', subname);
	for (i = 0; whispers[i].str != NULL; i++) {
		/* want it */
		if ((whispers[i].flag & whisper) && (has & whispers[i].flag) == 0) {
			fprintf(fp, "%s\n", whispers[i].str);
		}
	}
	fclose(fp);
	
}
