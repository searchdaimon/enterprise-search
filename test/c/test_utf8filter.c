/* Unit tests for the flex-generated UTF-8 filter in src/utf8-filter/ */
#include <stdlib.h>
#include <string.h>
#include "tap.h"
#include "utf8-filter/utf8-filter.h"

static void check(const char *in, const char *expect, const char *name)
{
	char *copy = strdup(in);
	char *out = utf8_filter(copy);
	is_str(out, expect, name);
	free(out);
	free(copy);
}

int main(void)
{
	check("plain ascii text", "plain ascii text", "ASCII passes through unchanged");
	check("", "", "the empty string stays empty");
	/* UTF-8 for "blåbærsyltetøy" (Norwegian: blueberry jam) */
	check("bl\xc3\xa5" "b\xc3\xa6" "rsyltet\xc3\xb8" "y", "bl\xc3\xa5" "b\xc3\xa6" "rsyltet\xc3\xb8" "y", "valid 2-byte UTF-8 sequences are kept");
	check("\xe2\x82\xac" "100", "\xe2\x82\xac" "100", "valid 3-byte UTF-8 (euro sign) is kept");
	check("a\xe5" "b", "ab", "a lone Latin-1 byte is removed");
	check("\xff\xfe", "", "invalid lead bytes are removed");
	check("tr\xc3" "uncated", "truncated", "a truncated multibyte sequence is dropped, text after it kept");
	return done_testing();
}
