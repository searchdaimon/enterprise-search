#include <locale.h>
#include <stdio.h>
#include <err.h>

#include "../common/boithohome.h"
#include "../getFiletype/identify_extension.h"


char *get_filetype_icon(char *ext) {
	static struct fte_data *fdata = NULL;
	if (fdata == NULL)
		fdata = fte_init(bfile("config/file_extensions.conf"));
 	/* TODO? fte_destroy(fdata) */

	static char *icon, *version;
	char *group, *descr;

	if (fdata == NULL) {
		errx(1, "No fte_data %d %s", __LINE__, __FILE__);
		return NULL;
	}
	if (!fte_getdescription(fdata, "eng", ext, &group, &descr, &icon, &version)) {
		warnx("no icon for ext %s\n", ext);
		icon[0] = '\0';
		return icon;
	}

	return icon;
}

