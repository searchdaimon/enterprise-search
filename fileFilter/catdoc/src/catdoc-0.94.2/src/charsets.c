/*
  Copyright 1998-2003 Victor Wagner
  Copyright 2003 Alex Ott
  This file is released under the GPL.  Details can be
  found in the file COPYING accompanying this distribution.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "catdoc.h"

char *charset_path=CHARSETPATH;
char *source_csname=SOURCE_CHARSET, *dest_csname=TARGET_CHARSET;
short int * source_charset;
int unknown_as_hex=0;
char bad_char[]=UNKNOWN_CHAR;
CHARSET target_charset;
/************************************************************************/
/* Converts char in input charset into unicode representation           */
/* Should be converted to macro                                         */
/************************************************************************/
int to_unicode (short int *charset, int c) {
	return charset[c];
}
/************************************************************************/
/* Search inverse charset record for given unicode char and returns     */
/* 0-255 char value if found, -1 otherwise                              */
/************************************************************************/
int from_unicode (CHARSET charset, int u) {
	short int *p;
	/* This is really assignment, not comparation */
	if ((p=charset[(unsigned)u>>8])) {
		return p[u & 0xff];
	} else {
		return -1;
	}
}
/************************************************************************/
/*  Converts direct (charset -> unicode) to reverse map                 */
/************************************************************************/
CHARSET make_reverse_map(short int *charset) {
	CHARSET newmap=calloc(sizeof(short int *), 256);
	int i,j,k,l;
	short int *p;   
	if (! charset) {
		return NULL;
	}	
	for (i=0;i<256;i++) {
		k= charset[i];
		j=  (unsigned)k>>8;
		if (!newmap[j]) {
			newmap[j] = malloc(sizeof(short int *)*256);
			if (!newmap[j]) {
				fprintf(stderr,"Insufficient memory for  charset\n");
				exit(1);
			}
			for (l=0,p=newmap[j];l<256;l++,p++) *p=-1;
		}
		p=newmap[j];
		p[k & 0xff]=i;
	}
	return newmap;
}

/************************************************************************/
/* Reads charset file (as got from ftp.unicode.org) and returns array of*/
/* 256 short ints (malloced) mapping from charset t unicode             */
/************************************************************************/
short int * read_charset(const char *filename) {
	char *path;
	FILE *f;
	short int *new=calloc(sizeof(short int),256);
	int c;
	long int uc;
	path= find_file(stradd(filename,CHARSET_EXT),charset_path);
	if (!path) {
		fprintf(stderr,"Cannot load charset %s - file not found\n",filename);
		return NULL;
	}
	f=fopen(path,"rb");

	if (!f) {
		perror(path); 
		return NULL;
	}
	if (input_buffer)
		setvbuf(f,input_buffer,_IOFBF,FILE_BUFFER);
	/* defaults */
	for (c=0;c<32;c++) {
		new[c]=c;
	}
	while (!feof(f)) {
		if (fscanf(f,"%i %li",&c,&uc)==2) {
			if (c<0||c>255||uc<0||(uc>0xFEFE&& uc!=0xFFFE)) {
				fprintf(stderr,"Invalid charset file %s\n",path);
				fclose(f);
				return NULL;
			}
			new[c]=uc;
		}
		while((fgetc(f)!='\n')&&!feof(f)) ;
	}
	fclose (f);
	free(path);
	return new;
}


/************************************************************************/
/* Reads 8-bit char and convers it from source charset                  */
/************************************************************************/

int get_8bit_char (FILE *f,long *offset,long fileend)
{
	unsigned char buf;
	if (catdoc_read(&buf, 1, 1, f)==0) return EOF;
	(*offset)++;  
	return to_unicode(source_charset,buf);
}


/************************************************************************/
/* Reads 16-bit unicode value. MS-Word runs on LSB-first machine only,  */
/* so read lsb first always and don't care about proper bit order       */
/************************************************************************/

int get_utf16lsb (FILE *f,long *offset,long fileend) {
	unsigned char buf[2];
    int result;
	result=catdoc_read(buf, 1, 2, f);
	if (result<0) {
		perror("read:");
		exit(1);
	}
	if (result !=2) {
		return EOF;
	}	
	(*offset)+=2;
	return ((int)buf[1])|(((int)buf[0])<<8);
}

/************************************************************************/
/* Reads 16-bit unicode value written in MSB order. For processing 
 * non-word files            .                                          */
/************************************************************************/
int get_utf16msb (FILE *f,long *offset,long fileend) {
	unsigned char buf[2];
    int result;
	result=catdoc_read(buf, 1, 2, f);
	if (result<0) {
		perror("read:");
		exit(1);
	}
	if (result !=2) {
		return EOF;
	}	
	(*offset)+=2;
	return ((int)buf[0])|(((int)buf[1])<<8);
}

int get_utf8 (FILE *f,long *offset,long fileend) {
	unsigned char buf[3];
	int d,c;
    int result;
	result=catdoc_read(buf, 1, 1, f);
	if (result<0) {
		perror("read");
		exit(1);
	}	
	if (result==0) return EOF;
	c=buf[0];
	d=0;
	if (c<0x80) 
		return c;
	if (c <0xC0) 
		return 0xfeff; /*skip corrupted sequebces*/
	if (c <0xE0) {
		if (catdoc_read(buf+1, 1, 1, f)<=0) return EOF;
		return ((c & 0x1F)<<6 | ((char)buf[1] & 0x3F));
	}
	if (c <0xF0) {
		if (catdoc_read(buf+1, 1, 2, f)<=2) return (int)EOF;
		return ((c & 0x0F)<<12)|
			((buf[1] & 0x3f)<<6)|
					 (buf[2] & 0x3f);
	}  
	return 0xFEFF; 
}

/**************************************************************************/
/*  Converts unicode char to output charset sequence. Coversion have      */
/*  three steps: 1. Replacement map is searched for the character in case */
/* it is not allowed for output format (% in TeX, < in HTML               */
/* 2. target charset is searched for this unicode char, if it wasn't      */
/*  replaced. If not found, then 3. Substitution map is searched          */
/**************************************************************************/
char *convert_char(int uc) {
	static char plain_char[]="a"; /*placeholder for one-char sequences */
	static char hexbuf[8];
	char *mapped;
	int c;
	if ((mapped=map_subst(spec_chars,uc))) return mapped;
	if (target_charset) { 
		c =from_unicode(target_charset,uc);
		if (c>=0) {
			*plain_char=c;
			return plain_char;
		}
		if ((mapped = map_subst(replacements,uc))) return mapped;
		if (unknown_as_hex) {
			sprintf(hexbuf,"\\x%04X",(unsigned)uc);
			/* This sprintf is safe, becouse uc is unicode character code,
			   which cannot be greater than 0xFFFE. It is ensured by routines
			   in reader.c
			   */
			return hexbuf;
		}   
		return  bad_char;
	} else {
		/* NULL target charset means UTF-8 output */
		return to_utf8(uc);
	}  
}
/******************************************************************/
/* Converts given unicode character to the utf-8 sequence         */
/* in the static string buffer. Buffer wouldbe overwritten upon   */
/* next call                                                      */
/******************************************************************/ 
char *to_utf8(unsigned int uc) {
	static char utfbuffer[4]; /* it shouldn't overflow becouse we never deal
								 with chars greater than 65535*/
	int count=0;
	if (uc< 0x80) {
		utfbuffer[0]=uc;
		count=1;
	} else  {
		if (uc < 0x800) {
			utfbuffer[count++]=0xC0 | (uc >> 6);
		} else {
			utfbuffer[count++]=0xE0 | (uc >>12);
			utfbuffer[count++]=0x80 | ((uc >>6) &0x3F);
		}	    
		utfbuffer[count++]=0x80 | (uc & 0x3F);
	}  
	utfbuffer[count]=0;
	return utfbuffer;
}    

struct cp_map {
	int codepage;
	char *charset_name;
};

struct cp_map cp_to_charset [] = {
	{10000,"mac-roman"},
	{10001,"mac-japanese"},
	{10002,"mac-tchinese"},
	{10003,"mac-korean"},
	{10004,"mac-arabic"},
	{10005,"mac-hebrew"},
	{10006,"mac-greek1"},
	{10007,"mac-cyrillic"},
	{10008,"mac-schinese"},
	{10010,"mac-romania"},
	{10017,"mac-ukraine"},
	{10021,"mac-thai"},
	{10029,"mac-centeuro"},
	{10079,"mac-iselandic"},
	{10081,"mac-turkish"},
	{10082,"mac-croatia"},
	{20866,"koi8-r"},
	{28591,"8859-1"},
	{28592,"8859-2"},
	{28593,"8859-3"},
	{28594,"8859-4"},
	{28595,"8859-5"},
	{28596,"8859-6"},
	{28597,"8859-7"},
	{28598,"8859-8"},
	{28599,"8859-9"},
	{28605,"8859-15"},
	{65001,"utf-8"},
    {0,NULL}};
const char *charset_from_codepage(unsigned int codepage) {
	
	static char buffer[7];
	struct cp_map *cp;
	if (codepage==1200||codepage==1201) {
		/* For UCS2 */
		return "";
	} else 
	if (codepage<10000) {
		sprintf(buffer,"cp%d",codepage);
		return buffer;
	} else {
		for (cp = cp_to_charset;cp->codepage!=0&& cp->codepage!=codepage;cp++);
		return cp->charset_name;
	}
}	
