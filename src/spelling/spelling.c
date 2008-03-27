#include <aspell.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "spelling.h"
#include "../common/boithohome.h"

/* XXX: Remove unused bfopen warning */
void *
_dummy_func_spl(void)
{
	void *p = bfopen;
	return p;
}

char *
spelling_conv(iconv_t conv, char *str)
{
	char *conv_word, *conv_wordp;
	size_t conv_in, conv_size, totsize;

	totsize = conv_size = strlen(str) * 2 + 1;
	conv_wordp = conv_word = malloc(conv_size);
	conv_in = strlen(str)+1;
	totsize -= iconv(conv, &str, &conv_in, &conv_word, &conv_size);
	conv_wordp[totsize] = '\0';

	return conv_wordp;
}


static char *
spelling_lookup_lang(char *lang)
{
	return lang;
}


struct spelling *
spelling_init(char *lang)
{
	AspellConfig *config;
	AspellCanHaveError *ret;
	struct spelling *spelling = malloc(sizeof(struct spelling));

	if (spelling == NULL)
		return NULL;

	config = new_aspell_config();

	aspell_config_replace(config, "lang", spelling_lookup_lang(lang));
	aspell_config_replace(config, "dict-dir", bfile("data/dict/"));
	aspell_config_replace(config, "encoding", "iso-8859-1");

	ret = new_aspell_speller(config);
	delete_aspell_config(config);

	if (aspell_error(ret) != 0) {
		delete_aspell_can_have_error(ret);
		return NULL;
	}

	spelling->speller = to_aspell_speller(ret);

	spelling->conv = iconv_open("iso-8859-1", "utf-8");
	spelling->conv_out = iconv_open("utf-8", "iso-8859-1");

	return spelling;
}


struct spelling_document *
spelling_document_init(char *lang)
{
	AspellCanHaveError *ret;
	AspellDocumentChecker *checker;
	struct spelling *speller;
	struct spelling_document *sd;

	sd = malloc(sizeof(*sd));
	if (sd == NULL)
		return NULL;
	speller = spelling_init(lang);
	if (speller == NULL) {
		free(sd);
		return NULL;
	}

	ret = new_aspell_document_checker(speller->speller);
	if (aspell_error(ret) != 0) {
		free(sd);
		spelling_destroy(speller);
		return NULL;
	}
	checker = to_aspell_document_checker(ret);

	sd->spelling = speller;
	sd->checker = checker;
	return sd;
}

/* XXX: Not wide char safe */
/* Remeber to free returned string */
char *
spelling_document_line(struct spelling_document *sd, char *in_line)
{
	char *newline;
	int diff, line_len;
	size_t line_size, conv_line;
	struct AspellToken token;
	char *line;
	size_t conv_in;

	line = spelling_conv(sd->spelling->conv, in_line);

	line_len = strlen(line);
	line_size = line_len + (line_len/10);
	if ((newline = malloc(line_size)) == NULL)
		return NULL;

	strcpy(newline, line);
	free(line);
	aspell_document_checker_process(sd->checker, newline, line_len);
	diff = 0;
	while (token = aspell_document_checker_next_misspelling(sd->checker),
	       token.len != 0) {
		char *word_begin;
		const char *word;
		int word_len;
		const AspellWordList *wl;
		AspellStringEnumeration *els;

		word_begin = newline + token.offset + diff;
		wl = aspell_speller_suggest(sd->spelling->speller, word_begin, token.len);
		els = aspell_word_list_elements(wl);
		if ((word = aspell_string_enumeration_next(els)) == NULL)
			continue;
		word_len = strlen(word);
		diff += word_len - token.len;
		memmove(word_begin + word_len, word_begin + token.len, strlen(word_begin + token.len) + 1);
		memcpy(word_begin, word, word_len);
		line_len += diff;
	}

	line = spelling_conv(sd->spelling->conv_out, newline);
	free(newline);

	return line;
}


void
spelling_document_destroy(struct spelling_document *sd)
{
	spelling_destroy(sd->spelling);
	delete_aspell_document_checker(sd->checker);
	free(sd);
}

int
spelling_correct(char *word, struct spelling *s)
{
	int ret;
	char *conv_word;
	char *p;
	size_t conv_in, conv_out;

	conv_word = spelling_conv(s->conv, word);

	if (isdigit(conv_word[0])) {
		int i, digit;

		digit = 1;
		for (i = 1; conv_word[i] != '\0'; i++) {
			if (!isdigit(conv_word[i])) {
				digit = 0;
				break;
			}
		}
		if (digit)
			return 1;
	}


	ret = aspell_speller_check(s->speller, conv_word, -1);

	free(conv_word);

	return ret;
} 


/* XXX: Return a weighted struct instead */
char **
spelling_suggestions(char *word, struct spelling *s)
{
	const AspellWordList *wl;
	AspellStringEnumeration *els;
	char **suggestions, **p;
	const char *sword;
	char *conv_word;

	conv_word = spelling_conv(s->conv, word);

	wl = aspell_speller_suggest(s->speller, conv_word, -1);
	free(conv_word);
	p = malloc(sizeof(char *) * (aspell_word_list_size(wl) + 1)); /* Leave room for NULL termination */
	suggestions = p;
	els = aspell_word_list_elements(wl);
	while ((sword = aspell_string_enumeration_next(els)) != 0) {
		conv_word = spelling_conv(s->conv_out, sword);
		*p = conv_word;

		if (*p == NULL) {
			while (p >= suggestions) {
				free(*p);
				p--;
			}
			return NULL;
		}
		p++;
	}
	*p = NULL;


	return suggestions;
}

void
spelling_destroy(struct spelling *s)
{
	delete_aspell_speller(s->speller);
	iconv_close(s->conv);
	iconv_close(s->conv_out);
	free(s);
}


void
spelling_suggestions_destroy(char **p)
{
	char **p2 = p;

	for (; *p2; p2++) {
		free(*p2);
	}
	free(p);
}


#ifdef TESTMAIN
#include <stdio.h>

static int
test1(int argc, char **argv)
{
	struct spelling *spelling;

        if (argc < 3) {
                fprintf(stderr, "Syntax: ./spellchecker dicthash words [words ...]\n");
                return -1;
        }
	
	spelling = spelling_init(argv[1]);

	argv += 2;
	argc -= 2;

	for (; argc > 0; argv++, argc--) {
		int ret = spelling_correct(*argv, spelling);
		printf("Checking %s... ", *argv);
		if (ret) {
			printf("correct!\n");
		}
		else {
			char **p, **p2;
			printf("incorrect, perhaps you meant:\n");
			p2 = p = spelling_suggestions(*argv, spelling);
			if (p == NULL) {
				printf("could not find any suggestions\n");
				continue;
			}
			for (; *p; p++) {
				printf("\t%s\n", *p);
			}
			spelling_suggestions_destroy(p2);
		}
	}

	return 0;
}

static int
test2(void)
{
	struct spelling_document *sd;
	char *line;
	char *some = "Hello, this is a simple tset!\n";

	sd = spelling_document_init("bb");
	if (sd == NULL) {
		fprintf(stderr, "Unable to init document checker.\n");
		return 1;
	}

	line = spelling_document_line(sd, some);
	printf("Corrected line: '%s'\n", line);
	free(line);

	spelling_document_destroy(sd);
	
	return 0;
}


int
main(int argc, char **argv)
{
	test1(argc, argv);
	test2();

	return(0);
}


#endif
