/*****************************************************************/
/* BIFF-stream (excel file) parsing                              */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) David Rysdam  1998                                        */
/* (c) Victor Wagner 1998-2003, (c) Alex Ott 2003	               */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "xls.h"
#include "catdoc.h"
#include "xltypes.h"
#include "float.h"
#include <math.h>
#include <time.h>
#ifndef HAVE_STRFTIME
#include "../compat/strftime.h"
#endif
static unsigned char rec[MAX_MS_RECSIZE];
int biff_version=0;
short int *formatTable=NULL;
char *forced_date_format = NULL;
size_t formatTableIndex = 0;
size_t formatTableSize = 0;
double date_shift = 25569.0;
#define FLT_FORMAT(a,b,c) a #b c
#define MK_FORMAT(x) FLT_FORMAT("%.",x,"g") 			
char number_format[8]=MK_FORMAT(DBL_DIG);

void CleanUpFormatIdxUsed(void);

void do_table(FILE *input,char *filename) {    
	long rectype;
	long reclen,build_year=0,build_rel=0,offset=0;
	int eof_flag=0;
	int itemsread=1;
	date_shift=25569.0; /* Windows 1900 date system */
	CleanUpFormatIdxUsed();
	while (itemsread) {
		catdoc_read(rec,2,1,input);
		biff_version=getshort(rec,0);
		catdoc_read(rec,2,1,input);
		reclen=getshort(rec,0);
		if ( biff_version == 0x0809 || biff_version == 0x0409 ||
				 biff_version == 0x0209 || biff_version == 0x0009 ) {
			if (reclen==8 || reclen==16) {
				if (biff_version == 0x0809 ) {
					itemsread=catdoc_read(rec,4,1,input);
					build_year=getshort(rec+2,0);
					build_rel=getshort(rec,0);
					if(build_year > 5 ) {
						itemsread=catdoc_read(rec,8,1,input);
						biff_version=8;
						offset=12;
					}
					else {
						biff_version=7;
						offset=4;
					}
				} else if (biff_version == 0x0209 ) {
					biff_version=3;
					offset=2;
				} else if (biff_version == 0x0409 ) {
					offset=2;
					biff_version=4;
				} else {
					biff_version=2;
				}
				itemsread=catdoc_read(rec,reclen-offset,1,input);
				break;
			} else {
				fprintf(stderr,"%s: Invalid BOF record\n",filename);
				return;
			} 
		} else {
			itemsread=catdoc_read(rec,126,1,input);
		}
	}
	if (catdoc_eof(input)) {
		fprintf(stderr,"%s: No BOF record found\n",filename);
		exit(1);
	}    
	while(itemsread){
		char buffer[2];
		rectype = 0;
		itemsread = catdoc_read(buffer, 2, 1, input);
		if (catdoc_eof(input)) {
			process_item(MSEOF,0,NULL);
			return;
		}
		
		rectype=getshort(buffer,0);
		if(itemsread == 0)
			break;
		reclen=0;

		itemsread = catdoc_read(buffer, 2, 1, input);
		reclen=getshort(buffer,0);
		if (reclen && reclen <MAX_MS_RECSIZE &&reclen >0){
			itemsread = catdoc_read(rec, 1, reclen, input);
			rec[reclen] = '\0';
		}
		if(eof_flag) {
			if (rectype != BOF) {
				break;
			}    
		}
/* 		fprintf(stderr,"Rectype 0x%04X reclen=%d\n",rectype, reclen); */
		process_item(rectype,reclen,rec);
		if (rectype == MSEOF) {
			eof_flag=1;
		} else {
			eof_flag=0;	
		}  
	}
	return;
}
unsigned char **sst=NULL;/* Shared string table parsed into array of strings in
														output encoding*/
int sstsize = 0; /*Number of strings in SST*/
unsigned char *sstBuffer=NULL; /*Unparsed sst to accumulate all its parts*/
int sstBytes = 0; /*Size of SST Data, already accumulated in the buffer */
int codepage=1251; /*default*/
int prev_rectype=0;
/* holds a pointer to formula value, becouse value itself would be in
 * next biff record
 */
unsigned char **saved_reference = NULL;

void process_item (int rectype, int reclen, char *rec) {
	if (rectype != CONTINUE && prev_rectype == SST) {
		/* we have accumulated  unparsed SST, and now encountered
		 * another record, which indicates that SST is ended */
		/* fprintf(stderr,"parse sst!\n");*/
		parse_sst(sstBuffer,sstBytes);
	}	
	switch (rectype) {
	case FILEPASS: {
		fprintf(stderr,"File is encrypted\n");
		exit(69);
		break;
	}
	case WRITEPROT: {
		fprintf(stderr,"File is write protected\n");
		break;
	}
		
	case 0x42: {
		if (source_charset) break;
		codepage=getshort(rec,0);
		/*fprintf(stderr,"CODEPAGE %d\n",codepage); */
		if (codepage!=1200) {
			const char *cp = charset_from_codepage(codepage); 
			source_charset=read_charset(cp);
		}		 
		break;
	}  
	case FORMAT: {
		int format_code;
		format_code=getshort(rec,0);
		SetFormatIdxUsed(format_code);
		/* this debug code prints format string */
		/*							  
			int i;
			char *ptr;
			fprintf(stderr,"Format %x \"",format_code);
			if (rec[2] == reclen - 3 && rec[3] != 0) {
			for (i=0,ptr=rec+3;i<rec[2];i++,ptr++) {
			fputc(*ptr,stderr);
			}
			} else {
			for (i=0,ptr=rec+5;i<rec[2];i++,ptr+=2) {
			fputc(*ptr,stderr);
			}
			}
			fprintf (stderr,"\"\n");
		*/
		break;
	}			 
	case SST: {
		/* Just copy SST into buffer, and wait until we get
		 * all CONTINUE records
		 */
/* 		fprintf(stderr,"SST\n"); */
		/* If exists first SST entry, then just drop it and start new*/
		if (sstBuffer != NULL) 
			free(sstBuffer);
		if (sst != NULL)
			free(sst);
		
		sstBuffer=(char*)malloc(reclen);
		sstBytes = reclen;
		if (sstBuffer == NULL ) {
			perror("SSTptr alloc error! ");
			exit(1);
		}	  
		memcpy(sstBuffer,rec,reclen);
		break;
	}	
	case CONTINUE: {
		if (prev_rectype != SST) {
			return; /* to avoid changing of prev_rectype;*/
		}    
		sstBuffer=realloc(sstBuffer,sstBytes+reclen);
		if (sstBuffer == NULL ) {
			perror("SSTptr realloc error! ");
			exit(1);
		}	  
		memcpy(sstBuffer+sstBytes,rec,reclen);
		sstBytes+=reclen;
		return;
	}			   
	case LABEL: {
		int row,col;
		unsigned char **pcell;
		unsigned char *src=(unsigned char *)rec+6;

		saved_reference=NULL;
		row = getshort(rec,0); 
		col = getshort(rec,2);
		/* 		fprintf(stderr,"LABEL!\n"); */
		pcell=allocate(row,col);
		*pcell=copy_unicode_string(&src);
		break;
	}     
	case BLANK: { int row,col;unsigned char **pcell;
			row = getshort(rec,0);
			col = getshort(rec,2);
			pcell=allocate(row,col);
			*pcell=NULL;
			break;
	}
	case MULBLANK: {
		int row, startcol,endcol;
		unsigned char **pcell;
		row = getshort(rec,0);
		startcol = getshort(rec,2);
		endcol=getshort(rec,reclen-2);
		pcell=allocate(row,endcol);
		*pcell=NULL;
		break;
	}	   
	case CONSTANT_STRING: {
		int row = getshort(rec,0); 
		int col = getshort(rec,2);
		unsigned char **pcell;
		int string_no=getshort(rec,6);
		if (!sst) {
			fprintf(stderr,"CONSTANT_STRING before SST parsed\n");
			exit(1);
		}    
		/* 									fprintf(stderr,"col=%d row=%d no=%d\n",col,row,string_no); */
									
		saved_reference=NULL;
		pcell=allocate(row,col);
		if (string_no>=sstsize|| string_no < 0 ) {
			fprintf(stderr,"string index out of boundary\n"); 
			exit(1);	 
		} else if (sst[string_no] !=NULL) {	
			int len;
			char *outptr;
			len=strlen(sst[string_no]);
			outptr=*pcell=malloc(len+1);
			strcpy(outptr,sst[string_no]);
		} else {
			*pcell=malloc(1);
			strcpy(*pcell,"");
		}	
		break;
	}
	case 0x03:
	case 0x103:
	case 0x303:
	case NUMBER: {
		int row,col;
		unsigned char **pcell;

		saved_reference=NULL;
		row = getshort(rec,0)-startrow; 
		col = getshort(rec,2);
		pcell=allocate(row,col);
		*pcell=strdup(format_double(rec,6,getshort(rec,4)));
		break;
	}
	case INTEGER_CELL: {
		int row,col;
		unsigned char **pcell;

		row = getshort(rec,0)-startrow;
		col = getshort(rec,2);
		pcell=allocate(row,col);
		*pcell=strdup(format_int(getshort(rec,7),getshort(rec,4)));		  
		break;

	}				  
	case RK: {
		int row,col,format_code;
		unsigned char **pcell;

		saved_reference=NULL;
		row = getshort(rec,0)-startrow; 
		col = getshort(rec,2);
		pcell=allocate(row,col);
		format_code = getshort(rec,4);
		*pcell=strdup(format_rk(rec+6,format_code));
		break;
	}
	case MULRK: {
		int row,col,startcol,endcol,offset,format_code;
		unsigned char **pcell;
		row = getshort(rec,0)-startrow; 
		startcol = getshort(rec,2);
		endcol = getshort(rec,reclen-2);
		saved_reference=NULL;

		for (offset=4,col=startcol;col<=endcol;offset+=6,col++) { 
			pcell=allocate(row,col);
			format_code=getshort(rec,offset);
			*pcell=strdup(format_rk(rec+offset+2,format_code));

		}		 
		break;
	} 
	case FORMULA: { 
		int row,col;
		unsigned char **pcell;
		saved_reference=NULL;
		row = getshort(rec,0)-startrow; 
		col = getshort(rec,2);
		pcell=allocate(row,col);
		if (((unsigned char)rec[12]==0xFF)&&(unsigned char)rec[13]==0xFF) {
			/* not a floating point value */
			if (rec[6]==1) {
				/*boolean*/
				char buf[2]="0";
				buf[0]+=rec[9];
				*pcell=strdup(buf);
			} else if (rec[6]==2) {
				/*error*/
				char buf[6]="ERROR";
				*pcell=strdup(buf);
			} else if (rec[6]==0) {
				saved_reference=pcell;
			}   
		} else {
			int format_code=getshort(rec,4);
			*pcell=strdup(format_double(rec,6,format_code));
		}		 
		break;
	}
	case STRING: {
		unsigned char *src=(unsigned char *)rec;
		if (!saved_reference) {
			fprintf(stderr,"String record without preceeding string formula\n");
			break;
		}
		*saved_reference=copy_unicode_string(&src);
		break;
	}	    
	case BOF: {
		if (rowptr) {
			fprintf(stderr,"BOF when current sheet is not flushed\n");
			free_sheet();
		}
		break;
	}	  
	case XF:
	case 0x43: /*from perl module Spreadsheet::ParseExecel */		  
		{
			short int formatIndex = getshort(rec,2);
			/* we are interested only in format index here */ 
			if (formatTableIndex >= formatTableSize) {
				formatTable=realloc(formatTable,
														(formatTableSize+=16)*sizeof(short int));
					  	  
				if (!formatTable) {
					fprintf(stderr,"Out of memory for format table");
					exit (1);
				}	  
			}	
			formatTable[formatTableIndex++] = formatIndex;
			break;
		} 
	case MS1904: /* Macintosh 1904 date system */ 
		date_shift=24107.0; 
		break;	 
						 
					 
	case MSEOF: {
		if (!rowptr) break;
		print_sheet();
		free_sheet();
		break;
	}
	case ROW: { 
		/*  		fprintf(stderr,"Row! %d %d %d\n",getshort(rec,0), getshort(rec+2,0),getshort(rec+4,0));  */
		break; 
	} 
	case INDEX: {
		/* 		fprintf(stderr,"INDEX! %d %d\n", getlong(rec+4,0), getlong(rec+8,0));  */
		break;
	}
	default: {
#if	0	
		fprintf(stderr,"Unknown record 0x%x\n length %d\n",rectype,reclen);		
#endif 
	}
	}
	prev_rectype=rectype; 
}  

/*
 * Extracts string from sst and returns mallocked copy of it
 */
char *copy_unicode_string (unsigned char **src) {
	int count=0;
	int flags = 0;
	int start_offset=0;
	int to_skip=0;								/* используется для подсчета длины данных
																 * за концом строки */
	int offset = 1;								/* для учета переменной длины первого поля  */
	int charsize;
	/* 	char *realstart=*src; */
	char *dest;										/* куда будем копировать строку */
	char *s,*d,*c;

	int i,u,l,len;

	/* 	for(i=0;i<20;i++) */
	/* 		fprintf(stderr,"%02x ",(*src)[i]); */
	/* 	fprintf(stderr,"\n"); */

	flags = *(*src+1+offset);
	if (! ( flags == 0 || flags == 1 || flags == 8 || flags == 9 ||
					flags == 4 || flags == 5 || flags == 0x0c || flags == 0x0d ) ) {
		count=**src;
		flags = *(*src+offset);
		offset --;
		flags = *(*src+1+offset);
		if (! ( flags == 0 || flags == 1 || flags == 8 || flags == 9 ||
						flags == 4 || flags == 5 || flags == 0x0c || flags == 0x0d ) ) {
			/* 			fprintf(stderr,"Strange flags = %d, returning NULL\n", flags); */
			return NULL;
		}
	}
	else {
		count=getshort(*src,0);
	}   
	charsize=(flags &0x01) ? 2 : 1;

	switch (flags & 12 ) {
	case 0x0c: /* Far East with RichText formating */
		to_skip=4*getshort(*src,2+offset)+getlong(*src, 4+offset);
		start_offset=2+offset+2+4;
		/* 		fprintf(stderr,"Far East with RichText formating\n"); */
		break;

	case 0x08: /* With RichText formating */
		to_skip=4*getshort(*src,2+offset);
		start_offset=2+offset+2;
		/* 		fprintf(stderr,"With RichText formating %d\n",getshort(*src,2+offset)); */
		break;

	case 0x04: /* Far East */
		to_skip=getlong(*src, 2+offset);
		start_offset=2+offset+4;
		/* 		fprintf(stderr,"Far East\n"); */
		break;

	default:
		to_skip=0;
		start_offset=2+offset;
		/* 		fprintf(stderr,"Default string\n"); */
	}

	/* 	fprintf(stderr,"count=%d skip=%d start_offset=%d\n", */
	/* 					count, to_skip, start_offset); */
	/* а здесь мы копируем строку	*/
	if ( (dest=malloc(count+1)) == NULL ) {
		perror("Dest string alloc error");
		*src+=(to_skip+start_offset+(count*charsize));
		exit(0);
	}
	*src+=start_offset;
	len = count;
	*dest=0;l=0;
	for (s=*src,d=dest,i=0;i<count;i++,s+=charsize) {
		/* 		fprintf(stderr,"l=%d len=%d count=%d charsize=%d\n",l,len,count,charsize); */
		if ( (charsize == 1 && (*s == 1 || *s == 0)) ||
				 (charsize == 2 && (*s == 1 || *s == 0) && *(s+1) != 4)) {
			/* 			fprintf(stderr,"extchar (unicode)=%02x %02x\n",*s, *(s+1)); */
			charsize=(*s &0x01) ? 2 : 1;
			if (charsize == 2)
				s-=1;
			count++;
			continue;
		}
		if ( charsize == 2 ){
			u=(unsigned short)getshort(s,0);
			c=convert_char(u);
			/* 			fprintf(stderr,"char=%02x %02x\n", *s, *(s+1)); */
		} else {
			if (!source_charset) {
				check_charset(&source_csname,source_csname);	
				/* fprintf(stderr,"charset=%s\n",source_csname);*/
				source_charset=read_charset(source_csname);
			}	
			u=(unsigned short)to_unicode(source_charset,(unsigned char)*s);
			c=convert_char(u);
		}
		if (c != NULL) {
			int dl = strlen(c);
			while (l+dl>=len) {
				len+=16;
				dest=realloc(dest,len+1);
			}
			d=dest+l;
			strcpy(d,c);
			d+=dl;
			l+=dl;
		}      
	}
	*src=s+to_skip;
	return dest;
}


/* 
 * Format code is index into format table (which is list of XF records
 * in the file 
 * Second word of XF record is format type idnex 
 * format index between 0x0E and 0x16 also between 0x2D and ox2F denotes
 * date if it is not used for explicitly stored formats.
 * BuiltInDateFormatIdx converts format index into index of explicit
 * built-in date formats sutable for strftime.
 */
int BuiltInDateFormatIdx (int index) {
	int offset;
	offset=1; /* offset of date formats */
	/* 0 is used as false -- format not found */
	if ((index>= 0x0E) && (index<=0x16)) {
		return offset+index-0x0E;
	} else 	
		if ((index>=0x2d) && (index<=0x2F)) {
			return offset+index-0x2d+9;
		} else if (index==0xa4) {	
			return 12+offset;
		} else 	
			return 0;
}	

/* 
 * GetBuiltInDateFormat stores and returns
 * built in xls2csv strftime formats.
 */
#define NUMOFDATEFORMATS 13
char *GetBuiltInDateFormat(int dateindex) {
	static char *formats[]={
		/* reserved  */ NULL, /* BuiltInDateFormatIdx use dateindex=0 as flag format not found */
		/* 0x0E */ "%m-%d-%y",		/* 01 */
		/* 0x0F */ "%d-%b-%y",		/* 02 */
		/* 0x10 */ "%d-%b",		/* 03 */
		/* 0x11 */ "%b-%d",	 	/* 04 */
		/* 0x12 */ "%l:%M %p",		/* 05 */
		/* 0x13 */ "%l:%M:%S %p",		/* 06 */
		/* 0x14 */ "%H:%M",		/* 07 */
		/* 0x15 */ "%H:%M:%S",		/* 08 */
		/* 0x16 */ "%m-%d-%y %H:%M",	/* 09 */
		/* 0x2d */ "%M:%S",		/* 10 */
		/* 0x2e */ "%H:%M:%S",		/* 11 */
		/* 0x2f */ "%M:%S",		/* 12 */
		/* 0xa4 */ "%m.%d.%Y %l:%M:%S %p"	/* 13 */
	};
	if (dateindex>0 && dateindex <= NUMOFDATEFORMATS) {
	  return formats[dateindex];
	} else 	
		return NULL;
}	

static char FormatIdxUsed[NUMOFDATEFORMATS];

void CleanUpFormatIdxUsed() {
	int i;
	for (i=0;i<NUMOFDATEFORMATS; i++);
	FormatIdxUsed[i]=0;
}

/* 
 * format index between 0x0E and 0x16 also between 0x2D and ox2F denotes
 * date in case when they are built-in Excel97 formats.
 * Nevertheless, those indexes can be used for explicitly stored formats,
 * which are not dates in general.
 * SetFormatIdxUsed marks this formats as already used 
 * and excludes them from list of built-in formats 
 * preventing misformatting of corresponding data.
 */
void SetFormatIdxUsed(int format_code) {
	int dateindex;
 	/*fprintf(stderr,"Format idx %x to be set to dirty\n",format_code);
	 */
	dateindex=BuiltInDateFormatIdx(format_code);
	if (dateindex) {
	  FormatIdxUsed[dateindex]=1;
 	  /*fprintf(stderr,"Date idx %d is set to be dirty\n",dateindex); */
	}
}	

/* 
 * format index between 0x0E and 0x16 also between 0x2D and ox2F denotes
 * date in case when they are built-in Excel97 formats.
 * Nevertheless, those indexes can be used for explicitly stored formats,
 * which are not dates in general.
 * SetFormatIdxUsed marks this formats as already used 
 * and excludes them from list of built-in formats 
 * preventing misformatting of corresponding data.
 * IsFormatIdxUsed tests this case.
 */
char IsFormatIdxUsed(int format_code) {
	int dateindex;
	dateindex=BuiltInDateFormatIdx(format_code);
	if (dateindex) {
		/* 	  fprintf(stderr,"Date idx %d is dirty\n",dateindex); */
	  return FormatIdxUsed[dateindex]==1;
	}
	else return 0;
}	


/* Checks if format denoted by given code is date
 * Format code is index into format table (which is list of XF records
 * in the file 
 * Second word of XF record is format type inex 
 * format index between 0x0E and 0x16 also between 0x2D and ox2F denotes
 * date.
 * If so, it returns strftime format for this date. Otherwise returns
 * NULL
 */


char *isDateFormat(int format_code) {
	int index;
	int dateindex;
	if (format_code>=formatTableIndex) {
		fprintf(stderr,"Format code %d is used before definition\n",format_code);
		return NULL;
	}
	
	index = formatTable[format_code];
	if (IsFormatIdxUsed(index)) {
		fprintf(stderr,"Format %x is redefined\n",index);
	  /* this format is something user-defined --- not a standard built-in date*/
	  return NULL;
	}
	dateindex=BuiltInDateFormatIdx(index);
	if (dateindex) {
	  if (forced_date_format) return forced_date_format;
		return GetBuiltInDateFormat(dateindex);
	} else 	
		return NULL;
}	



time_t float2date(double d);
/*
 * Extracts floating point value and formats it 
 */ 

char *number2string(double d,short int format_code) { 
	static char buffer [128];
	char *datefmt;
	if ((datefmt=isDateFormat(format_code))!=NULL) {
		time_t t = float2date(d);	
		strftime(buffer, 127,datefmt, gmtime(&t));  
	} else {
		sprintf(buffer,number_format,d);
	}
	return buffer;
}
		
char *format_double(char *rec,int offset,int format_code) {	
	union { char cc[8];
		double d;} dconv;
	char *d,*s; 
	int i;
# ifdef WORDS_BIGENDIAN     
	for(s=rec+offset+8,d=dconv.cc,i=0;
			i<8;i++) *(d++)=*(--s);
# else       
	for(s=rec+offset,d=dconv.cc,i=0;
			i<8;i++) *(d++)=*(s++);
# endif     
	return number2string(dconv.d,format_code);					
}

/*
 * Formats integer value into static buffer
 */
char *format_int(int value,int format_code) {
	static char buffer[12];
	sprintf(buffer,"%i",value);
	return buffer;
}	
/*
 * Formats RK record
 */
char* format_rk(char *rec,short int format_code) {
	double value=0.0;
	int i;

	if ( *(rec) & 0x02 )
	{
		value=(double)(getlong(rec,0)>>2);
	}
	else { 
		union { char cc[8];
			double d;} dconv;
		char *d,*s;
		for(i=0;i<8;i++)
			dconv.cc[i]='\0';
# ifdef WORDS_BIGENDIAN     
		for(s=rec+4,d=dconv.cc,i=0; i<4;i++) 
			*(d++)=*(--s);
		dconv.cc[0]=dconv.cc[0] & 0xfc;
# else       
		for(s=rec,d=dconv.cc+4,i=0;
				i<4;i++) *(d++)=*(s++);
		dconv.cc[3]=dconv.cc[3] & 0xfc;
# endif     
		value=dconv.d;
	}
	if ( *(rec) & 0x01 ) 
		value=value*0.01;
	return number2string(value,format_code);
}


/* 
 * Converts excel date into time_t
 */
time_t float2date(double f) { 
	/* Hacked version. Excell stores date as floating point count of days
	 * since 1.1.1900. or 1.1.1904
	 * We are substracting value of 1.1.1970 and multiplying
	 * by 86400 thus getting seconds from the epoch
	 */
	return rint((f-date_shift)*86400); 
}

/*
 * Parses SST into array of strings
 */
void parse_sst(char *sstbuf,int bufsize) {
	int i; /* index into sst */
	unsigned char *curString; /* pointer into unparsed buffer*/
	unsigned char *barrier=(unsigned char *)sstbuf+bufsize; /*pointer to end of buffer*/
	unsigned char **parsedString;/*pointer into parsed array*/ 
			
	sstsize = getlong(sstbuf+4,0);
	sst=malloc(sstsize*sizeof(char *));
	
	if (sst == NULL) {
		perror("SST allocation error");
		exit(1);
	}
	memset(sst,0,sstsize*sizeof(char *));
	for (i=0,parsedString=sst,curString=sstbuf+8;
			 i<sstsize && curString<barrier; i++,parsedString++) {
		/* 		fprintf(stderr,"copying %d string\n",i); */
		*parsedString = copy_unicode_string(&curString);
	}       
	/* 	fprintf(stderr,"end sst i=%d sstsize=%d\n",i,sstsize); */

}	
