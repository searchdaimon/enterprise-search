#ifndef _DATEVIEW_H_
#define _DATEVIEW_H_

#include <time.h>

typedef struct _dateview_item {
	struct _dateview_item *prev, *next;
	time_t time;
	void *obj;
} dateview_item;

typedef struct _dateview {
	int *output;
	time_t start, end;
} dateview;

enum dateview_output_type {
	TODAY = 1,
	YESTERDAY,
	LAST_WEEK,
	LAST_MONTH,
	THIS_YEAR,
	LAST_YEAR,
	TWO_YEARS_PLUS,
};

typedef struct _dateview_output {
	struct _dateview_output *next;

	enum dateview_output_type type;
	int type_number; /* Which month, which week, etc. */

	int length;
	dateview_item *head;
} dateview_output;


int date_info_start(dateview *, time_t, time_t);
int date_info_add(dateview *, time_t);
dateview *date_info_end(dateview *dv);
void date_info_free(dateview *dv);

#endif /* _DATEVIEW_H_ */
