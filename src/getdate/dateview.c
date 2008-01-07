
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "dateview.h"
#include "getdate.h"


int
date_info_start(dateview *dv, time_t start, time_t end)
{
	enum dateview_output_type type = TWO_YEARS_PLUS;

	dv->output = malloc(sizeof(int) * type);
	if (dv->output == NULL)
		return -1;
	memset(dv->output, 0, sizeof(int) * type);
	dv->start = start;
	dv->end = end;

	return 0;
}

time_t getdate_dateview(enum dateview_output_type);


int
date_info_add(dateview *dv, time_t checktime)
{
	time_t today, yesterday, this_week, this_month, this_year, last_year;
	time_t nowtime;
	struct tm tm;

#if 0
	time(&nowtime);
	gmtime_r(&nowtime, &tm);
	tm.tm_sec = 0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	today = mktime(&tm);
	yesterday = today - (24 * 60 * 60);
	last_week = today - (7 * 24 * 60 * 60);
	last_month = today - (31 * 24 * 60 * 60); /* Not completely correct */
	tm.tm_mday = 1;
	tm.tm_mon = 0;
	this_year = mktime(&tm);
	tm.tm_year -= 1;
	last_year = mktime(&tm);
#else
	today = getdate_dateview(TODAY);
	yesterday = getdate_dateview(YESTERDAY);
	this_week = getdate_dateview(THIS_WEEK);
	this_month = getdate_dateview(THIS_MONTH);
	this_year = getdate_dateview(THIS_YEAR);
	last_year = getdate_dateview(LAST_YEAR);

#endif

	if (checktime >= today) {
		dv->output[TODAY-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_WEEK-1] += 1;
	} else if (checktime >= yesterday) {
		dv->output[YESTERDAY-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_WEEK-1] += 1;
	} else if (checktime >= this_week) {
		dv->output[THIS_WEEK-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
	} else if (checktime >= this_month) {
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
	} else if (checktime >= this_year) {
		dv->output[THIS_YEAR-1] += 1;
	} else if (checktime >= last_year) {
		dv->output[LAST_YEAR-1] += 1;
	} else {
		dv->output[TWO_YEARS_PLUS-1] += 1;
	}

	return 0;
}

void
date_info_free(dateview *dv)
{
	free(dv->output);
}

dateview *
date_info_end(dateview *dv)
{
	return dv;
}

#ifdef TEST_DATEVIEW

#include <stdio.h>

int
main(void)
{
	dateview dv;
	dateview *dv2;
	enum dateview_output_type type = TWO_YEARS_PLUS;
	unsigned int i;
	time_t date;

#define get_date(x) get_date(x, NULL, 0)

	//init_getdate();
	date_info_start(&dv, 0, -1);

#if 1
	/*
	date_info_add(&dv, get_date("last week"));
	date_info_add(&dv, get_date("2 years ago"));
	date_info_add(&dv, get_date("last week"));
	date_info_add(&dv, get_date("2 weeks ago"));

	date_info_add(&dv, get_date("today"));
	date_info_add(&dv, get_date("yesterday"));
	date_info_add(&dv, get_date("today 5 min ago"));
	date_info_add(&dv, get_date("2 weeks ago"));

	date_info_add(&dv, get_date("3 weeks ago"));
	date_info_add(&dv, get_date("last week"));
	date_info_add(&dv, get_date("last year"));
	date_info_add(&dv, get_date("2 year ago"));

	date_info_add(&dv, get_date("last year"));
	date_info_add(&dv, get_date("4 years ago"));
	*/
#else
	/*
	date_info_add(&dv, get_date("1 year 3 week ago"));
	date_info_add(&dv, get_date("1 year 3 week ago"));
	date_info_add(&dv, get_date("1 year 2 week ago"));
	date_info_add(&dv, get_date("1 year 4 week ago"));
	date_info_add(&dv, get_date("1 year 2 week ago"));
	date_info_add(&dv, get_date("1 year 5 week ago"));
	*/
#endif

	date = 1141239390;
	printf("%s",ctime(&date));	
	date_info_add(&dv, date);

	date = 1071945355;
	printf("%s",ctime(&date));	
	date_info_add(&dv, date);

	date = 1161882999;
	printf("%s",ctime(&date));	
	date_info_add(&dv, date);

	date = 1107718883;
	printf("%s",ctime(&date));	
	date_info_add(&dv, date);

	date = 1164224319;
	printf("%s",ctime(&date));	
	date_info_add(&dv, date);


	/* Not needed right now */
	dv2 = date_info_end(&dv);

	for (i = 0; i < type; i++) {
		printf("Type: %d Count: %d\n", i, dv2->output[i]);
	}

#if 0
        TODAY,
        YESTERDAY,
        LAST_WEEK,
        LAST_MONTH,
        THIS_YEAR,
        LAST_YEAR,
        TWO_YEARS_PLUS,
#endif

	date_info_free(dv2);

	return 0;
}

#endif /* TEST_DATEVIEW */
