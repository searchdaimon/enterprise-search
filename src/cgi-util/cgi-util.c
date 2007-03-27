/*
  cgi-util.c
  
  version 2.1.7
  
  by Bill Kendrick <bill@newbreedsoftware.com>
  and Mike Simons <msimons@moria.simons-clan.com>
  
  New Breed Software
  http://www.newbreedsoftware.com/cgi-util/
  
  April 6, 1996 - October 18, 2002
*/


#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif

#include <string.h>
#include "cgi-util.h"


/* Globals: */

cgi_entry_type *cgi_entries = NULL;
int cgi_num_entries = 0;
int cgi_errno = CGIERR_NONE;
int cgi_request_method = CGIREQ_NONE;
int cgi_content_type = CGITYPE_NONE;
char * cgi_query = NULL;


/* English error strings: */

char * cgi_error_strings[CGIERR_NUM_ERRS] = {
  "", "Not an integer", "Not a double", "Not a boolean",
  "Unknown method", "Incorrect Content Type",
  "NULL Query String", "Bad Content Length",
  "Content Length Discrepancy", "No Cookies", "Cookie Not Found",
  "Not That Many"
};


/* Debugging.  If set, this library will attempt to append debugging
   information to a file named "debug.txt" in the current directory. */

/* #define DEBUG */
//#define DEBUG


/* Internal use: Write debugging stuff to a file */

static void debug(char * str1, char * str2)
{
#ifdef DEBUG
  FILE * fi;
  
  fi = fopen("/tmp/debug.txt", "a");
  if (fi != NULL)
    {
      fprintf(fi, "%s:%s\n", str1, str2);
      fclose(fi);
    }
#endif
}


/* Converts hexadecimal to decimal (character): */

char x2c(char *what)
{
  register char digit;
  
  digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
  return (digit);
}


/* Unescapes "%"-escaped characters in a query: */

void unescape_url(char *url)
{
  register int x,y,len;
  
  debug("url",url);
  len = strlen(url);
  
  for (x=0, y=0; url[y]; ++x, ++y)
    {
      if ((url[x] = url[y]) == '%'
	  && y < len - 2)   /* 2.0.4 - MJ Pomraning (pilcrow@mailbag.com) */
	{
	  url[x] = x2c(&url[y+1]);
	  y+=2;
        }
    }
  url[x] = '\0';
}


/* Converts pluses back to spaces in a query: */

void plustospace(char *str)
{
  register int x;
  
  for (x=0; str[x]; x++)
    if (str[x] == '+')
      str[x] = ' ';
}



/* Internal use: Read a line and return its length: */

int lineread(FILE * stream, char * buf, int count)
{
  fgets(buf, count, stream);
  
  if (!feof(stream))
    return(strlen(buf));
  else
    return(0);
}



/* Initialize the CGI.  Grab data from the browser and prepare it for us. */

int cgi_init(void)
{
  int cl, i, in_multipart_headers, which_entry, length_gotten, in_name;
  char * boundary;
  

  
  /* Default, no errors, no name/value pairs ("entries"): */
  
  cgi_errno = CGIERR_NONE;
  cgi_num_entries = 0;
  length_gotten = 0;
  
  
  /* Check for REQUEST_METHOD (set by HTTP server): */
  
  if (getenv("REQUEST_METHOD") == NULL)
    {
      /* None set?  Assume the user is invoking the CGI from a shell prompt
	 (for debugging): */
      
      cgi_request_method = CGIREQ_NONE;
    }
  else
    {
      /* Determine the exact request method, and grab the data (if any)
	 in the appropriate manner: */
      
      if (strcmp(getenv("REQUEST_METHOD"), "POST") == 0)
	{
	  /* Post method (data is sent to us via "stdin"): */
	  
	  cgi_request_method = CGIREQ_POST;
	  
	  
	  if (getenv("CONTENT_TYPE") == NULL)
	    {
	      /* Content type is not set! */
	      
	      cgi_errno = CGIERR_INCORRECT_TYPE;
	      cgi_content_type = CGITYPE_UNKNOWN;
	      
	      return(cgi_errno);
	    }
	  else if (strstr(getenv("CONTENT_TYPE"),
			  "application/x-www-form-urlencoded") == 
                   getenv("CONTENT_TYPE"))
	    {
	      cgi_content_type = CGITYPE_APPLICATION_X_WWW_FORM_URLENCODED;
	      
	      
	      /* How much data do we expect? */
	      
	      if (getenv("CONTENT_LENGTH") == NULL ||
		  sscanf(getenv("CONTENT_LENGTH"), "%d", &cl) != 1)
		{
		  cgi_errno = CGIERR_BAD_CONTENT_LENGTH;
		  return(cgi_errno);
		}
	      
	      
	      /* Create space for it: */
	      
	      cgi_query = malloc(cl + 1);
	      /* 2.0.1 - Tadek Orlowski (orlowski@epnet.com) ... "+1" */
	      
	      if (cgi_query == NULL)
		{
		  cgi_errno = CGIERR_OUT_OF_MEMORY;
		  return(cgi_errno);
		}
	      
	      
	      /* Read it in: */
	      
	      fgets(cgi_query, cl + 1, stdin);
	      
	      
	      /* Verify that we got as much data as we expected: */
	      
	      if (strlen(cgi_query) != cl)
		cgi_errno = CGIERR_CONTENT_LENGTH_DISCREPANCY;
	    }
	  else if (strstr(getenv("CONTENT_TYPE"),
			  "multipart/form-data") == getenv("CONTENT_TYPE"))
	    {
	      cgi_content_type = CGITYPE_MULTIPART_FORM_DATA;
	      
	      cgi_query = malloc(2050);
	      if (cgi_query == NULL)
		{
		  cgi_errno = CGIERR_OUT_OF_MEMORY;
		  return(cgi_errno);
		}
	      
	      
	      /* Determine the boundary string: */
	      
	      if (strstr(getenv("CONTENT_TYPE"),
			 "boundary=") == NULL)
		{
		  cgi_errno = CGIERR_NO_BOUNDARY;
		  return(cgi_errno);
		}
	      
	      boundary = strdup(strstr(getenv("CONTENT_TYPE"),
				       "boundary=") + 9);
	      
	      debug("boundary", boundary);
	      
	      
	      /* Read in until there's no more: */
	      
	      in_multipart_headers = 0;
	      which_entry = -1;
	      
	      do
		{
		  length_gotten = lineread(stdin, cgi_query, 2048);
		  
		  debug("cgi_query", cgi_query);
		  
		  if (length_gotten > 0)
		    {
		      if (strstr(cgi_query, boundary) == cgi_query + 2 &&
			  cgi_query[0] == '-' && cgi_query[1] == '-')
			{
			  /* We got a boundary! */
			  
			  in_multipart_headers = 1;
			  which_entry = -1;
			}
		      else /* (Not a boundary) */
			{
			  if (in_multipart_headers == 1)
			    {
			      /* We had just got a boundary, read headers: */
			      
			      if (cgi_query[0] == '\r' || cgi_query[0] == '\n')
				{
				  /* Blank line, end of headers: */
				  
				  in_multipart_headers = 0;
				}
			      else /* (Not a blank line) */
				{
				  /* What kind of header is it? */
				  
				  if (strstr(cgi_query,
					     "Content-Disposition: ") ==
				      cgi_query)
				    {
				      /* Content-disposition: */
				      
				      /* For now, just look for "name=": */
				      
				      if (strstr(cgi_query, "name=\"") != NULL)
					{
					  /* Add a new entry: */
					  
					  which_entry = cgi_num_entries;
					  cgi_num_entries++;
					  
					  
					  /* Make more room: */
					  
					  cgi_entries =
					    realloc(cgi_entries,
						    sizeof(cgi_entry_type) *
						    cgi_num_entries);
					  
					  if (cgi_entries == NULL)
					    {
					      cgi_errno = CGIERR_OUT_OF_MEMORY;
					      return(cgi_errno);
					    }
					  
					  
					  /* Fill in the name slot: */
					  
					  cgi_entries[which_entry].name =
					    strdup(strstr(cgi_query,
							  "name=\"") +
						   6);
					  
					  
					  /* Truncate after quote: */
					  
					  if (strchr(cgi_entries[which_entry].
						     name, '\"') != NULL)
					    {
					      strcpy(strchr(cgi_entries
							    [which_entry].name,
							    '\"'), "\0");
					    }
					  
					  
					  /* Set default content-type: */
					  
					  cgi_entries[which_entry].
					    content_type =
					    "application/octet-stream";
					  
					  
					  /* Set default content-length: */
					  
					  cgi_entries[which_entry].
					    content_length = 0;
					  
					  
					  /* Set default value: */
					  
					  cgi_entries[which_entry].val =
					    strdup("");
					  
					  
					  debug("entry.name",
						cgi_entries[which_entry].name);
					}
				    }
				  else if (strstr(cgi_query,
						  "Content-Type: ") ==
					   cgi_query)
				    {
				      /* Content-type: */
				      
				      cgi_entries[which_entry].content_type =
					strdup(strstr(cgi_query,
						      "Content-Type: ") +
					       14);
				      
				      debug("entry.content_type",
					    cgi_entries[which_entry].
					    content_type);
				    }
				}
			    }
			  else /* in_multipart_headers == 0 */
			    {
			      /* If we're recording into a particular
				 entry, copy the data: */
			      
			      if (which_entry != -1)
				{
				  /* Make more room: */
				  
				  cgi_entries[which_entry].val =
				    realloc(cgi_entries[which_entry].val,
					    strlen(cgi_entries[which_entry].
						   val) + length_gotten + 1);
				  
				  if (cgi_entries[which_entry].val == NULL)
				    {
				      cgi_errno = CGIERR_OUT_OF_MEMORY;
				      return(cgi_errno);
				    }
				  
				  
				  /* Append the data: */
				  
				  memcpy(cgi_entries[which_entry].val +
					 (cgi_entries[which_entry].
					  content_length),
					 cgi_query, length_gotten);

				  cgi_entries[which_entry].content_length =
				    (cgi_entries[which_entry].content_length +
				     length_gotten);
				}
			    }
			}
		    }
		}
	      while (length_gotten > 0);
	      
	      free(cgi_query);
	    }
	  else
	    {
	      /* Content type is unrecognized! */
	      
	      cgi_errno = CGIERR_INCORRECT_TYPE;
	      cgi_content_type = CGITYPE_UNKNOWN;
	      
	      return(cgi_errno);
	    }
	}
      else if (strcmp(getenv("REQUEST_METHOD"), "GET") == 0)
	{
	  /* For now, assume Content Type of
	     "application/x-www-form-urlencoded"
	     (Is this a bad assumption?) */
	  
	  cgi_content_type = CGITYPE_APPLICATION_X_WWW_FORM_URLENCODED;
	  
	  
	  /* GET method (data sent via "QUERY_STRING" env. variable): */
	  
	  cgi_request_method = CGIREQ_GET;
	  
	  
	  /* Get a pointer to the data: */
	  
	  cgi_query = getenv("QUERY_STRING");
	  
	  if (cgi_query == NULL)
	    {
	      /* Does the "QUERY_STRING" env. variable not exist!? */
	      
	      /* cgi_errno = CGIERR_NULL_QUERY_STRING; */
	      
	      /* return(cgi_errno); */
	      
	      cl = 0;
	      cgi_query = "";
	    }
	  else
	    {
	      /* Determine the content length by seeing how big the
		 string is: */
	      
	      cl = strlen(cgi_query);
	    }
	}
      else
	{
	  /* Something else? We can't handle it! */
	  
	  cgi_request_method = CGIREQ_UNKNOWN;
	  cgi_errno = CGIERR_UNKNOWN_METHOD;
	  cgi_num_entries = 0;
	  
	  return(cgi_errno);
	}      
      
      
      if (cgi_content_type != CGITYPE_MULTIPART_FORM_DATA)
	{
	  /* How many entries (name/value pairs) do we need to
	     allocate space for? (They should be separated by "&"'s) */
	  
	  cgi_num_entries = 0;
	  
	  for (i = 0; i <= cl; i++)
	    if (cgi_query[i] == '&' || cgi_query[i] == '\0')
	      cgi_num_entries++;
	  
	  
	  /* Allocate the space for that many structures: */
	  
	  cgi_entries = malloc(sizeof(cgi_entry_type) * cgi_num_entries);
	  if (cgi_entries == NULL)
	    {
	      cgi_errno = CGIERR_OUT_OF_MEMORY;
	      return(cgi_errno);
	    }
	  
	  
	  /* Grab each name/value pair: */
	  
	  cgi_num_entries = 0;
	  
	  
	  /* (Begin with the first half of the first pair): */
	  
	  if (cgi_query[0] != '\0' && cgi_query[0] != '&')
	    {
	      cgi_entries[0].name = cgi_query;
	      cgi_entries[0].content_type = "text/html";
	    }
	  
	  
	  /* Go through the entire string of characters: */
	  
	  in_name = 1;
	  for (i = 0; i <= cl; i++)
	    {
	      if (cgi_query[i] == '&')
		{
		  /* "&" represents the end of a name/value pair: */
		  
		  cgi_entries[cgi_num_entries].name = cgi_query + i + 1;
		  cgi_entries[cgi_num_entries].content_type = "text/html";
		  cgi_query[i] = '\0';
                  in_name = 1;
		}
	      else if (cgi_query[i] == '=' && in_name)
		{
		  /* "=" is the end of the name half of a name/value pair: */
		  
		  cgi_entries[cgi_num_entries].val = cgi_query + i + 1;
		  
		  /*  plustospace(cgi_entries[cgi_num_entries].val);
		      unescape_url(cgi_entries[cgi_num_entries].val); */
		  
		  cgi_num_entries++;
		  in_name = 0;
		  
		  cgi_query[i] = '\0';
		}
	    }
	  
	  for (i = 0; i < cgi_num_entries; i++)
	    {
	      plustospace(cgi_entries[i].val);
	      unescape_url(cgi_entries[i].val);
	    }
	  
	}
      
      
      /* Fix any NULL strings to be empty strings */
      /* 2.0.4 - MJ Pomraning (pilcrow@mailbag.com) */
      
      for (i = 0; i < cgi_num_entries; i++)
	{
	  if (cgi_entries[i].name == NULL)
	    cgi_entries[i].name = "";
	  if (cgi_entries[i].val == NULL)
	    cgi_entries[i].val = "";
	}
    }
  
  return(CGIERR_NONE);
}


/* Free up memory that was allocated when we called "cgi_init()": */

void cgi_quit(void)
{
  if (cgi_request_method == CGIREQ_NONE ||
      cgi_request_method == CGIREQ_UNKNOWN)
    {
      /* Nothing to do! */
    }
  else
    {
      if (cgi_request_method == CGIREQ_POST)
	{
	  /* Was it POST method?  Free the data we had read from "stdin" */
	  
	  free(cgi_query);
	}


      /* Free the entry structures themselves: */
      
      free(cgi_entries);
    }
  
  cgi_entries = NULL;
  cgi_num_entries = 0;
  cgi_errno = CGIERR_NONE;
  cgi_request_method = CGIREQ_NONE;
  cgi_query = NULL;
  cgi_content_type = CGITYPE_NONE;
}


/* Grab a cookie, if it exists.  Return NULL if it doesn't: */
/* (Based on code by Pete Cassidy (pcassidy@iol.ie) - May 10, 2000) */
/* (Restructured by Chris Wareham (chris.wareham@catchword.com) -
    Nov 1, 2000) */


const char *cgi_getcookie(const char *cookie_name)
{
  char *cookieval, *cookies, *name, *value;
  
  
  cookieval = NULL;
  
  /* get raw cookie data */
  
  if(!(cookies = getenv("HTTP_COOKIE")))
    {
      /* no cookies */
      
      cgi_errno = CGIERR_NO_COOKIES;
      
      return NULL;
    }
  

  /* strtok() is destructive, so make a copy of the raw cookie data */
  
  if(!(cookies = strdup(cookies)))
    {
      cgi_errno = CGIERR_OUT_OF_MEMORY;
      return NULL;
    }
  
  /* tokenize the cookies and check for the one we're looking for */
  
  for(name = strtok(cookies, ";"); name; name = strtok(NULL, ";"))
    {
      /* ignore any leading whitespace */
      
      while(isspace((unsigned char)*name))
	name++;
      
      
      /* '=' signifies the end of the "name" portion of the cookie */
      
      value = strchr(name, '=') + 1;
      *(value - 1) = '\0';

      
      /* see if this is our cookie */
      
      if(!strcmp(name, cookie_name))
	{
	  /* copy the "value" portion of the cookie */
	  
	  if(!(cookieval = strdup(value)))
	    {
	      cgi_errno = CGIERR_OUT_OF_MEMORY;
	      return NULL;
	    }
	  break;
        }
    }
  
  /* free the temporary copy of the raw cookie data */
  
  free(cookies);
  
  /* set the appropriate error based on whether we found the cookie or not */
  
  if(!cookieval)
    cgi_errno = CGIERR_COOKIE_NOT_FOUND;
  else
    cgi_errno = CGIERR_NONE;
  
  return cookieval;
}


#ifdef SKIP_JUNK

/* ------------------------------------------------------------------------- */
/* THIS IS THE OLD cgi_getcookie(), AND IS CURRENTLY DISABLED */

const char * cgi_getcookie(const char * cookie_name)
{
  char * cookieval, * tmpcookie, * rawcookie, * left, * right;
  int done;


  /* Get raw cookie data: */

  rawcookie = getenv("HTTP_COOKIE");
  if (rawcookie == NULL)
    {
      /* No cookies at all?  The cookie we want can't exist! */

      cgi_errno = CGIERR_NO_COOKIES;
      return(NULL);
    }


  /* Strtok is destructive, so make a temporary copy of the raw cookie data: */

  tmpcookie = malloc(sizeof(char) * (strlen(rawcookie) + 1));
  if (tmpcookie == NULL)
    {
      cgi_errno = CGIERR_OUT_OF_MEMORY;
      return(NULL);
    }

  strcpy(tmpcookie, rawcookie);


  /* Tokenize out all cookies and check for the one we're looking for: */

  left = strtok(tmpcookie, ";");
  cookieval = NULL;
  done = 0;

  do
    {
      /* Grab the righthand size of the current cookie pair's "=" sign: */

      right = strchr(left, '=') + (1 * sizeof(char));


      /* Change the "=" into a NULL character, to get the lefthand side: */

      *strchr(left, '=') = '\0';


      /* See if this is our cookie: */

      if (strcmp(left, cookie_name) == 0)
	{
          /* If so, set our return-string to the value (righthand side): */

          cookieval = malloc(sizeof(char) * (strlen(right) + 1));
          if (cookieval == NULL)
            {
              cgi_errno = CGIERR_OUT_OF_MEMORY;
              return(NULL);
            }

          strcpy(cookieval, right);
          done = 1;
	}


      /* Jump to next cookie: */

      if (!done)
	{
          left = strtok(NULL, ";");
          if (left == NULL)
            {
              /* No more to parse? */

              done = 1;
            }
          else
            {
              /* Skip the extra space: */

              left++;
            }
	}
    }
  while (!done);


  /* Free the temporary copy of the raw cookie data: */

  free(tmpcookie);


  /* Return the cookie value (which may be NULL if we never found it): */

  if (cookieval == NULL)
    cgi_errno = CGIERR_COOKIE_NOT_FOUND;
  else
    cgi_errno = CGIERR_NONE;

  return(cookieval);
}

/* ------------------------------------------------------------------------- */

#endif


/* Count number of entries named 'field_name': */
/* Stephen Woodbridge <woodbri@swoodbridge.com> */

int cgi_getnumentries(const char *field_name)
{
  int cnt, i;
  
  cnt = 0;
  
  for (i = 0; i < cgi_num_entries; i++)
    {
      if (strcmp(cgi_entries[i].name, field_name) == 0)
	cnt++;
    }
  
  return cnt;
}



/* Grab a value and return it as a string: */

const char * cgi_getentrystr(const char *field_name)
{
  return cgi_getnentrystr(field_name, 0);
}


/* Grab a value and return it as a string: */

const char * cgi_getnentrystr(const char *field_name, int n)
{
  int x;
  int cnt;
  
  cnt = 0;
  
  if (n < 0 || n > cgi_getnumentries(field_name))
    {
      cgi_errno = CGIERR_N_OUT_OF_BOUNDS;
      return(NULL);
    }
  
  if (cgi_request_method != CGIREQ_NONE)
    {
      /* Look for the name: */
      
      for (x = 0; x < cgi_num_entries; x++)
	{
	  if (strcmp(cgi_entries[x].name, field_name) == 0)
	    {
	      if (cnt == n)
		return (cgi_entries[x].val);
	      
	      cnt++;
	    }
	}
      
      return(NULL);
    }
  else
    {
      /* printf("CGI-UTIL: \"%s\" ? ", field_name);
      fgets(buf, 512, stdin);
      buf[strlen(buf) - 1] = '\0'; */
      
      return(NULL);
    }
}


/* Grab a content-type and return it as a string: */

const char * cgi_getentrytype(const char *field_name)
{
  return cgi_getnentrytype(field_name, 0);
}


/* Grab a content-type and return it as a string: */

const char * cgi_getnentrytype(const char *field_name, int n)
{
  int x;
  int cnt;
  
  cnt = 0;


  if (n < 0 || n > cgi_getnumentries(field_name))
    {
      cgi_errno = CGIERR_N_OUT_OF_BOUNDS;
      return(NULL);
    }
  
  
  if (cgi_request_method != CGIREQ_NONE)
    {
      /* Look for the name: */
      
      for (x = 0; x < cgi_num_entries; x++)
	{
	  if (strcmp(cgi_entries[x].name, field_name) == 0)
	    {
	      if (cnt == n)
		return (cgi_entries[x].content_type);
	      
	      cnt++;
	    }
	}
      
      return(NULL);
    }
  else
    return(NULL);
}


/* Grab a value and return it as an integer: */

int cgi_getentryint(const char *field_name)
{
  return cgi_getnentryint(field_name, 0);
}


/* Grab a value and return it as an integer: */

int cgi_getnentryint(const char *field_name, int n)
{
  int v;
  
  v = 0;
  
  if (cgi_getentrystr(field_name) != NULL)
    {
      if (sscanf(cgi_getnentrystr(field_name, 0), "%d", &v) != 1)
	cgi_errno = CGIERR_NOT_INTEGER;
    }
  else
    cgi_errno = CGIERR_NOT_INTEGER;
  
  return(v);
}


/* Grab a value and return it as a double: */

double cgi_getentrydouble(const char *field_name)
{
  return cgi_getnentrydouble(field_name, 0);
}


/* Grab a value and return it as a double: */

double cgi_getnentrydouble(const char *field_name, int n)
{
  double v;
  
  v = 0;
  
  if (cgi_getentrystr(field_name) != NULL)
    {
      if (sscanf(cgi_getnentrystr(field_name, 0), "%lf", &v) != 1)
	cgi_errno = CGIERR_NOT_DOUBLE;
    }
  else
    cgi_errno = CGIERR_NOT_DOUBLE;
  
  return(v);
}


/* Grab a value and return it as a boolean (depending on if the
   value was "yes", "on" or "true", or "no", "off" or "false"): */

int cgi_getentrybool(const char *field_name, int def)
{
  return cgi_getnentrybool(field_name, def, 0);
}


/* Grab a value and return it as a boolean (depending on if the
   value was "yes", "on" or "true", or "no", "off" or "false"): */

int cgi_getnentrybool(const char *field_name, int def, int n)
{
  const char * temp;
  int v, cnt;
  
  
  cnt = 0;
  

  if (n < 0 || n > cgi_getnumentries(field_name))
    {
      cgi_errno = CGIERR_N_OUT_OF_BOUNDS;
      return(def);
    }
  
  
  /* Assume the default: */
  
  v = def;
  
  
  /* Get the value (if any): */
  
  temp = cgi_getnentrystr(field_name, n);
  
  if (temp != NULL)
    {
      if (strcasecmp(temp, "yes") == 0 ||
	  strcasecmp(temp, "on") == 0 ||
	  strcasecmp(temp, "true") == 0)
	{
	  /* A "yes" or "on" is a 1: */
	  
	  v = 1;
	}
      else if (strcasecmp(temp, "no") == 0 ||
	       strcasecmp(temp, "off") == 0 ||
	       strcasecmp(temp, "false") == 0)
	{
	  /* A "no" or "off" is a 0: */
	  
	  v = 0;
	}
      else if (temp[0] != 0)
	{
	  /* We got something, but not "yes", "on", "no" or "off": */
	  
	  cgi_errno = CGIERR_NOT_BOOL;
	}
    }
  else
    cgi_errno = CGIERR_NOT_BOOL;
  
  
  return(v);
}


/* Open a file and send it to "stdout" (the browser): */
/* (Returns an error if we can't open the file) */

int cgi_dump_no_abort(const char * filename)
{
  FILE * fi;
  int c;
  
  
  cgi_errno = CGIERR_NONE;
  
  
  /* Open the file: */
  
  fi = fopen(filename, "r");
  if (fi == NULL)
    cgi_errno = CGIERR_CANT_OPEN;
  else
    {  
      /* Read data and push it to "stdout": */
      
      do
	{
	  c = fgetc(fi);
	  if (c != EOF)
	    fputc(c, stdout);
	}
      while (c != EOF);
      
      fclose(fi);
    }
  
  return(cgi_errno);
}


/* Open a file and send it to "stdout" (the browser): */
/* (Displays an error message and quits the CGI if we can't open the file) */

void cgi_dump(const char * filename)
{
  if (cgi_dump_no_abort(filename) != CGIERR_NONE)
    {
      printf("Can't open %s - %s\n", filename, strerror(errno));
      exit(0);
    }
}


/* Display a simple error message and quit the CGI: */

void cgi_error(const char * reason)
{
  printf("<h1>Error</h1>\n");
  printf("%s\n", reason);
  
  exit(0);
}


/* Returns whether or not an e-mail address appears to be in the correct
   syntax ("username@host.domain"): */

int cgi_goodemailaddress(const char * addr)
{
  int i;
  
  
  /* No "@".. what? */
  
  if (strchr(addr, '@') == NULL)
    return 0;
  
  
  /* "@" or "." at the end or beginning? */
  
  if (addr[strlen(addr - 1)] == '@' ||
      addr[strlen(addr - 1)] == '.' ||
      addr[0] == '@' || addr[0] == '.')
    return 0;
  
  
  /* No "." after the "@"?  More than one "@"? */
  
  if (strchr(strchr(addr, '@'), '.') == NULL ||
      strchr(strchr(addr, '@') + 1, '@') != NULL)
    return 0;
  
  
  /* Any illegal characters within the string? */
  
  for (i = 0; i < strlen(addr); i++)
    {
      if (isalnum((unsigned char)addr[i]) == 0 &&
	  addr[i] != '.' && addr[i] != '@' && addr[i] != '_' &&
	  addr[i] != '-')
	return(0);
    }
  
  
  /* Must be ok... */
  
  return 1;
}


/* Returns the English string description for a particular cgi-util error
   value: */

const char * cgi_strerror(int err)
{
  if (err < 0 || err > CGIERR_NUM_ERRS)
    return("");
  else
    return(cgi_error_strings[err]);
}
