/*
  cgi-util.h
  
  version 2.1.7
  
  by Bill Kendrick <bill@newbreedsoftware.com>
  and Mike Simons <msimons@moria.simons-clan.com>
  
  New Breed Software
  http://www.newbreedsoftware.com/cgi-util/
  
  April 6, 1996 - October 18, 2002
*/

#ifndef CGI_UTIL_H
#define CGI_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#define CGIUTILVER "2.1.7"


/* Form data (name/value entries): */

typedef struct entry_type
{
  char * name;
  char * val;
  char * content_type;
  int content_length;
} cgi_entry_type;

extern cgi_entry_type * cgi_entries;


/* Number of unique name/value entries found (by cgi_init() below). */

extern int cgi_num_entries;


/* Possible error states: */

enum {
  CGIERR_NONE,
  CGIERR_NOT_INTEGER,
  CGIERR_NOT_DOUBLE,
  CGIERR_NOT_BOOL,
  CGIERR_UNKNOWN_METHOD,
  CGIERR_INCORRECT_TYPE,
  /*  CGIERR_NULL_QUERY_STRING, */
  CGIERR_BAD_CONTENT_LENGTH,
  CGIERR_CONTENT_LENGTH_DISCREPANCY,
  CGIERR_CANT_OPEN,
  CGIERR_OUT_OF_MEMORY,
  CGIERR_NO_BOUNDARY,
  CGIERR_NO_COOKIES,
  CGIERR_COOKIE_NOT_FOUND,
  CGIERR_N_OUT_OF_BOUNDS,
  CGIERR_NUM_ERRS
};

extern char * cgi_error_strings[CGIERR_NUM_ERRS];


/* Current error state. */

extern int cgi_errno;


enum {
  CGIREQ_NONE,
  CGIREQ_GET,
  CGIREQ_POST,
  CGIREQ_UNKNOWN
};

/*
  Request method:
*/

extern int cgi_request_method;


enum {
  CGITYPE_NONE,
  CGITYPE_APPLICATION_X_WWW_FORM_URLENCODED,
  CGITYPE_MULTIPART_FORM_DATA,
  CGITYPE_UNKNOWN
};

/*
  Content type:
*/

extern int cgi_content_type;


/*
  Initializes CGI - receives form data (via either "post" or "get" method).
*/

int cgi_init(void);


/*
  Quits the CGI - frees data structures.
*/

void cgi_quit(void);


/*
  Searches a cookie by name and returns its value or NULL.

  cookie_name = string (name) to search for.
*/

const char * cgi_getcookie(const char * cookie_name);


/*
  Counts the number of entries for a given name
  
  field_name = string (name) to search for.
*/

int cgi_getnumentries(const char *field_name);


/*
  Searches for an entry (name) and returns its value or an empty string.
  
  field_name = string (name) to search for.
  n = nth entry for this name
*/

const char * cgi_getentrystr(const char *field_name);
const char * cgi_getnentrystr(const char *field_name, int n);


/*
  Searches for an entry (name) and returns its content-type string or NULL.
  
  field_name = string (name) to search for.
  n = nth entry for this name
*/

const char * cgi_getentrytype(const char *field_name);
const char * cgi_getnentrytype(const char *field_name, int n);



/*
  Searches for an entry (name) and returns its value or 0.
  
  return = value, converted from string to integer.
  field_name = string (name) to search for.
  n = nth entry for this name
*/

int cgi_getentryint(const char *field_name);
int cgi_getnentryint(const char *field_name, int n);



/*
  Searches for an entry (name) and returns its value or 0.0.
  
  return = value, converted from string to double.
  field_name = string (name) to search for.
  n = nth entry for this name
*/

double cgi_getentrydouble(const char *field_name);
double cgi_getnentrydouble(const char *field_name, int n);



/*
  Searches for an entry (name) and returns:
  
  0 if the value is "no" or "off"
  1 if the value is "yes" or "on"
  def if the value is none of those
  
  return = determined by entry's value (yes/on=1, no/off=0, else def)
  field_name = string (name) to search for.
  n = nth entry for this name
*/

int cgi_getentrybool(const char *field_name, int def);
int cgi_getnentrybool(const char *field_name, int def, int n);



/*
  Opens "filename" and displays it to stdout (browser).

  return = -1 on error, 0 on ok.
*/

int cgi_dump_no_abort(const char * filename);



/*
  Opens "filename" and displays it to stdout (browser).
  
  Displays error message to stdout and aborts on error.
*/

void cgi_dump(const char * filename);



/*
  Makes sure an email address is formatted correctly.
  
  Returns 1 on good address, 0 on badly formatted one.
*/

int cgi_goodemailaddress(const char * addr);



/*
  Displays a generic error message and quits (exit(0);).
*/

void cgi_error(const char * reason);


/*
  Returns the english text string for a CGI error.
*/

const char * cgi_strerror(int err);


#ifdef __cplusplus
}
#endif

#endif /* #ifdef CGI_UTIL_H */
