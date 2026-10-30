/*
 * Unit tests for src/common/bfileutil.c.
 *
 * sfindductype() picks the file extension out of a path or URL. It used to cast
 * the path pointer through `int`, which truncates it on 64-bit and crashed the
 * boitho-bbdn worker for every pushed document whose type was not given
 * explicitly. These tests pin the fixed behaviour.
 */
#include <stdlib.h>
#include "tap.h"
#include "common/bfileutil.h"

static void check(const char *path, const char *expect, const char *name)
{
	char buf[512];
	char *got;

	snprintf(buf, sizeof(buf), "%s", path);
	got = sfindductype(buf);
	is_str(got, expect, name);
	free(got);
}

int main(void)
{
	check("http://smoke.test/page1.html", "html", "extension of a URL");
	check("/share/docs/report.PDF", "PDF", "extension keeps its case (callers lower-case it)");
	check("archive.tar.gz", "gz", "only the last extension counts");
	check("http://host/dir/", NULL, "a directory URL has no extension");
	check("http://host/noext", NULL, "no dot after the last slash means no extension");
	check("http://host/a.b/file", NULL, "a dot in a parent directory does not count");
	check("http://host/page.html%20copy", "html", "a %-escape terminates the extension");
	check("http://host/page.html more", "html", "a space terminates the extension");
	check("file.", "", "a trailing dot yields the empty extension");
	return done_testing();
}
