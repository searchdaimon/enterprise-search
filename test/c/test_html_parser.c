/*
 * Unit tests for the bison/flex HTML parser in src/parser2 (html_parser_run).
 *
 * This is the parser searchd uses to build result snippets and IndexerLot uses
 * to tokenise documents. Its token values used to travel through bison's
 * default `int` YYSTYPE, which truncated the char* on 64-bit and crashed on the
 * first tag; these tests parse a small document end to end.
 */
#include <stdlib.h>
#include <string.h>
#include "tap.h"
#include "parser2/html_parser.h"

static char words[4096];
static int nwords, nlinks, ntitlewords;

static void collect(char *word, int pos, enum parsed_unit pu, enum parsed_unit_flag puf, void *wordlist)
{
	(void)pos; (void)wordlist;
	if (pu == pu_word || pu == pu_linkword) {
		nwords++;
		if (puf == puf_title) ntitlewords++;
		if (strlen(words) + strlen(word) + 2 < sizeof(words)) {
			strcat(words, word);
			strcat(words, " ");
		}
	} else if (pu == pu_link) {
		nlinks++;
	}
}

static void parse(const char *url, const char *html, char **title, char **body)
{
	char *copy = strdup(html);
	words[0] = '\0';
	nwords = nlinks = ntitlewords = 0;
	*title = *body = NULL;
	html_parser_run((char *)url, copy, strlen(copy), title, body, collect, NULL);
	free(copy);
}

int main(void)
{
	char *title, *body;

	html_parser_init();

	parse("http://smoke.test/kumquat.html",
	      "<html><head><title>Kumquat Page</title></head>"
	      "<body><h1>Marmalade</h1><p>kumquat marmalade recipe</p>"
	      "<a href=\"/other.html\">other page</a></body></html>",
	      &title, &body);
	ok(title != NULL, "a title is extracted");
	is_str(title, "Kumquat Page", "title text");
	ok(body != NULL && strstr(body, "marmalade") != NULL, "body text contains the paragraph");
	ok(body != NULL && strstr(body, "<p>") == NULL, "body text has tags stripped");
	ok(strstr(words, "kumquat ") != NULL, "word callback saw 'kumquat'");
	ok(strstr(words, "recipe ") != NULL, "word callback saw 'recipe'");
	ok(ntitlewords >= 2, "title words are flagged as title");
	ok(nlinks >= 1, "the link was reported");
	ok(strstr(words, "other ") != NULL, "link text words are reported");
	free(title); free(body);

	parse("http://smoke.test/plain.html", "no tags at all", &title, &body);
	ok(body != NULL && strstr(body, "no tags at all") != NULL, "tagless input passes through");
	free(title); free(body);

	parse("http://smoke.test/empty.html", "", &title, &body);
	ok(1, "empty input does not crash");
	free(title); free(body);

	parse("http://smoke.test/broken.html", "<html><body><p>unclosed <b>bold <a href='x'>link", &title, &body);
	ok(body != NULL && strstr(body, "unclosed") != NULL, "unterminated markup still yields text");
	free(title); free(body);

	html_parser_exit();
	return done_testing();
}
