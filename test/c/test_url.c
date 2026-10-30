/*
 * Unit tests for URL normalisation in src/common/url.c.
 *
 * The table is ported from the old ad-hoc src/urltest program (which no longer
 * compiled). An expected value of "" means the URL must be rejected, either by
 * url_normalization() or by gyldig_url().
 */
#include <string.h>
#include "tap.h"
#include "common/bstr.h"
#include "common/url.h"

struct urlcase {
	const char *in;
	const char *expect;
};

static const struct urlcase cases[] = {
	/* normal URLs, these should always pass through unchanged */
	{"http://www.boitho.com/File.html", "http://www.boitho.com/File.html"},
	{"http://sub.boitho.com/File.html", "http://sub.boitho.com/File.html"},

	/* canonicalisation */
	{"http://www.boitho.com", "http://www.boitho.com/"},
	{"http://www.boitho.com:80/", "http://www.boitho.com/"},
	{"http://www.boitho.com:80/File.html", "http://www.boitho.com/File.html"},
	{"http://www.boitho.com?a=b", "http://www.boitho.com/?a=b"},
	{"HTTP://www.Boitho.coM/File.html", "http://www.boitho.com/File.html"},
	{"http://www.boitho.com/side.html#august", "http://www.boitho.com/side.html"},

	/* must be rejected */
	/* legalUrl() wants a host with a subdomain (www.); the old urltest expected
	 * these to be rewritten with www. added, which the code never did. */
	{"http://boitho.com/", ""},
	{"http://boitho.com", ""},
	{"http://boitho.com/File.html", ""},
	{"http://www.boitho.com/div/javascript:openchild('/product-popup.pl?pcode=10136136');", ""},
	{"http://www.boitho.com/div/JavaScript:openchild('/product-popup.pl?pcode=10136136');", ""},
	{"http://www.swindon.gov.uk/atozlistingsbyletterdisplay/socialcare/signpost/socialcare/signpost/community/roadstransport/feedbackcomments.htm", ""},
	{"", ""},
	{"http://x", ""},
};

int main(void)
{
	size_t i;
	char url[512];
	char name[700];

	for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		const struct urlcase *c = &cases[i];
		int accepted;

		strscpy(url, c->in, sizeof(url));
		accepted = url_normalization(url, sizeof(url)) && gyldig_url(url);

		if (c->expect[0] == '\0') {
			snprintf(name, sizeof(name), "rejects \"%s\"", c->in);
			ok(!accepted, name);
		} else {
			snprintf(name, sizeof(name), "accepts \"%s\"", c->in);
			ok(accepted, name);
			snprintf(name, sizeof(name), "normalises \"%s\" -> \"%s\"", c->in, c->expect);
			is_str(url, c->expect, name);
		}
	}

	/* url_depth counts path segments below the host */
	is_int(url_depth("http://www.boitho.com/"), 0, "url_depth of the root is 0");
	is_int(url_depth("http://www.boitho.com/a/"), 1, "url_depth of one directory");
	is_int(url_depth("http://www.boitho.com/a/b/c.html"), 3, "url_depth counts every segment including the file");

	return done_testing();
}
