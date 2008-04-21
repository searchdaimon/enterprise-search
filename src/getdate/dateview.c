
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "dateview.h"
#include "getdate.h"

time_t getdate_dateview(enum dateview_output_type);

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

	dv->today = getdate_dateview(TODAY);
	dv->yesterday = getdate_dateview(YESTERDAY);
	dv->this_week = getdate_dateview(THIS_WEEK);
	dv->this_month = getdate_dateview(THIS_MONTH);
	dv->this_year = getdate_dateview(THIS_YEAR);
	dv->last_year = getdate_dateview(LAST_YEAR);

	return 0;
}



int
date_info_add(dateview *dv, time_t checktime)
{
	struct tm tm;

	if (checktime >= dv->today) {
		dv->output[TODAY-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_WEEK-1] += 1;
	} else if (checktime >= dv->yesterday) {
		dv->output[YESTERDAY-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_WEEK-1] += 1;
	} else if (checktime >= dv->this_week) {
		dv->output[THIS_WEEK-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
		dv->output[THIS_MONTH-1] += 1;
	} else if (checktime >= dv->this_month) {
		dv->output[THIS_MONTH-1] += 1;
		dv->output[THIS_YEAR-1] += 1;
	} else if (checktime >= dv->this_year) {
		dv->output[THIS_YEAR-1] += 1;
	} else if (checktime >= dv->last_year) {
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

#if 0
#if 1
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
#else
	date_info_add(&dv, get_date("1 year 3 week ago"));
	date_info_add(&dv, get_date("1 year 3 week ago"));
	date_info_add(&dv, get_date("1 year 2 week ago"));
	date_info_add(&dv, get_date("1 year 4 week ago"));
	date_info_add(&dv, get_date("1 year 2 week ago"));
	date_info_add(&dv, get_date("1 year 5 week ago"));
#endif
#endif

#if 0
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

#endif


#if 0
	for (i = 0; i < type; i++) {
		printf("Type: %d Count: %d\n", i, dv2->output[i]);
	}
#endif

#if 0
        TODAY,
        YESTERDAY,
        THIS_WEEK,
        THIS_MONTH,
        THIS_YEAR,
        LAST_YEAR,
        TWO_YEARS_PLUS,
#endif

	/* Not needed right now */
	dv2 = date_info_end(&dv);


	date = 1164224319;
	for (i = 0; i < 1000000; i++) {
		date_info_add(&dv, date);
	}

	date_info_free(dv2);

	return 0;
}

#endif /* TEST_DATEVIEW */
