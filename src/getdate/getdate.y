%{
/*
**  Originally written by Steven M. Bellovin <smb@research.att.com> while
**  at the University of North Carolina at Chapel Hill.  Later tweaked by
**  a couple of people on Usenet.  Completely overhauled by Rich $alz
**  <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990;
**
**  This grammar has 10 shift/reduce conflicts.
**
**  This code is in the public domain and has no copyright.
*/
/*
**  Modified to be a reentrant parser and added multiple language support
**  by Eirik A. Nygaard.
*/
/* SUPPRESS 287 on yaccpar_sccsid *//* Unused static variable */
/* SUPPRESS 288 on yyerrlab *//* Label unused */

#include <sys/types.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "getdate.h"
#include "languages.h"

/* The code at the top of get_date which figures out the offset of the
   current time zone checks various CPP symbols to see if special
   tricks are need, but defaults to using the gettimeofday system call.
   Include <sys/time.h> if that will be used.  */


/* NOTES on rebuilding getdate.c (particularly for inclusion in CVS
   releases):

   We don't want to mess with all the portability hassles of alloca.
   In particular, most (all?) versions of bison will use alloca in
   their parser.  If bison works on your system (e.g. it should work
   with gcc), then go ahead and use it, but the more general solution
   is to use byacc instead of bison, which should generate a portable
   parser.  I played with adding "#define alloca dont_use_alloca", to
   give an error if the parser generator uses alloca (and thus detect
   unportable getdate.c's), but that seems to cause as many problems
   as it solves.  */

#define yyparse getdate_yyparse
#define yylex getdate_yylex
#define yyerror getdate_yyerror

static int yyparse ();
static int yylex ();
static int yyerror ();


/*
** Date tables and which language they belong to.
*/

typedef struct _LANGTABLE {
	const TABLE *MonthDayTable;
	const TABLE *UnitsTable;
	const TABLE *OtherTable;
	const TABLE *TimezoneTable;
	const TABLE *MilitaryTable;
} LANGTABLE;

LANGTABLE languagetable[LANGUAGE_MAX];
#define lookup_language(lang) (lang > LANGUAGE_LAST) ? NULL : &languagetable[lang]


/*
**  Daylight-savings mode:  on, off, or not yet known.
*/
typedef enum _DSTMODE {
    DSTon, DSToff, DSTmaybe
} DSTMODE;

/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum _MERIDIAN {
    MERam, MERpm, MER24
} MERIDIAN;

/*
**  Global variables.  We could get rid of most of these by using a good
**  union as the yacc stack.  (This routine was originally written before
**  yacc had the %union construct.)  Maybe someday; right now we only use
**  the %union very rarely.
*/

struct yyglobalvars {
	DSTMODE	yyDSTmode;
	time_t	yyDayOrdinal;
	time_t	yyDayNumber;
	int	yyHaveDate;
	int	yyHaveDay;
	int	yyHaveRel;
	int	yyHaveTime;
	int	yyHaveZone;
	time_t	yyTimezone;
	time_t	yyDay;
	time_t	yyHour;
	time_t	yyMinutes;
	time_t	yyMonth;
	time_t	yySeconds;
	time_t	yyYear;
	MERIDIAN	yyMeridian;
	time_t	yyRelMonth;
	time_t	yyRelSeconds;
};

%}

%pure-parser
%lex-param   {char **input}
%lex-param   {LANGTABLE *lt}
%parse-param {char **input}
%parse-param {LANGTABLE *lt}
%parse-param {struct yyglobalvars *yyvars}

%union {
    time_t		Number;
    enum _MERIDIAN	Meridian;
}

%token	tAGO tDAY tDAYZONE tID tMERIDIAN tMINUTE_UNIT tMONTH tMONTH_UNIT
%token	tSEC_UNIT tSNUMBER tUNUMBER tZONE tDST

%type	<Number>	tDAY tDAYZONE tMINUTE_UNIT tMONTH tMONTH_UNIT
%type	<Number>	tSEC_UNIT tSNUMBER tUNUMBER tZONE
%type	<Meridian>	tMERIDIAN o_merid

%%

spec	: /* NULL */
	| spec item
	;

item	: time {
	    yyvars->yyHaveTime++;
	}
	| zone {
	    yyvars->yyHaveZone++;
	}
	| date {
	    yyvars->yyHaveDate++;
	}
	| day {
	    yyvars->yyHaveDay++;
	}
	| rel {
	    yyvars->yyHaveRel++;
	}
	| cvsstamp {
	    yyvars->yyHaveTime++;
	    yyvars->yyHaveDate++;
	    yyvars->yyHaveZone++;
	}
	| number
	;

cvsstamp: tUNUMBER '.' tUNUMBER '.' tUNUMBER '.' tUNUMBER '.' tUNUMBER '.' tUNUMBER {
	    yyvars->yyYear = $1;
	    if (yyvars->yyYear < 100) yyvars->yyYear += 1900;
	    yyvars->yyMonth = $3;
	    yyvars->yyDay = $5;
	    yyvars->yyHour = $7;
	    yyvars->yyMinutes = $9;
	    yyvars->yySeconds = $11;
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = 0;
	}
	;

time	: tUNUMBER tMERIDIAN {
	    yyvars->yyHour = $1;
	    yyvars->yyMinutes = 0;
	    yyvars->yySeconds = 0;
	    yyvars->yyMeridian = $2;
	}
	| tUNUMBER ':' tUNUMBER o_merid {
	    yyvars->yyHour = $1;
	    yyvars->yyMinutes = $3;
	    yyvars->yySeconds = 0;
	    yyvars->yyMeridian = $4;
	}
	| tUNUMBER ':' tUNUMBER tSNUMBER {
	    yyvars->yyHour = $1;
	    yyvars->yyMinutes = $3;
	    yyvars->yyMeridian = MER24;
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = - ($4 % 100 + ($4 / 100) * 60);
	}
	| tUNUMBER ':' tUNUMBER ':' tUNUMBER o_merid {
	    yyvars->yyHour = $1;
	    yyvars->yyMinutes = $3;
	    yyvars->yySeconds = $5;
	    yyvars->yyMeridian = $6;
	}
	| tUNUMBER ':' tUNUMBER ':' tUNUMBER tSNUMBER {
	    yyvars->yyHour = $1;
	    yyvars->yyMinutes = $3;
	    yyvars->yySeconds = $5;
	    yyvars->yyMeridian = MER24;
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = - ($6 % 100 + ($6 / 100) * 60);
	}
	;

zone	: tZONE {
	    yyvars->yyTimezone = $1;
	    yyvars->yyDSTmode = DSToff;
	}
	| tDAYZONE {
	    yyvars->yyTimezone = $1;
	    yyvars->yyDSTmode = DSTon;
	}
	|
	  tZONE tDST {
	    yyvars->yyTimezone = $1;
	    yyvars->yyDSTmode = DSTon;
	}
	;

day	: tDAY {
	    yyvars->yyDayOrdinal = 1;
	    yyvars->yyDayNumber = $1;
	}
	| tDAY ',' {
	    yyvars->yyDayOrdinal = 1;
	    yyvars->yyDayNumber = $1;
	}
	| tUNUMBER tDAY {
	    yyvars->yyDayOrdinal = $1;
	    yyvars->yyDayNumber = $2;
	}
	;

date	: tUNUMBER '/' tUNUMBER {
	    yyvars->yyMonth = $1;
	    yyvars->yyDay = $3;
	}
	| tUNUMBER '/' tUNUMBER '/' tUNUMBER {
	    if ($1 >= 100) {
		yyvars->yyYear = $1;
		yyvars->yyMonth = $3;
		yyvars->yyDay = $5;
	    } else {
		yyvars->yyMonth = $1;
		yyvars->yyDay = $3;
		yyvars->yyYear = $5;
	    }
	}
	| tUNUMBER tSNUMBER tSNUMBER {
	    /* ISO 8601 format.  yyyy-mm-dd.  */
	    yyvars->yyYear = $1;
	    yyvars->yyMonth = -$2;
	    yyvars->yyDay = -$3;
	}
	| tUNUMBER tMONTH tSNUMBER {
	    /* e.g. 17-JUN-1992.  */
	    yyvars->yyDay = $1;
	    yyvars->yyMonth = $2;
	    yyvars->yyYear = -$3;
	}
	| tMONTH tUNUMBER {
	    yyvars->yyMonth = $1;
	    yyvars->yyDay = $2;
	}
	| tMONTH tUNUMBER ',' tUNUMBER {
	    yyvars->yyMonth = $1;
	    yyvars->yyDay = $2;
	    yyvars->yyYear = $4;
	}
	| tUNUMBER tMONTH {
	    yyvars->yyMonth = $2;
	    yyvars->yyDay = $1;
	}
	| tUNUMBER tMONTH tUNUMBER {
	    yyvars->yyMonth = $2;
	    yyvars->yyDay = $1;
	    yyvars->yyYear = $3;
	}
	;

rel	: relunit tAGO {
	    yyvars->yyRelSeconds = -yyvars->yyRelSeconds;
	    yyvars->yyRelMonth = -yyvars->yyRelMonth;
	}
	| relunit
	;

relunit	: tUNUMBER tMINUTE_UNIT {
	    yyvars->yyRelSeconds += $1 * $2 * 60L;
	}
	| tSNUMBER tMINUTE_UNIT {
	    yyvars->yyRelSeconds += $1 * $2 * 60L;
	}
	| tMINUTE_UNIT {
	    yyvars->yyRelSeconds += $1 * 60L;
	}
	| tSNUMBER tSEC_UNIT {
	    yyvars->yyRelSeconds += $1;
	}
	| tUNUMBER tSEC_UNIT {
	    yyvars->yyRelSeconds += $1;
	}
	| tSEC_UNIT {
	    yyvars->yyRelSeconds++;
	}
	| tSNUMBER tMONTH_UNIT {
	    yyvars->yyRelMonth += $1 * $2;
	}
	| tUNUMBER tMONTH_UNIT {
	    yyvars->yyRelMonth += $1 * $2;
	}
	| tMONTH_UNIT {
	    yyvars->yyRelMonth += $1;
	}
	;

number	: tUNUMBER {
	    if (yyvars->yyHaveTime && yyvars->yyHaveDate && !yyvars->yyHaveRel)
		yyvars->yyYear = $1;
	    else {
		if($1>10000) {
		    yyvars->yyHaveDate++;
		    yyvars->yyDay= ($1)%100;
		    yyvars->yyMonth= ($1/100)%100;
		    yyvars->yyYear = $1/10000;
		}
		else {
		    yyvars->yyHaveTime++;
		    if ($1 < 100) {
			yyvars->yyHour = $1;
			yyvars->yyMinutes = 0;
		    }
		    else {
		    	yyvars->yyHour = $1 / 100;
		    	yyvars->yyMinutes = $1 % 100;
		    }
		    yyvars->yySeconds = 0;
		    yyvars->yyMeridian = MER24;
	        }
	    }
	}
	;

o_merid	: /* NULL */ {
	    $$ = MER24;
	}
	| tMERIDIAN {
	    $$ = $1;
	}
	;

%%


/* ARGSUSED */
static int
yyerror(s)
    char	*s;
{
  return 0;
}


static time_t
ToSeconds(Hours, Minutes, Seconds, Meridian)
    time_t	Hours;
    time_t	Minutes;
    time_t	Seconds;
    MERIDIAN	Meridian;
{
    if (Minutes < 0 || Minutes > 59 || Seconds < 0 || Seconds > 59)
	return -1;
    switch (Meridian) {
    case MER24:
	if (Hours < 0 || Hours > 23)
	    return -1;
	return (Hours * 60L + Minutes) * 60L + Seconds;
    case MERam:
	if (Hours < 1 || Hours > 12)
	    return -1;
	if (Hours == 12)
	    Hours = 0;
	return (Hours * 60L + Minutes) * 60L + Seconds;
    case MERpm:
	if (Hours < 1 || Hours > 12)
	    return -1;
	if (Hours == 12)
	    Hours = 0;
	return ((Hours + 12) * 60L + Minutes) * 60L + Seconds;
    default:
	abort ();
    }
    /* NOTREACHED */
}


/* Year is either
   * A negative number, which means to use its absolute value (why?)
   * A number from 0 to 99, which means a year from 1900 to 1999, or
   * The actual year (>=100).  */
static time_t
Convert(Month, Day, Year, Hours, Minutes, Seconds, Meridian, DSTmode, yyvars)
    time_t	Month;
    time_t	Day;
    time_t	Year;
    time_t	Hours;
    time_t	Minutes;
    time_t	Seconds;
    MERIDIAN	Meridian;
    DSTMODE	DSTmode;
    struct yyglobalvars *yyvars;
{
    static int DaysInMonth[12] = {
	31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    time_t	tod;
    time_t	Julian;
    int		i;

    if (Year < 0)
	Year = -Year;
    if (Year < 69)
	Year += 2000;
    else if (Year < 100)
	Year += 1900;
    DaysInMonth[1] = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0)
		    ? 29 : 28;
    /* Checking for 2038 bogusly assumes that time_t is 32 bits.  But
       I'm too lazy to try to check for time_t overflow in another way.  */
    if (Year < EPOCH || Year > 2038
     || Month < 1 || Month > 12
     /* Lint fluff:  "conversion from long may lose accuracy" */
     || Day < 1 || Day > DaysInMonth[(int)--Month])
	/* FIXME:
	 * It would be nice to set a global error string here.
	 * "February 30 is not a valid date" is much more informative than
	 * "Can't parse date/time: 100 months" when the user input was
	 * "100 months" and addition resolved that to February 30, for
	 * example.  See rcs2-7 in src/sanity.sh for more. */
	return -1;

    for (Julian = Day - 1, i = 0; i < Month; i++)
	Julian += DaysInMonth[i];
    for (i = EPOCH; i < Year; i++)
	Julian += 365 + (i % 4 == 0);
    Julian *= SECSPERDAY;
    Julian += yyvars->yyTimezone * 60L;
    if ((tod = ToSeconds(Hours, Minutes, Seconds, Meridian)) < 0)
	return -1;
    Julian += tod;
    if (DSTmode == DSTon
     || (DSTmode == DSTmaybe && localtime(&Julian)->tm_isdst))
	Julian -= 60 * 60;
    return Julian;
}


static time_t
DSTcorrect(Start, Future)
    time_t	Start;
    time_t	Future;
{
    time_t	StartDay;
    time_t	FutureDay;

    StartDay = (localtime(&Start)->tm_hour + 1) % 24;
    FutureDay = (localtime(&Future)->tm_hour + 1) % 24;
    return (Future - Start) + (StartDay - FutureDay) * 60L * 60L;
}


static time_t
RelativeDate(Start, DayOrdinal, DayNumber)
    time_t	Start;
    time_t	DayOrdinal;
    time_t	DayNumber;
{
    struct tm	*tm;
    time_t	now;

    now = Start;
    tm = localtime(&now);
    now += SECSPERDAY * ((DayNumber - tm->tm_wday + 7) % 7);
    now += 7 * SECSPERDAY * (DayOrdinal <= 0 ? DayOrdinal : DayOrdinal - 1);
    return DSTcorrect(Start, now);
}


static time_t
RelativeMonth(Start, RelMonth, yyvars)
    time_t	Start;
    time_t	RelMonth;
    struct yyglobalvars *yyvars;
{
    struct tm	*tm;
    time_t	Month;
    time_t	Year;

    if (RelMonth == 0)
	return 0;
    tm = localtime(&Start);
    Month = 12 * (tm->tm_year + 1900) + tm->tm_mon + RelMonth;
    Year = Month / 12;
    Month = Month % 12 + 1;
    return DSTcorrect(Start,
	    Convert(Month, (time_t)tm->tm_mday, Year,
		(time_t)tm->tm_hour, (time_t)tm->tm_min, (time_t)tm->tm_sec,
		MER24, DSTmaybe, yyvars));
}


static int
LookupWord(buff, lt, yylval)
    char		*buff;
    LANGTABLE		*lt;
    YYSTYPE		*yylval;
{
    register char	*p;
    register char	*q;
    register const TABLE	*tp;
    int			i;
    int			abbrev;

    /* Make it lowercase. */
    for (p = buff; *p; p++)
	if (isupper(*p))
	    *p = tolower(*p);

    if (strcmp(buff, "am") == 0 || strcmp(buff, "a.m.") == 0) {
	yylval->Meridian = MERam;
	return tMERIDIAN;
    }
    if (strcmp(buff, "pm") == 0 || strcmp(buff, "p.m.") == 0) {
	yylval->Meridian = MERpm;
	return tMERIDIAN;
    }

    /* See if we have an abbreviation for a month. */
    if (strlen(buff) == 3)
	abbrev = 1;
    else if (strlen(buff) == 4 && buff[3] == '.') {
	abbrev = 1;
	buff[3] = '\0';
    }
    else
	abbrev = 0;

    for (tp = lt->MonthDayTable; tp->name; tp++) {
	if (abbrev) {
	    if (strncmp(buff, tp->name, 3) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
	}
	else if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}
    }

    for (tp = lt->TimezoneTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    if (strcmp(buff, "dst") == 0) 
	return tDST;

    for (tp = lt->UnitsTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    /* Strip off any plural and try the units table again. */
    /* XXX: Support more than just enligsh plural. */
    i = strlen(buff) - 1;
    if (buff[i] == 's') {
	buff[i] = '\0';
	for (tp = lt->UnitsTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
	buff[i] = 's';		/* Put back for "this" in OtherTable. */
    }

    for (tp = lt->OtherTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    /* Military timezones. */
    if (buff[1] == '\0' && isalpha(*buff)) {
	for (tp = lt->MilitaryTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
    }

    /* Drop out any periods and try the timezone table again. */
    for (i = 0, p = q = buff; *q; q++)
	if (*q != '.')
	    *p++ = *q;
	else
	    i++;
    *p = '\0';
    if (i)
	for (tp = lt->TimezoneTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }

    return tID;
}


static int
yylex(YYSTYPE *yylval, char **input, LANGTABLE *lt)
{
    register char	c;
    register char	*p;
    char		buff[20];
    int			Count;
    int			sign;
    char		*yyInput;

    yyInput = *input;
    for ( ; ; ) {
	while (isspace(*yyInput))
	    yyInput++;

	if (isdigit(c = *yyInput) || c == '-' || c == '+') {
	    if (c == '-' || c == '+') {
		sign = c == '-' ? -1 : 1;
		if (!isdigit(*++yyInput))
		    /* skip the '-' sign */
		    continue;
	    }
	    else
		sign = 0;
	    for (yylval->Number = 0; isdigit(c = *yyInput++); )
		yylval->Number = 10 * yylval->Number + c - '0';
	    yyInput--;
	    if (sign < 0)
		yylval->Number = -yylval->Number;
            *input = yyInput;
	    return sign ? tSNUMBER : tUNUMBER;
	}
	if (isalpha(c)) {
	    for (p = buff; isalpha(c = *yyInput++) || c == '.'; )
		if (p < &buff[sizeof buff - 1])
		    *p++ = c;
	    *p = '\0';
	    yyInput--;
            *input = yyInput;
	    return LookupWord(buff, lt, yylval);
	}
	if (c != '(') {
            *input = yyInput;
	    return *yyInput++;
	}
	Count = 0;
	do {
	    c = *yyInput++;
	    if (c == '\0') {
                *input = yyInput;
		return c;
	    }
	    if (c == '(') {
		Count++;
	    }
	    else if (c == ')') {
		Count--;
	    }
	} while (Count > 0);
    }
}

#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (a, b)
     struct tm *a, *b;
{
  int ay = a->tm_year + (TM_YEAR_ORIGIN - 1);
  int by = b->tm_year + (TM_YEAR_ORIGIN - 1);
  int days = (
	      /* difference in day of year */
	      a->tm_yday - b->tm_yday
	      /* + intervening leap days */
	      +  ((ay >> 2) - (by >> 2))
	      -  (ay/100 - by/100)
	      +  ((ay/100 >> 2) - (by/100 >> 2))
	      /* + difference in years * 365 */
	      +  (long)(ay-by) * 365
	      );
  return (60*(60*(24*days + (a->tm_hour - b->tm_hour))
	      + (a->tm_min - b->tm_min))
	  + (a->tm_sec - b->tm_sec));
}

static time_t
_get_date(p, now, language)
    char		*p;
    struct timeb	*now;
    int			language;
{
    struct tm		*tm, gmt;
    struct timeb	ftz;
    time_t		Start;
    time_t		tod;
    time_t		nowtime;
    LANGTABLE		*langtbl;
    struct yyglobalvars yyvars;

    langtbl = lookup_language(language);
    if (langtbl == NULL)
	    return -1;

    if (now == NULL) {
	struct tm *gmt_ptr;

        now = &ftz;
	(void)time (&nowtime);

	gmt_ptr = gmtime (&nowtime);
	if (gmt_ptr != NULL)
	{
	    /* Make a copy, in case localtime modifies *tm (I think
	       that comment now applies to *gmt_ptr, but I am too
	       lazy to dig into how gmtime and locatime allocate the
	       structures they return pointers to).  */
	    gmt = *gmt_ptr;
	}

	if (! (tm = localtime (&nowtime)))
	    return -1;

	if (gmt_ptr != NULL)
	    ftz.timezone = difftm (&gmt, tm) / 60;
	else
	    /* We are on a system like VMS, where the system clock is
	       in local time and the system has no concept of timezones.
	       Hopefully we can fake this out (for the case in which the
	       user specifies no timezone) by just saying the timezone
	       is zero.  */
	    ftz.timezone = 0;

	if(tm->tm_isdst)
	    ftz.timezone += 60;
    }
    else
    {
	nowtime = now->time;
    }

    tm = localtime(&nowtime);
    yyvars.yyYear = tm->tm_year + 1900;
    yyvars.yyMonth = tm->tm_mon + 1;
    yyvars.yyDay = tm->tm_mday;
    yyvars.yyTimezone = now->timezone;
    yyvars.yyDSTmode = DSTmaybe;
    yyvars.yyHour = 0;
    yyvars.yyMinutes = 0;
    yyvars.yySeconds = 0;
    yyvars.yyMeridian = MER24;
    yyvars.yyRelSeconds = 0;
    yyvars.yyRelMonth = 0;
    yyvars.yyHaveDate = 0;
    yyvars.yyHaveDay = 0;
    yyvars.yyHaveRel = 0;
    yyvars.yyHaveTime = 0;
    yyvars.yyHaveZone = 0;

    if (yyparse(&p, langtbl, &yyvars)
     || yyvars.yyHaveTime > 1 || yyvars.yyHaveZone > 1 || yyvars.yyHaveDate > 1 || yyvars.yyHaveDay > 1)
	return -1;

    if (yyvars.yyHaveDate || yyvars.yyHaveTime || yyvars.yyHaveDay) {
	Start = Convert(yyvars.yyMonth, yyvars.yyDay, yyvars.yyYear, yyvars.yyHour, yyvars.yyMinutes,
	                yyvars.yySeconds, yyvars.yyMeridian, yyvars.yyDSTmode, &yyvars);
	if (Start < 0)
	    return -1;
    }
    else {
	Start = nowtime;
	if (!yyvars.yyHaveRel)
	    Start -= ((tm->tm_hour * 60L + tm->tm_min) * 60L) + tm->tm_sec;
    }

    Start += yyvars.yyRelSeconds;
    Start += RelativeMonth(Start, yyvars.yyRelMonth, &yyvars);

    if (yyvars.yyHaveDay && !yyvars.yyHaveDate) {
	tod = RelativeDate(Start, yyvars.yyDayOrdinal, yyvars.yyDayNumber, &yyvars);
	Start += tod;
    }

    /* Have to do *something* with a legitimate -1 so it's distinguishable
     * from the error return value.  (Alternately could set errno on error.) */
    return Start == -1 ? 0 : Start;
}

time_t
get_date(char *timebuf, struct timeb *tb, int lang, struct timerange *tr)
{
	char *buf, *p;
	time_t starttime, endtime;

	buf = malloc(strlen(timebuf) + 1);
	strcpy(buf, timebuf);

	p = index(buf, '-');
	if (p == NULL || p >= timebuf + strlen(timebuf) - 1) {
		starttime = _get_date(timebuf, tb, lang);
		if (tr != NULL)
			tr->start = tr->end = starttime;
		free(buf);
		return tr->start;
	}
	*p = '\0';
	p++;

	starttime = _get_date(buf, tb, lang);
	endtime = _get_date(p, tb, lang);
	free(buf);
	if (starttime == -1 || endtime == -1)
		return -1;
	
	if (tr != NULL) {
		tr->start = starttime;
		tr->end = endtime;
	}

	return endtime - starttime;
}

/* XXX: Remove this */
void
init_getdate(void)
{

#define add_language_item(suffix, name) do {\
	languagetable[__CONCAT(LANGUAGE_, suffix)].name = __CONCAT(name, suffix);\
} while(0)
#define add_language(suffix) do {\
		add_language_item(suffix, MonthDayTable); \
		add_language_item(suffix, UnitsTable); \
		add_language_item(suffix, OtherTable); \
		add_language_item(suffix, TimezoneTable); \
		add_language_item(suffix, MilitaryTable); \
	} while (0)

	add_language(EN);
	add_language(NO);

#undef add_language
#undef add_language_item
}


#if	defined(TEST)

/* ARGSUSED */
int
main(ac, av)
    int		ac;
    char	*av[];
{
    char	buff[128];
    time_t	d;
    int		language;
    struct timerange tr;

    init_getdate();
    printf("Enter language to use: ");
    fgets(buff, sizeof(buff), stdin);
    language = atoi(buff);
    fflush(stdout);
    printf("Enter date, or blank line to exit. Separate dates with '-'\n\t> ");
    fflush(stdout);
    while (fgets(buff, sizeof(buff), stdin) && buff[0]) {
	d = get_date(buff, NULL, language, &tr);
	if (d == -1) {
	    printf("Bad format - couldn't convert.\n");
	} else {
	    printf("> %s", ctime(&tr.start));
	    printf("> %s", ctime(&tr.end));
	    printf("%s", ctime(&d));
	}

	printf("\t> ");
	fflush(stdout);
    }
    return(0);
}
#endif	/* defined(TEST) */
