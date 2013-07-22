/*****************************************************************/
/* Reading routines for rtf files                                */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 2003, (c) Alex Ott 2003	             */
/*****************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "catdoc.h"

/********************************************************
 * Datatypes declaration
 * 
 */
typedef enum {
	RTF_CODEPAGE,
	RTF_FONT_CHARSET,
	RTF_UC,
	RTF_UNICODE_CHAR,
	RTF_CHAR,
	RTF_PARA,
	RTF_TABLE_START,
	RTF_TABLE_END,
	RTF_ROW,
	RTF_CELL,
	RTF_UNKNOWN,
	RTF_OVERLAY,
	RTF_PICT,
	RTF_F,
	RTF_AUTHOR,
	RTF_FONTTBL,
	RTF_INFO,
	RTF_STYLESHEET,
	RTF_COLORTBL,
	RTF_LISTOVERRIDETABLE,
	RTF_LISTTABLE,
	RTF_RSIDTBL,
	RTF_GENERATOR,
	RTF_DATAFIELD,
	RTF_LANG,
	RTF_PARD,
	RTF_TAB,
	RTF_SPEC_CHAR,
	RTF_EMDASH,
	RTF_ENDASH,
	RTF_EMSPACE,
	RTF_ENSPACE,
 	RTF_BULLET, 
 	RTF_LQUOTE,
	RTF_RQUOTE,
	RTF_LDBLQUOTE,
	RTF_RDBLQUOTE,
	RTF_ZWNONJOINER,
} RTFTypes;

typedef struct {
	char *name;
	RTFTypes type;
} RTFTypeMap;

RTFTypeMap rtf_types[]={
	{"uc",RTF_UC},
	{"ansicpg",RTF_CODEPAGE},
	{"pard",RTF_PARD},
	{"par",RTF_PARA},
	{"cell",RTF_CELL},
	{"row",RTF_ROW},
 	{"overlay",RTF_OVERLAY}, 
 	{"pict",RTF_PICT},
 	{"author",RTF_AUTHOR},
 	{"f",RTF_F}, 
 	{"fonttbl",RTF_FONTTBL}, 
 	{"info",RTF_INFO}, 
 	{"stylesheet",RTF_STYLESHEET},
 	{"colortbl",RTF_COLORTBL},
 	{"listtable",RTF_LISTTABLE},
 	{"listoverridetable",RTF_LISTOVERRIDETABLE},
 	{"rsidtbl",RTF_RSIDTBL}, 
 	{"generator",RTF_GENERATOR}, 
 	{"datafield",RTF_DATAFIELD}, 
 	{"lang",RTF_LANG}, 
 	{"tab",RTF_TAB}, 
	{"emdash",RTF_EMDASH},
	{"endash",RTF_ENDASH},
	{"emspace",RTF_EMDASH},
	{"enspace",RTF_ENDASH},
 	{"bullet",RTF_BULLET}, 
 	{"lquote",RTF_LQUOTE},
	{"rquote",RTF_RQUOTE},
	{"ldblquote",RTF_LDBLQUOTE},
	{"rdblquote",RTF_RDBLQUOTE},
	{"zwnj",RTF_ZWNONJOINER},
/* 	{"",}, */
/* 	{"",}, */
	{"u",RTF_UNICODE_CHAR}
};

#define RTFNAMEMAXLEN 32
#define RTFARGSMAXLEN 64

/**
 * Structure describing rtf command
 * 
 */
typedef struct {
	RTFTypes type;
	char name[RTFNAMEMAXLEN+1];
	signed int numarg;
/*	void *args; */
} RTFcommand;


#define MAXFONTNAME 64
/**
 * 
 * 
 */
typedef struct {
	int name;
	char fontname[MAXFONTNAME+1];
} RTFFont;

/**
 * Structure to describe style
 * 
 */
typedef struct {
	int codepage;
} RTFStyle;

/**
 * Structure to store values, local to rtf group
 * 
 */
typedef struct {
	int uc;						/**< How much symbols to skip */
	RTFStyle* style;			/**< curren style */
} RTFGroupData;

/********************************************************
 * Functions declaration
 * 
 */

extern int forced_charset;
signed long getNumber(FILE *f);

int getRtfCommand(FILE *f, RTFcommand *command );
unsigned short int rtf_to_unicode(int code);
RTFTypes getCommandType(char *name);
signed int getCharCode(FILE *f);
void rtfSetCharset(short int **charset_ptr,unsigned int codepage);

/********************************************************
 * Global data
 * 
 */
short int *current_charset;
int rtf_level=0;

/********************************************************
 * Functions implementation
 * 
 */
extern unsigned short int buffer[];
void add_to_buffer(int *bufptr,unsigned short int c) {
	buffer[++(*bufptr)]=c;
	if (*bufptr > PARAGRAPH_BUFFER-2) {
		buffer[++(*bufptr)]=0;
		output_paragraph(buffer);
		*bufptr=-1;
	}
}

void end_paragraph(int *bufptr) {
				   add_to_buffer(bufptr,0x000a);
				   add_to_buffer(bufptr,0);
				   output_paragraph(buffer);
				   *bufptr=-1;
}				   

/** 
 * Parses RTF file from file stream
 * 
 * @param f - file stream descriptor
 */
int parse_rtf(FILE *f) {
	int para_mode=0, data_skip_mode=0,i;
	RTFGroupData *groups=NULL;
	int group_count=0, group_store=20;
	int bufptr=-1;
	current_charset=source_charset;
	fseek(f,0,SEEK_SET);
	if((groups=(RTFGroupData*)calloc(group_store,sizeof(RTFGroupData))) == NULL ) {
		perror("Can\'t allocate memory: ");
		return 1;
	}
	groups[0].uc = 2; /* DEfault uc = 2 */
	while ( !feof(f) ) {
		int c = fgetc(f);
		if ( feof( f ) )
			break;
		switch (c) {
		case '\\': {
			int code;
			RTFcommand com;
			if ((code=getRtfCommand(f, &com)) != 0)
				break;
			switch (com.type) {
			case RTF_SPEC_CHAR:
/* 				fprintf(stderr, "Spec Char found=%s and arg=%c\n", */
/* 				com.name, com.numarg); */
				if (com.numarg == '*' && data_skip_mode == 0) {
					data_skip_mode=group_count;
				} else if (com.numarg == '\r') {
					end_paragraph(&bufptr);
				} else if (com.numarg == '~') {
					add_to_buffer(&bufptr,0xA0);/* NO-BREAK SPACE */
				} else if (com.numarg == '-') {
					add_to_buffer(&bufptr,0xAD);/* Optional hyphen */
				}	

				   break;
			case RTF_EMDASH:
				   add_to_buffer(&bufptr,0x2014);/* EM DASH*/
				   break;
			case RTF_ENDASH: 
				   add_to_buffer(&bufptr,0x2013);break;
			case RTF_BULLET: 
				   add_to_buffer(&bufptr,0x2022);break;
			case RTF_LQUOTE: add_to_buffer(&bufptr,0x2018);break;
			case RTF_RQUOTE: add_to_buffer(&bufptr,0x2019);break;
			case RTF_LDBLQUOTE: add_to_buffer(&bufptr,0x201C);break;
			case RTF_RDBLQUOTE: add_to_buffer(&bufptr,0x201D);break;
			case RTF_ZWNONJOINER: add_to_buffer(&bufptr,0xfeff);break;
			case RTF_EMSPACE:
			case RTF_ENSPACE:
					add_to_buffer(&bufptr,' ');break;
			case RTF_CHAR:
/*  				fprintf(stderr, "RTF char %d\n", com.numarg); */
				if (data_skip_mode == 0) {
				 	add_to_buffer(&bufptr,rtf_to_unicode(com.numarg));
				}	
				break;
			case RTF_UC:
				groups[group_count].uc=com.numarg;
				break;
			case RTF_TAB:
				add_to_buffer(&bufptr,0x0009);
				break;
			case RTF_UNICODE_CHAR:
				if (com.numarg < 0)
					break;
/*  				fprintf(stderr, "Unicode char %d\n", com.numarg);  */
				if (data_skip_mode == 0)
					add_to_buffer(&bufptr,com.numarg);
				i=groups[group_count].uc;
				while((--i)>0)
					fgetc(f);
				break;
			case RTF_PARA:
				/*if (para_mode > 0) {*/
					end_paragraph(&bufptr);	
				/*}*/	
				para_mode=group_count;
				break;
			case RTF_PICT:
			case RTF_FONTTBL:
			case RTF_INFO:
			case RTF_COLORTBL:
			case RTF_STYLESHEET:
			case RTF_LISTTABLE:
			case RTF_LISTOVERRIDETABLE:
			case RTF_RSIDTBL:
			case RTF_GENERATOR:
			case RTF_DATAFIELD:
				if (data_skip_mode == 0){
					data_skip_mode=group_count;
				}
				break;
			case RTF_LANG:
/* 				fprintf(stderr, "Selected lang = %d\n",com.numarg); */
				break;
			case RTF_CODEPAGE:
				rtfSetCharset(&current_charset,com.numarg);
			default:
/*  				fprintf(stderr, "Unknown command with name %s and arg=%d\n",  */
/*  						com.name, com.numarg);  */
			;
			}
			break;
		}
		case '{':
			group_count++;
			if (group_count >= group_store ) {
				group_store+=10;
				if((groups=(RTFGroupData*)realloc(groups,
												  group_store*sizeof(RTFGroupData)))
				   == NULL ) {
					perror("Can\'t allocate memory: ");
					return 1;
				}
			}
			if (para_mode)
				add_to_buffer(&bufptr,0x20);
			groups[group_count]=groups[group_count-1];
			break;
		case '}':
			group_count--;
			if(group_count < 0)
				group_count=0;
			if(para_mode > 0 && para_mode > group_count) {
 				/*add_to_buffer(&bufptr,0);
				output_paragraph(buffer);
				fprintf(stderr,"\nGROUP_END para_mode=%d group_count=%d bufptr=%d\n", para_mode,group_count,bufptr);
				bufptr=-1;*/
				para_mode=0;
			}
			if(data_skip_mode > group_count) {
				data_skip_mode=0;
			}
			break;
		default:
			if (data_skip_mode == 0)
				if (c != '\n' && c != '\r')
					add_to_buffer(&bufptr,rtf_to_unicode(c));
		}
	}
	if (bufptr>=0) {
		add_to_buffer(&bufptr,'\n');
		add_to_buffer(&bufptr,0);
		output_paragraph(buffer);
	}	
	free(groups);
	return 0;
}  

/** 
 * Convert text string to number
 * 
 * @param f stream to read data from
 * 
 * @return converted number
 */
signed long getNumber(FILE *f) {
	int c,count=0;
	char buf[RTFARGSMAXLEN];
	
	while(isdigit(c=fgetc(f)) || c=='-') {
		if(feof(f))
			return -1;
		buf[count++]=(char)c;
	}
	ungetc(c,f);
	buf[count]='\0';
	return strtol(buf, (char **)NULL, 10);
}

/** 
 * Parse command stream from rtf file and fill command structure
 * 
 * @param f - rtf file stream
 * @param command - pointer to RTFcommand structure to fill
 * 
 * @return parse code not 0 - error, 0 - success
 */
int getRtfCommand(FILE *f, RTFcommand *command ) {
	int c=fgetc(f);
	if (isalpha(c)) {
		int name_count=1;
		command->name[0]=(char)c;
		while(isalpha(c=fgetc(f)) && name_count < RTFNAMEMAXLEN) {
			if(feof(f))
				return 1;
			command->name[name_count++]=(char)c;
		}
		command->name[name_count]='\0';
		command->type=getCommandType(command->name);
/* 		command->args=NULL; */
		ungetc(c,f);
		if (isdigit(c) || c == '-' )
			command->numarg=getNumber(f);
		else
			command->numarg=0;
		c=fgetc(f);
		if(!(c==' ' || c=='\t'))
			ungetc(c,f);
	} else {
		command->name[0]=(char)c;
		command->name[1]='\0';
/* 		command->args=NULL; */
		if (c == '\'') {
			command->type=RTF_CHAR;
			command->numarg=getCharCode(f);
			if(feof(f))
				return -1;
		} else {
			command->type=RTF_SPEC_CHAR;
			command->numarg=c;
		}
	}
	
	return 0;
}

/** 
 * Converts char to unicode.
 * 
 * @param code - integer code of char
 * 
 * @return converted char
 */
unsigned short int rtf_to_unicode(int code) {
	int cc=code;
	if (code < 0 || (cc=to_unicode(current_charset, code)) < 0 ) return 0xFEFF;
	return cc;
}

/** 
 * Convert name of RTF command to RTFType
 * 
 * @param name name to convert
 * 
 * @return RTFType, if unknown command, then return RTF_UNKNOWN
 */
RTFTypes getCommandType(char *name) {
	int i, olen=sizeof(rtf_types)/sizeof(RTFTypeMap);
	for (i = 0; i < olen ; i++) {
		if ( strcmp(name,rtf_types[i].name) == 0 ) {
			return rtf_types[i].type;
		}
	}
	return RTF_UNKNOWN;
}

/** 
 * Return number representing char code in Hex
 * 
 * @param f stream to read data from
 * 
 * @return converted number
 */
signed int getCharCode(FILE *f) {
	int c,count=0,i;
	char buf[RTFARGSMAXLEN];
	for(i=0;i<2; i++) {
		if (isdigit(c=fgetc(f))||(c>='a' && c<='f')) {
			if(feof(f))
				return -1;
			buf[count++]=(char)c;
		} else 
			ungetc(c,f);
	}

	buf[count]='\0';
	return strtol(buf, (char **)NULL, 16);
}

void rtfSetCharset(short int **charset_ptr,unsigned int codepage)
{
	/* Do not override charset if it is specified in the command line */
	const char *charset_name;
	char *save_buf = input_buffer;
	if (forced_charset) return;
	charset_name = charset_from_codepage(codepage);
	check_charset(&source_csname,charset_name);
	input_buffer=NULL;
	*charset_ptr = read_charset(source_csname);	
	input_buffer = save_buf;
}
