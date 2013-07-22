/*
  Copyright 1998-2003 Victor Wagner
  Copyright 2003 Alex Ott
  This file is released under the GPL.  Details can be
  found in the file COPYING accompanying this distribution.
*/
#ifndef CATDOC_H
#define CATDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* There is some strange thing on aix */
#if (defined(_AIX)||defined(___AIX)) && !defined(__unix)
# define __unix 1
#endif

/* These include files are always available */
#include <stdio.h>
#include <ctype.h>

/* This is our own file */
#include "ole.h"

/*
 * User customization
 *
 */


#if defined(__MSDOS__) || defined(_WIN32)
/* MS-DOS doesn't like dot at first char and thinks that suffix 
 * should be separated by dot. So we'd call personal config catdoc.rc
 */
# define USERRC "catdoc.rc"
/* In DOS, %s in path gets replaced with full path to executable including
   trailing backslash.
 */
# ifndef SYSTEMRC
#  define SYSTEMRC "%s\\catdoc.rc"
# endif
# ifndef CHARSETPATH
#  define CHARSETPATH "%s\\charsets"
# endif
/* Function to add executable directory in place of %s in path.
   Not usable in Unix, where executable can have more then one
   link and configuration files are usially kept separately   from executables
 */
char *add_exe_path(const char* name);
/* Separator of directories in list, such as PATH env var. */
# define LIST_SEP ';'
/* Separator of levels inside path */
# define DIR_SEP '\\'
#else
/* On POSIX systems personal configuration files should start with dot*/
# ifndef USERRC
#  define USERRC ".catdocrc"
# endif

# ifndef SYSTEMRC
#  define SYSTEMRC "/usr/local/lib/catdoc/catdocrc"
# endif

# ifndef CHARSETPATH
#  define CHARSETPATH "/usr/local/lib/catdoc"
# endif
/* Macro to add executable directory in place of %s in path.
   Not usable in Unix, where executable can have more then one
   link and configuration files are usially kept separately   from executables
 */
# define add_exe_path(name) name
/* Separator of directories in list, such as PATH env var. */
# define LIST_SEP ':'
/* Separator of levels inside path */
#define DIR_SEP '/'
#endif

/* Charset files distributed with catdoc always have .txt extension*/
#ifndef CHARSET_EXT
# define CHARSET_EXT ".txt"
#endif

/* Default charsets */
#ifndef TARGET_CHARSET
#if defined(__MSDOS__) || defined(_WIN32)
#define TARGET_CHARSET "cp866"
#else
#define TARGET_CHARSET "koi8-r"
#endif
#endif

#ifndef SOURCE_CHARSET
#define SOURCE_CHARSET "cp1251"
#endif

#ifndef UNKNOWN_CHAR
#define UNKNOWN_CHAR "?"
#endif
/* On MS-DOS and WIN32 files have to have 3-char extension */
#if defined(__MSDOS__) || defined(_WIN32)
# ifndef SPEC_EXT
#  define SPEC_EXT ".spc"
# endif
# ifndef REPL_EXT 
#  define REPL_EXT ".rpl"
# endif
#else

/* On other system we'll rename them to something more readable */
# ifndef SPEC_EXT
#  define SPEC_EXT ".specchars"
# endif
# ifndef REPL_EXT
#  define REPL_EXT ".replchars"
# endif
#endif
#if defined(__MSDOS__) && !defined(__DJGPP__)
/* Buffer sizes for 16-bit DOS progran */
#define PARAGRAPH_BUFFER 16384
#define FILE_BUFFER  32256
#define PATH_BUF_SIZE 80 
#else
/* Buffers for 32-bit and more program */
#define PARAGRAPH_BUFFER 262144
#define FILE_BUFFER 262144
#define PATH_BUF_SIZE 1024
#endif

/* Buffer for single line. Should be greater than wrap margin +
  longest substitution sequence */
#define LINE_BUF_SIZE 512
/*   Default value for wrap margin */
#ifndef WRAP_MARGIN
#define WRAP_MARGIN 72
#endif
/* variable (defined in catdoc.c) which holds actual value of wrap margin*/
extern  int wrap_margin;
/*
 * Public types variables and procedures which should be avalable
 * to all files in the program
 */

#ifdef __TURBOC__
/* Turbo C defines broken isspace, which works only for us-ascii */
#undef isspace
#define isspace(c) ((unsigned char)(c) <=32)
#endif

/* Structure to store UNICODE -> target charset mappings */
/* array of 256 pointers (which may be null) to arrays of 256 short ints
   which contain 8-bit character codes or -1 if no matching char */
typedef short int  ** CHARSET;

/* structure to store multicharacter substitution mapping */
/* Array of 256 pointers to arrays of 256 pointers to string */
/* configuration variables defined in catdoc.c */
typedef char *** SUBSTMAP;

extern short int *source_charset;
extern char bad_char[]; /* defines one-symbol string to replace unknown unicode chars */
extern char *source_csname;
extern char *dest_csname;
extern char *format_name;
extern CHARSET target_charset;
extern SUBSTMAP spec_chars;
                /* Defines unicode chars which should be
                replaced by strings before UNICODE->target chatset
                mappigs are applied i.e. TeX special chars like %
                */
extern SUBSTMAP replacements;
                 /* Defines unicode chars which could be
                    mapped to some character sequence if no
                    corresponding character exists in the target charset
                    i.e copyright sign */
extern int verbose; /* if true, some additional information would be
		       printed. defined in analyze.c */
extern int (*get_unicode_char)(FILE *f,long *offset,long fileend); 
/* pointer to function which gets
                                     a char from stream */

extern int get_utf16lsb (FILE *f,long *offset,long fileend);
extern int get_utf16msb (FILE *f,long *offset,long fileend);
extern int get_utf8 (FILE *f,long *offset,long fileend);
extern int get_8bit_char (FILE *f,long *offset,long fileend);

extern int get_word8_char (FILE *f,long *offset,long fileend);

extern const char *charset_from_codepage(unsigned int codepage);
extern  short int *read_charset(const char *filename);
extern CHARSET make_reverse_map (short int *charset);

extern int to_unicode (short int *charset, int c) ;

extern int from_unicode (CHARSET charset, int u) ;

extern char* convert_char(int unicode_char);

extern char* to_utf8(unsigned int uc);

extern char* map_path, *charset_path;
extern int signature_check;
extern int unknown_as_hex;
char *find_file(char *name, const char *path);
char *stradd(const char *s1, const char *s2);
void read_config_file(const char *filename);
#ifdef HAVE_LANGINFO
void get_locale_charset(void);
#if  defined(HAVE_STRFTIME) && !defined(__TURB0C__)
void	set_time_locale();
#endif	
#endif
SUBSTMAP read_substmap(char* filename);
extern int longest_sequence;/* for checking which value of wrap_margin
                             can cause buffer overflow*/
char *map_subst(SUBSTMAP map,int uc);

int check_charset(char **filename,const char *charset);
int process_file(FILE *f,long stop);
void copy_out(FILE *f, char *header);
void output_paragraph(unsigned short int *buffer) ;
int parse_rtf(FILE *f);
/* format recognition*/
int analyze_format(FILE *f);
void list_charsets(void);
int parse_word_header(unsigned char *buffer,FILE *f,int offset,long curpos);
/* large buffers for file IO*/
extern char *input_buffer,*output_buffer;
#ifndef HAVE_STRDUP
	char *strdup(const char *s);
#endif
/* numeric conversions */	
long int getlong(unsigned char *buffer,int offset);
unsigned long int getulong(unsigned char *buffer,int offset);
unsigned int getshort(unsigned char *buffer,int offset);
#endif
