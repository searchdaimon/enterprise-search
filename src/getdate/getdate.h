#ifndef _GETDATE_H_
#define _GETDATE_H_

#include <time.h>

//Runarb: Ser ikke ut til å bruke dette?
//int state;
//union datedata {
//	int number;
//	char *foo;
//} datedata;
//
//#define ANUMBER 1
//#define TNUMBER 2
//#define WORD 3

struct datelib {
        time_t start;
        time_t end;

        struct tm tmstart;

        struct {
                int year, month, week, day;
        } modify;

#ifdef __INGETDATE
        enum yytokentype lowest;
#else
	int __padding1;
#endif
};


int getdate(char *, struct datelib *);

#endif /* _GETDATE_H_ */
