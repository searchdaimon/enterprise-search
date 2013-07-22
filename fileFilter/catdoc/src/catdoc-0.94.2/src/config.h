/* src/config.h.  Generated automatically by configure.  */
/* src/config.h.in.  Generated automatically from configure.in by autoheader 2.13.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if the setvbuf function takes the buffering type as its second
   argument and the buffer pointer as the third, as on System V
   before release 3.  */
/* #undef SETVBUF_REVERSED */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define this if you have XPG4 comliant nl_langinfo, which accepts CODESET argument */
#define HAVE_LANGINFO 1

/* Character encoding used by default for 8-bit source files */
#define SOURCE_CHARSET "cp1251"

/* Output character encoding used by default, if impossible to determine encoding from locale */
#define TARGET_CHARSET "koi8-r"

/* Suffix for files with special symbols map (ones to be replaced regardless of availability in target encoding) */
#define SPEC_EXT ".specchars"

/* Suffix for symbols replacement map (what to do with symbols, which are not available in the target encoding) */
#define REPL_EXT ".replchars"

/* Symbol to represent character which is not available either in target encoding or in replacement map */
#define UNKNOWN_CHAR "?"

