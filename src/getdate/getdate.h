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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     unknown = 258,
     NUMBER = 259,
    // WORD = 260,
     DAY = 261,
     WEEK = 262,
     MONTH = 263,
     YEAR = 264
   };
#endif
/* Tokens.  */
#define unknown 258
#define NUMBER 259
#define WORD 260
#define DAY 261
#define WEEK 262
#define MONTH 263
#define YEAR 264


struct datelib {
        time_t start;
        time_t end;

        struct tm tmstart;

        struct {
                int year, month, week, day;
        } modify;

        enum yytokentype lowest;
};

