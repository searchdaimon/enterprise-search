/* Unit tests for the flex-generated query parser in src/query/ (get_query). */
#include <string.h>
#include "tap.h"
#include "query/query_parser.h"

static void parse(const char *text, query_array *qa)
{
	char buf[1024];
	strncpy(buf, text, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	memset(qa, 0, sizeof(*qa));
	get_query(buf, strlen(buf), qa);
}

int main(void)
{
	query_array qa;
	char out[1024];

	parse("hello world", &qa);
	is_int(qa.n, 2, "two words parse into two terms");
	is_int(qa.query[0].operand, QUERY_WORD, "first term is a plain word");
	is_str(qa.query[0].s[0], "hello", "first term text");
	is_str(qa.query[1].s[0], "world", "second term text");
	destroy_query(&qa);

	parse("\"new york\" pizza", &qa);
	is_int(qa.n, 2, "a phrase and a word parse into two terms");
	is_int(qa.query[0].operand, QUERY_PHRASE, "quoted text is a phrase");
	is_int(qa.query[0].n, 2, "the phrase holds two words");
	is_str(qa.query[0].s[0], "new", "phrase word 1");
	is_str(qa.query[0].s[1], "york", "phrase word 2");
	is_int(qa.query[1].operand, QUERY_WORD, "the trailing word is a plain word");
	destroy_query(&qa);

	parse("apple -banana", &qa);
	is_int(qa.n, 2, "minus operator still yields two terms");
	is_int(qa.query[1].operand, QUERY_SUB, "-word is a subtracted word");
	is_str(qa.query[1].s[0], "banana", "subtracted word text");
	destroy_query(&qa);

	parse("Hello WORLD", &qa);
	is_int(qa.n, 2, "mixed case parses");
	is_str(qa.query[0].s[0], "Hello", "the parser preserves case (lower-casing happens downstream)");
	is_str(qa.query[1].s[0], "WORLD", "the parser preserves case (2)");
	destroy_query(&qa);

	parse("report collection:intranet", &qa);
	is_int(qa.n, 2, "collection: command parses as its own term");
	is_int(qa.query[1].operand, QUERY_COLLECTION, "collection: sets the collection operand");
	is_str(qa.query[1].s[0], "intranet", "collection name is captured");
	destroy_query(&qa);

	parse("   ", &qa);
	is_int(qa.n, 0, "whitespace-only query has no terms");
	destroy_query(&qa);

	parse("round trip", &qa);
	sprint_query(out, sizeof(out), &qa);
	is_str(out, "round trip", "sprint_query reproduces a simple query");
	destroy_query(&qa);

	return done_testing();
}
