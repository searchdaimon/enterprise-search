
#include <time.h>
#include <stdlib.h>

#include "dateview.h"

#ifdef TEST_DATEVIEW
	#include "getdate.h"
#endif


void
date_info_start(dateview *dv, time_t start, time_t end)
{
	dv->head = NULL;
	dv->tail = NULL;
	dv->start = start;
	dv->end = end;
}

static void
date_info_insert(dateview_item *dvi, void *obj, time_t time,
                 dateview_item *prev, dateview_item *next)
{
	dvi->obj = obj;
	dvi->time = time;
	dvi->next = next;
	dvi->prev = prev;
}

int
date_info_add(dateview *dv, void *obj, time_t time)
{
	dateview_item *dvi, *p, *last;

	/* Skip it if it is not within the wanted range */
	if ((dv->start != -1 && time < dv->start) || (dv->end != -1 && time > dv->end))
		return 1;

	dvi = malloc(sizeof(dateview_item));
	if (dvi == NULL)
		return -1;

	if (dv->head == NULL) {
		dv->head = dvi;
		date_info_insert(dv->head, obj, time, NULL, NULL);
		dv->tail = dvi;
		return 0;
	}

	for (p = dv->head, last = NULL; p && p->time <= time; p = p->next)
		last = p;

	if (!last) {
		date_info_insert(dvi, obj, time, NULL, dv->head);
		dv->head->prev = dvi;
		dv->head = dvi;
	}
	else {
		date_info_insert(dvi, obj, time, last, p);
		if (last->next)
			last->next->prev = dvi;
		last->next = dvi;
		if (p == NULL)
			dv->tail = dvi;
	}

	return 0;
}

static dateview_output *
date_info_output_get(dateview_item **dvihead, time_t until, enum dateview_output_type type, dateview *dv)
{
	dateview_output *dvo;
	dateview_item *dvi;
	int c;

	dvo = malloc(sizeof(dateview_output));
	if (dvo == NULL)
		return NULL;
	dvo->next = NULL;


	for (c = 0, dvi = *dvihead; dvi->time >= until; c++, dvi = dvi->prev) {
		if (dvi == dv->head) {
			c++;
			dvi = NULL;
			break;
		}
	}
	dvo->type = type;
	dvo->length = c;
	dvo->head = *dvihead;

	*dvihead = dvi;

	return dvo;
}

/* XXX: Make this a function! */
#define date_info_output_get_wrap(_time, _type, _typenum) \
	do {\
		dvo = date_info_output_get(&dvihead, _time, _type, dv);\
		if (dvo->length > 0) {\
			if (dvohead == NULL) {\
				dvohead = dvo;\
				dvop = dvo;\
			}\
			else {\
				dvop->next = dvo;\
				dvop = dvo;\
			}\
			dvo->type_number = _typenum;\
			if (dvihead == NULL)\
				return dvohead;\
		}\
		else {\
			free(dvo);\
		}\
	} while(0)



static dateview_output *
dateview_range_all(dateview *dv)
{
	time_t today, yesterday, last_week, last_month, this_year, last_year;
	time_t yearago_t;
	time_t nowtime;
	int yearsago;
	struct tm tm;
	dateview_item *dvihead;
	dateview_output *dvo, *dvohead = NULL, *dvop;

	time(&nowtime);
	gmtime_r(&nowtime, &tm);
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	today = mktime(&tm);
	yesterday = today - (24 * 60 * 60);
	last_week = today - (7 * 24 * 60 * 60);
	last_month = today - (31 * 24 * 60 * 60);
	tm.tm_mday = 1;
	tm.tm_mon = 1;
	this_year = mktime(&tm);
	tm.tm_year -= 1;
	last_year = mktime(&tm);

	dvihead = dv->tail;
	date_info_output_get_wrap(today, TODAY, 0);
	date_info_output_get_wrap(yesterday, YESTERDAY, 0);
	date_info_output_get_wrap(last_week, LAST_WEEK, 0);
	date_info_output_get_wrap(last_month, LAST_MONTH, 0);
	date_info_output_get_wrap(this_year, THIS_YEAR, 0);
	date_info_output_get_wrap(last_year, LAST_YEAR, 0);

	/* The current year and the one before that is already been taken care off */
	yearsago = 2;
	while (dvihead != NULL) {
		tm.tm_year--;
		yearago_t = mktime(&tm);

		date_info_output_get_wrap(yearago_t, YEARS_AGO, yearsago);
		yearsago++;
	}

	return dvohead;
}

static dateview_output *
dateview_range_month(dateview *dv)
{
	dateview_output *dvo, *dvop, *dvohead = NULL;
	dateview_item *dvihead;

	dvihead = dv->tail;

	while (dvihead != NULL) {
		int week;
		struct tm tm;
		time_t time;
	
		gmtime_r(&(dvihead->time), &tm);
		week = (tm.tm_yday / 7) + 1;
		tm.tm_yday = week * 7;

		time = mktime(&tm);
		date_info_output_get_wrap(time, WEEK, week);
	}

	return dvohead;
}


static void
date_info_output_free(dateview_output *dvohead)
{
	dateview_output *dvo, *dvop;

	dvo = dvohead;
	while (dvo) {
		dvop = dvo;
		dvo = dvo->next;
		free(dvop);
	}
}

dateview_output *
date_info_end(dateview *dv)
{
	time_t nowtime;

	time(&nowtime);

	if ((dv->tail->time - (20 * 24 * 60 * 60)) > nowtime) {
		return(dateview_range_all(dv));
	}

	else if ((dv->head->time + (31 * 24 * 60 * 60)) > dv->tail->time)
		return(dateview_range_month(dv));
	

	return (dateview_range_all(dv));
}

#ifdef TEST_DATEVIEW

#include <stdio.h>


int
main(void)
{
	dateview dv;
	dateview_item *p = NULL;
	dateview_output *dvo, *dvohead;
#define get_date(x) get_date(x, NULL, 0)

	init_getdate();
	date_info_start(&dv, 0, -1);

#if 0
	date_info_add(&dv, NULL, get_date("last week"));
	date_info_add(&dv, NULL, get_date("2 years ago"));
	date_info_add(&dv, NULL, get_date("last week"));
	date_info_add(&dv, NULL, get_date("2 weeks ago"));

	date_info_add(&dv, NULL, get_date("today"));
	date_info_add(&dv, NULL, get_date("yesterday"));
	date_info_add(&dv, NULL, get_date("today 5 min ago"));
	date_info_add(&dv, NULL, get_date("2 weeks ago"));

	date_info_add(&dv, NULL, get_date("3 weeks ago"));
	date_info_add(&dv, NULL, get_date("last week"));
	date_info_add(&dv, NULL, get_date("last year"));
	date_info_add(&dv, NULL, get_date("2 year ago"));

	date_info_add(&dv, NULL, get_date("last year"));
	date_info_add(&dv, NULL, get_date("4 years ago"));
#else

	date_info_add(&dv, NULL, get_date("1 year 3 week ago"));
	date_info_add(&dv, NULL, get_date("1 year 3 week ago"));
	date_info_add(&dv, NULL, get_date("1 year 2 week ago"));
	date_info_add(&dv, NULL, get_date("1 year 4 week ago"));
	date_info_add(&dv, NULL, get_date("1 year 2 week ago"));
	date_info_add(&dv, NULL, get_date("1 year 5 week ago"));

#endif


	dvohead = dvo = date_info_end(&dv);

	for (; dvo != NULL; dvo = dvo->next) {
		printf("Matched: %d %d(%d)\n", dvo->length, dvo->type, dvo->type_number);
	}
	date_info_output_free(dvohead);

#if 1

#if DATEVIEW_BACKWARDS
	for (p = dv.tail; 1; p = p->prev) {
#else
	for (p = dv.head; p != NULL; p = p->next) {
#endif
		printf("Date info: time_t: %s", ctime(&p->time));
#if DATEVIEW_BACKWARDS
		if (p == dv.head)
			break;
#endif
	}
#endif

	return 0;
}

#endif
