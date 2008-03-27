/*
 * Eirik A. Nygaard
 * February 2007
 */


%{

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __INGETDATE
#include "getdate.h"
#include "dateview.h"

//#define yyparse getdate_yyparse
#define yylex datelib_yylex
#define yyerror datelib_yyerror

//void yyerror(char *);
//static int datelib_yylex(YYSTYPE *, char **);

struct nexttoken {
	enum yytokentype token;
	union {
		int number;
	} value;
};

static int yyparse ();
static int yylex ();
static int yyerror (char **, struct datelib *, struct nexttoken *, const char *);

void subtract_date(struct tm *, enum yytokentype, int);
static inline void set_lowest(struct datelib *, enum yytokentype);

%}


%union {
	int number;
	int month;
}

%token unknown
%token NUMBER WORD
%token DAY WEEK MONTH YEAR 
%token PLUS

%type	<number>	NUMBER
%type	<number>	number

%pure-parser
%lex-param   {char **input}
%lex-param   {struct nexttoken *nexttoken}
%parse-param {char **input}
%parse-param {struct datelib *result}
%parse-param {struct nexttoken *nexttoken}


%%

daterange:
		/* NULL */
	|	daterange date plus
	;

plus:
		/* NULL */
	|	PLUS {
		result->frombigbang = 1;
	}

number:
		/* NULL means 1 */
		{ $$ = 1; }
	|	NUMBER { $$ = yylval.number; }

date:
		day
	|	week
	|	month
	|	year
	;

day:
		number DAY {
			set_lowest(result, DAY);
			result->modify.day += $1;
			//printf("day: %d\n", $1);
		}
	;

week:
		number WEEK {
			set_lowest(result, WEEK);
			result->modify.week += $1;
			//printf("week: %d\n", $1);
		}
	;

month:
		number MONTH {
			set_lowest(result, MONTH);
			result->modify.month += $1;
			//printf("month: %d\n", $1);
		}
	;

year:
		number YEAR {
			set_lowest(result, YEAR);
			result->modify.year += $1;
			//printf("year: %d\n", $1);
		}
	;

%%



#define issep(c) (isspace(c) || c == '\n' || c == '\0')
#define lexreturn(token) do { *inputp = input; return (token); } while (0)

struct wordtable {
	int token;
	const char *word;
	int number;
};

struct wordtable wordtable[] = {
	{ YEAR, "years", -1 },
	{ YEAR, "year", -1 },
	{ MONTH, "month", -1 },
	{ MONTH, "months", -1 },
	{ WEEK, "week", -1 },
	{ WEEK, "weeks", -1 },
	{ DAY, "day", -1 },
	{ DAY, "days", -1 },
	{ DAY, "today", 0 },
	{ DAY, "yesterday", 1 },
	{ -1, NULL, -1 }
};

struct numbertable {
	const char *word;
	int number;
};

struct numbertable numbertable[] = {
	{ "this", 0 },
	{ "last", 1 },
	{ "two", 2 },
	{ NULL, -1 }
};


/* Bunch of C code */

/* XXX: Rewrite into flex */
static int
_datelib_yylex(YYSTYPE *yylval, char **inputp, struct nexttoken *nt)
{
	char *input;
	int c;

	input = *inputp;

	if (nt->token != unknown) {
		enum yytokentype tok = nt->token;
		nt->token = unknown;
		return tok;
	}

start:
	while (isspace(*input))
		input++;
	if (*input == '\0')
		return YYEOF;

	c = *input;
	if (isdigit(c)) { // || ((c == '-' || c == '+') && isdigit(*(input + 1)))) {
		int sign = 1;
		char *p;

		/* + or - */
		if (!isdigit(*input))
			sign = *input++ == '-' ? -1 : 1;

		for (p = input; isdigit(*input); input++) {
		//	printf("looping\n");
		}
		if (!issep(*input))
			lexreturn(unknown);

		if (*input != '\0') {
			*input = '\0';
			input++;
		}

		yylval->number = sign * atoi(p);
		lexreturn(NUMBER);
	}

	if (isalpha(c)) {
		int i;
		char *p;

		for (p = input; (isalnum(*input)); input++)
			;
		if (!issep(*input))
			lexreturn(unknown);

		if (*input != '\0') {
			*input = '\0';
			input++;
		}
		if (strcmp(p, "ago") == 0) /* XXX */
			goto start;
		if (strcmp(p, "plus") == 0)
			lexreturn(PLUS);

		for (i = 0; wordtable[i].token != -1; i++) {
			if (strcmp(wordtable[i].word, p) == 0) {
				if (wordtable[i].number != -1) {
					nt->token = wordtable[i].token;
					yylval->number = wordtable[i].number;
					lexreturn(NUMBER);
				} else {
					lexreturn(wordtable[i].token);
				}
			}
		}
		for (i = 0; numbertable[i].word != NULL; i++) {
			if (strcmp(numbertable[i].word, p) == 0) {
				yylval->number = numbertable[i].number;
				lexreturn(NUMBER);
			}
		}

	}

	return unknown;
}

/*
 * Have to call this in the correct order, first for the year, then month, and
 * so on.
 */

static int months[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void
subtract_date(struct tm *tm, enum yytokentype type, int number)
{
	switch (type) {
		case YEAR:
			tm->tm_year -= number;
			break;
		case MONTH:
		{
			subtract_date(tm, YEAR, number / 12);
			number %= 12;
			if (tm->tm_mon < number) {
				tm->tm_mon = 11 + tm->tm_mon - (number - 1);
				subtract_date(tm, YEAR, 1);
			}
			else {
				tm->tm_mon -= number;
			}
			break;
		}
#if 0
		case WEEK:
			subtract_date(tm, DAY, number * 7);
			break;
#endif
		case DAY:
			{
				int dmon = months[tm->tm_mon];

				/* Leap year? */
				dmon += (tm->tm_mon == 1 && tm->tm_year % 4 == 0 &&
				         (tm->tm_year % 100 != 0 || tm->tm_year % 400 == 0))
					? 1 : 0;

				if (number > dmon) {
					subtract_date(tm, MONTH, 1);
					subtract_date(tm, DAY, number - dmon);
					break;
				}
				if (tm->tm_mday <= number) {
					int days;
					subtract_date(tm, MONTH, 1);
					days = months[tm->tm_mon];
					days += (tm->tm_mon == 1 && tm->tm_year % 4 == 0 &&
					         (tm->tm_year % 100 != 0 || tm->tm_year % 400 == 0))
					        ? 1 : 0;
					tm->tm_mday = tm->tm_mday - (number - 1) + days;
				}
				else {
					tm->tm_mday -= number;
				}
			}
			break;
		default:
			fprintf(stderr, "Unknown type: %d\n", type);
			break;
	}
}


static int
datelib_yylex(YYSTYPE *yylval, char **inputp, struct nexttoken *nt)
{
	int foo = _datelib_yylex(yylval, inputp, nt);

	//printf("Foo: %d\n", foo);

	return foo;
}


static int
datelib_yyerror(char **input, struct datelib *dl, struct nexttoken *nt, const char *s)
{
	dl->frombigbang = 2;
	return 0;
}

static inline void
set_lowest(struct datelib *dl, enum yytokentype type)
{
	/* XXX: Depends on the way bison generate an enum from the tokens */
	if (type == YEAR || type < dl->lowest)
		dl->lowest = type;
}

void
fixdate(struct datelib *dl, struct tm *tmend)
{

	subtract_date(&dl->tmstart, YEAR, dl->modify.year);
	subtract_date(&dl->tmstart, MONTH, dl->modify.month);
	subtract_date(&dl->tmstart, DAY, (dl->modify.week * 7));
	/* Find start of current week */
	if (dl->modify.week > 0 || dl->lowest == WEEK) {
		int dayssinces;
		int days;
		int i;

		dayssinces = 0;
		for (i = 0; i < dl->tmstart.tm_mon; i++)
			dayssinces += months[i];
		if (dl->tmstart.tm_mon > 1) {
			if (dl->tmstart.tm_year % 4 == 0 && (dl->tmstart.tm_year % 100 != 0 || dl->tmstart.tm_year % 400 == 0))
				dayssinces++;

		}
		days = dayssinces;
		dayssinces += dl->tmstart.tm_mday;


		dayssinces -= dayssinces%7;
		dayssinces -= days;
		dl->tmstart.tm_mday = dayssinces;
	}
	subtract_date(&dl->tmstart, DAY, dl->modify.day);

	memcpy(tmend, &dl->tmstart, sizeof *tmend);
	switch (dl->lowest) {
		case YEAR:
			tmend->tm_year++;
			tmend->tm_mon = 0;
			dl->tmstart.tm_mon= 0;
		case MONTH:
			if (dl->lowest == MONTH) {
				if (tmend->tm_mon == 11) {
					tmend->tm_mon = 0;
					tmend->tm_year++;
				}
				else {
					tmend->tm_mon++;
				}
			}
			dl->tmstart.tm_mday = 0;
			tmend->tm_mday = 0;
		case WEEK:
			if (dl->lowest == WEEK) {
				if (tmend->tm_mday > 23) {
					tmend->tm_mday = 30 - tmend->tm_mday + 7;
					tmend->tm_mon++;
				}
				else {
					tmend->tm_mday += 7;
				}
			}
			//dl->tmstart.tm_mday = 0;
			//tmend->tm_mday = 0;
		case DAY:
			if (dl->lowest == DAY) {
				if (tmend->tm_mday >= 30) {
					tmend->tm_mday = 1;
					tmend->tm_mon++;
				}
				else {
					tmend->tm_mday++;
				}
			}
			dl->tmstart.tm_hour = 0;
			dl->tmstart.tm_min = 0;
			dl->tmstart.tm_sec = 0;
			tmend->tm_hour = 0;
			tmend->tm_min = 0;
			tmend->tm_sec = 0;
		default:
			break;
	}

}

time_t
getdate_dateview(enum dateview_output_type type)
{
	time_t now, test;
	struct tm tmend;
	struct datelib dl;
	enum yytokentype datetype;

	now = time(NULL);
	gmtime_r(&now, &dl.tmstart);
	dl.lowest = type;
	dl.frombigbang = 0;
	memset(&dl.modify, '\0', sizeof(dl.modify));
	datetype = YEAR;
	switch(type) {
		case TODAY: dl.modify.day = 0; datetype = DAY; break;
		case YESTERDAY: dl.modify.day = 1; datetype = DAY; break;
		case THIS_WEEK: dl.modify.week = 0; datetype = WEEK; break;
		case THIS_MONTH: dl.modify.month= 0; datetype = MONTH; break;
		case THIS_YEAR: dl.modify.year= 0; datetype = YEAR; break;
		case LAST_YEAR: dl.modify.year = 1; datetype = YEAR; break;
		case TWO_YEARS_PLUS: dl.modify.year = 2; dl.frombigbang = 1; datetype = YEAR; break;
	}
	dl.lowest = datetype;
	fixdate(&dl, &tmend);
	test = mktime(&dl.tmstart);
	dl.start = dl.frombigbang ? 0 : test;
	//test = mktime(&tmend);
	//dl->end = test;
	return dl.start;
}

int
getdate(char *str, struct datelib *dl)
{
	char *p, *input = strdup(str);
	time_t now, test;
	struct tm tmend;
	struct nexttoken nexttoken;
	int i;

	if (input == NULL)
		return -1;

	p = input;
	
	now = time(NULL);
	gmtime_r(&now, &dl->tmstart);
	dl->lowest = YEAR;
	dl->frombigbang = 0;
	memset(&dl->modify, '\0', sizeof(dl->modify));
	nexttoken.token = unknown;
	yyparse(&input, dl, &nexttoken);
	if (dl->frombigbang == 2) {
		free(p);
		return -1;
	}
	fixdate(dl, &tmend);
	test = mktime(&dl->tmstart);
	dl->start = dl->frombigbang ? 0 : test;
	free(p);
	test = mktime(&tmend);
	dl->end = test;
	return 0;
}

#ifdef TEST_GETDATE

int
main(int argc, char **argv)
{
	struct datelib dl;

	getdate("2 years 5 days 1 week ago", &dl);
	printf("%s\n", ctime(&dl.start));
	printf("%s\n", ctime(&dl.end));
	getdate("1 years", &dl);
	printf("%s\n", ctime(&dl.start));
	printf("%s\n", ctime(&dl.end));
	getdate("2 months", &dl);
	printf("%s\n", ctime(&dl.start));
	printf("%s\n", ctime(&dl.end));
	getdate("1 months", &dl);
	printf("%s\n", ctime(&dl.start));
	printf("%s\n", ctime(&dl.end));


	getdate("last months", &dl);

	printf("%s\n", ctime(&dl.start));
	printf("%s\n", ctime(&dl.end));

	return 0;
}

#endif /* TEST_GETDATE */
