/*****************************************************************/
/* Definition specific for Excel file handling                   */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 2003, (c) Alex Ott 2003	                 */
/*****************************************************************/

#ifndef XLS_H
#define XLS_H

#include <stdio.h>
#include <math.h>
/* types of quoting */
#define QUOTE_NEVER 0
#define QUOTE_SPACES_ONLY 1
#define QUOTE_ALL_STRINGS 2
#define QUOTE_EVERYTHING 3

struct rowdescr {
	int last, end;
	unsigned char **cells;
};	
/* structure to hold parsed table */
extern struct rowdescr *rowptr;
extern int startrow;
/* xls2csv-specific configuration */
extern char cell_separator;
extern int quote_mode;
void print_sheet(void);
void free_sheet(void);
void print_value(unsigned char *value);
char *format_double(char *rec,int offset,int format_code);
char *format_int(int value,int format_code);
char *format_rk(char *rec,short int format_code);
char *gettypename(long rectype);
void parse_sst(char *sstbuf,int bufsize);
void process_item (int rectype, int reclen, char *rec); 
unsigned char **allocate(int row,int col);
char *copy_unicode_string(unsigned char **src);
char convert8to8(char *src,int count);
char *convert16to8(char *src,int count);
void do_table(FILE *input,char *filename);
char *mark_string(char *instr);

/* ---- added by Igor ---- */
void SetFormatIdxUsed(int format_code);
/* -- end added by Igor -- */


#ifdef __TURBOC__
#define rint(x) floor((x+0.5))
#endif
#endif
