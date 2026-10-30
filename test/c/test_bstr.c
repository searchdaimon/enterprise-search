/* Unit tests for the string helpers in src/common/bstr.c */
#include <stdlib.h>
#include <string.h>
#include "tap.h"
#include "common/bstr.h"

int main(void)
{
	char buf[64];
	char **list;
	int n;

	/* strscpy: bounded copy that always NUL-terminates */
	memset(buf, 'X', sizeof(buf));
	strscpy(buf, "hello", sizeof(buf));
	is_str(buf, "hello", "strscpy copies a short string");

	memset(buf, 'X', sizeof(buf));
	strscpy(buf, "hello world", 6);
	is_str(buf, "hello", "strscpy truncates to n-1 bytes");
	is_int(buf[6], 'X', "strscpy does not write past n");

	strscpy(buf, "", sizeof(buf));
	is_str(buf, "", "strscpy copies the empty string");

	/* chomp: strip one trailing newline */
	strcpy(buf, "line\n");
	chomp(buf);
	is_str(buf, "line", "chomp removes a trailing newline");
	chomp(buf);
	is_str(buf, "line", "chomp is a no-op without a newline");

	/* strsandr: search and replace, all occurrences */
	strcpy(buf, "a-b-c");
	strsandr(buf, "-", "+");
	is_str(buf, "a+b+c", "strsandr replaces every occurrence");

	strcpy(buf, "aXb");
	strsandr(buf, "X", "LONGER");
	is_str(buf, "aLONGERb", "strsandr grows the string");

	strcpy(buf, "aXXb");
	strsandr(buf, "X", "");
	is_str(buf, "ab", "strsandr can delete");

	strcpy(buf, "nothing here");
	strsandr(buf, "zzz", "y");
	is_str(buf, "nothing here", "strsandr leaves non-matching text alone");

	/* strcasesandr: case-insensitive variant */
	strcpy(buf, "Hello HELLO hello");
	strcasesandr(buf, sizeof(buf), "hello", "hi");
	is_str(buf, "hi hi hi", "strcasesandr ignores case");

	/* btolower: ASCII and Latin-1 upper-case letters */
	is_int(btolower('A'), 'a', "btolower lowers ASCII");
	is_int(btolower('z'), 'z', "btolower leaves lower-case alone");
	is_int(btolower('1'), '1', "btolower leaves digits alone");
	is_int(btolower(0xC5), 0xE5, "btolower lowers Latin-1 A-ring");
	is_int(btolower(0xD8), 0xF8, "btolower lowers Latin-1 O-slash");
	is_int(btolower(0xC6), 0xE6, "btolower lowers Latin-1 AE");
	is_int(btolower(0xD7), 0xF7, "btolower maps 0xD7 (multiplication sign) as the legacy code does");

	/* split: Perl-style split, NULL-terminated list */
	n = split("a;b;c", ";", &list);
	is_int(n, 3, "split returns the element count");
	is_str(list[0], "a", "split element 0");
	is_str(list[1], "b", "split element 1");
	is_str(list[2], "c", "split element 2");
	ok(list[3] == NULL, "split list is NULL-terminated");
	FreeSplitList(list);

	n = split("a;;b", ";", &list);
	is_int(n, 3, "split keeps empty fields");
	is_str(list[1], "", "split empty field is \"\"");
	FreeSplitList(list);

	n = split("single", ";", &list);
	is_int(n, 1, "split without a delimiter hit returns one element");
	is_str(list[0], "single", "split single element is the whole input");
	FreeSplitList(list);

	n = split("1<x>2<x>3<x>4<x>5<x>6<x>7<x>8<x>9<x>10<x>11<x>12", "<x>", &list);
	is_int(n, 12, "split grows its list past the initial allocation");
	is_str(list[11], "12", "split last element after growth");
	FreeSplitList(list);

	n = split("", ";", &list);
	is_int(n, 1, "split of the empty string yields one empty element");
	FreeSplitList(list);

	is_int(split(NULL, ";", &list), -1, "split rejects NULL input");

	/* strlwcat: strlcat with a warning on overflow */
	strcpy(buf, "abc");
	is_int(strlwcat(buf, "def", sizeof(buf)), 6, "strlwcat returns the combined length");
	is_str(buf, "abcdef", "strlwcat appends");

	strcpy(buf, "abc");
	is_int(strlwcat(buf, "defgh", 6), 8, "strlwcat reports the length it wanted");
	is_str(buf, "abcde", "strlwcat truncates to fit");

	/* strdupnul */
	ok(strdupnul(NULL) == NULL, "strdupnul(NULL) is NULL");
	{
		char *d = strdupnul("x");
		is_str(d, "x", "strdupnul copies");
		free(d);
	}

	return done_testing();
}
