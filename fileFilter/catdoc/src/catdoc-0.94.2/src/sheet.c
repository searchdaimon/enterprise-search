/*****************************************************************/
/* Representation and handling of Excell worksheets in memory    */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 1998-2003, (c) Alex Ott 2003	             */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "xls.h"
struct rowdescr *rowptr=NULL;
int startrow=0,lastrow=0;
char cell_separator = ',';
int quote_mode = QUOTE_ALL_STRINGS;
char *sheet_separator = "\f";
/* 
 * Allocates cell for given row and col and returns pointer to poitrer
 * of cell contents
 */ 
unsigned char **allocate (int row,int col) {
	unsigned int newrow,newcol;
	if (row>=lastrow) {
		newrow=(row/16+1)*16;
		rowptr=realloc(rowptr,newrow*sizeof(struct rowdescr));
		if (rowptr == NULL) {
			perror("allocating sheet ");
			exit(1);
		}	
		memset(rowptr+lastrow,0,(newrow-lastrow)*sizeof(struct rowdescr));
		lastrow=newrow;
	}
	if (col>=rowptr[row].end) {
		newcol=(col/16+1)*16;
		rowptr[row].cells=realloc(rowptr[row].cells,newcol*sizeof(char *));
		if (rowptr[row].cells == NULL) {
			perror("allocating row");
			exit(1);
		}	
		memset(rowptr[row].cells+rowptr[row].end,0,(newcol-rowptr[row].end)
				*sizeof(char *));
		rowptr[row].end=newcol;
	}  
	if (col>rowptr[row].last) rowptr[row].last=col;
	return (rowptr[row].cells+col);
}
/*
 * Frees up all memory used by sheet
 */ 
void free_sheet(void) {
	int i,j;
	struct rowdescr *row;
	unsigned char **col;
	for (row=rowptr,i=0;i<lastrow;i++,row++) {
		if (!row->cells) continue;
		for (col=row->cells,j=0;j<row->end;j++,col++) {
			if (*col) {
				free(*col);
			}
		}
		free (row->cells);
	}
	free (rowptr);
	rowptr=NULL;
	lastrow=0;
}

/*
 * prints out one value with quoting
 * uses global variable quote_mode
 */ 
void print_value(unsigned char *value) 
{
	int i,len;
	int quotes=0;
	if (value != NULL) {
		len=strlen((char *)value);
	} else {
		len = 0;
	}
	switch (quote_mode) {
		case QUOTE_NEVER:
			break;
		case QUOTE_SPACES_ONLY:	  
			for (i=0;i<len;i++) {
				if (isspace(value[i]) || value[i]==cell_separator || 
						value[i]=='"') {
					quotes=1;
					break;
				}
			}    
			break;
		case QUOTE_ALL_STRINGS:
			{ char *endptr;
			  strtod(value,&endptr);
			  quotes=(*endptr != '0');
			break;
			}  
		case QUOTE_EVERYTHING:
			quotes = 1;
			break;     
	}  	  
	if (quotes) {
		fputc('\"',stdout);
		for (i=0;i<len;i++) {
			if (value[i]=='\"') {
				fputc('\"',stdout);
				fputc('\"',stdout);
			} else {
				fputc(value[i],stdout);
			}
		}   
		fputc('\"',stdout);
	} else {
		fputs((char *)value,stdout);
	}
}    
/*
 * Prints sheet to stdout. Uses global variable cell_separator
 */ 
void print_sheet(void) {
	int i,j,printed=0;
	struct rowdescr *row;
	unsigned char **col;
	lastrow--;
	while (lastrow>0&&!rowptr[lastrow].cells) lastrow--;
	for(i=0,row=rowptr;i<=lastrow;i++,row++) {
		if (row->cells) {
			for (j=0,col=row->cells;j<=row->last;j++,col++) {
				if (j){
					fputc(cell_separator,stdout);
					printed=1;
				}
				if (*col) {
					print_value(*col);
					printed=1;
				}
			}
			if (printed) {
				fputc('\n',stdout);
				printed=0;
			}
		}
	}
	fputs(sheet_separator,stdout);
}
