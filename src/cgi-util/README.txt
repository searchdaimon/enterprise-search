README.txt for cgi-util 2.1.5

by: Bill Kendrick <bill@newbreedsoftware.com>
    Mike Simons <msimons@moria.simons-clan.com>

Other contributors listed in "CHANGES.txt"

http://www.newbreedsoftware.com/cgi-util/

June 16, 1999 - October 18, 2002


WHAT IS IT?
-----------
  "cgi-util" is a library which provides a set of C functions you can
  use to create Common Gateway Interface ("CGI") programs.
  Simply call cgi-util's initialization function and send HTML (or any
  other kind of data) out from your program and to the user's web browser.

  cgi-util allows you to grab data sent to your CGI program by way of
  forms or URL-encoded data.  Both "POST" and "GET" methods are handled
  transparently.  It also supports file-upload.  (NOTE: UNDER DEVELOPMENT!)

  cgi-util is probably not the most feature-filled library, but it
  is fast, simple and takes virtually no time to learn how to use.


INSTALLING IT
-------------
  There are a number of ways you can install the cgi-util library.  These
  are discussed in detail in the file "INSTALL".


CREATING A CGI
--------------
  To create a CGI in C using cgi-util, simply follow these simple steps:

    1. Include the cgi-util header file:

           #include "cgi-util.h"

       or, if you installed the header file in a shared place:

           #include <cgi-util.h>


    2. Call cgi-util's initialization function:

           cgiinit();

       Note: The "cgiinit()" function returns a status result.  To be
       safe, you should make sure no errors occured:

           if (cgiinit() != CGIERR_NONE)
             exit(0);


    3. Send a "Content-type" value to the webserver.  Since your
       program's "stdout" stream is connected to your webserver, you can
       just use "printf()" or "write(0,...)":

       Since most CGI's generate an HTML page (versus plain text, graphics
       or some other format), you'll almost always specify the "text/html"
       content type:

           printf("Content-type: text/html\n\n");

       Note: The two "\n"'s at the end produce a blank line, which ends
       the HTTP headers.  If you want to specify other HTTP headers,
       you of course can.  For example:

           printf("Content-type: text/html\n");
           printf("Pragma: no-cache\n\n");

       Simply remember to add that one extra "\n" character at the end
       to produce the blank line to end your HTTP headers.


    4. Use the "cgi_getentry..." family of functions to grab data from the
       web browser.  This data may be coming from an HTML form, or it may
       have been URL-encoded (in other words, the URL that the browser
       was directed to included the path to your CGI program, followed by
       data.  For example: http://site.com/path/to/your.cgi?data...")

       The "cgi_getentry..." family includes:

           char * cgi_getentrystr()
             Returns the string value of a form field.

           int cgi_getentryint()
             Returns the integer value of a form field.

           double cgi_getentrydouble()
             Returns the double (floating point) value of a form field.

           int cgi_getentrybool()
             Returns the boolean value of a form field, depending on whether
             the form string value is "on" or "yes" (true, which returns 1),
             or "off" or "no" (false, which returns 0).


     5. Send HTML data out to the browser based on the input you received.
        Most of the time, you'll probably be using "printf()" to send HTML
        to the browser:

            printf("<html><head><title>...</title></head>\n");
            ...
            printf("</html>\n");

        You may also wish to copy entire files to the resulting page.
        For example, rather than embeding your standard navigation links
        at the bottom of the CGI's page output, you can create a file
        and then use on of cgi-util's "cgi_dump..." functions:

            cgi_dump("file.html");

        If you need to change something in the HTML later, you need only
        edit the HTML file, not the C file.  No recompiling need take place.

        Most of the time, you'll be "dumping" HTML files which you know
        will be readable, since they are technically part of your CGI
        software.  The standard "dump()" function will abort your CGI with
        an HTML error message in the rare case that the file can't be open.

        Hopefully, this will never happen except during development, at
        which point, you simply correct the problem and it should never
        break again.

        If, however, you're trying to open a file which may or may not
        be available, you can use the "cgi_dump_no_abort()" function call
        which does not abort the CGI or display any HTML error message, but
        instead returns an error value:

            if (cgi_dump_no_abort("file.html") != CGIERR_NONE)
              ...

        Finally, another function exists which lets you manually abort your
        CGI with an HTML error message, "cgi_error()":

            cgi_error("reason for error");


     6. When you're done, free up any memory used by this instance of your
        CGI.  The "cgi_quit()" function frees any data allocated by
        the "cgi_init()" function:

            cgi_quit();


FUNCTION DESCRIPTIONS
---------------------
  The following functions are available when using cgi-util:

    int cgi_init(void)
    ------------------
      Receives form data.  It does this by checking environment variables
      (which should be set by the webserver before the CGI process begins)
      to determine how to get the data, and then grabbing it and creating
      a list of name/value pairs.

      The following global cgi-util variables are set:

        int cgi_request_method
        ----------------------
          CGIREQ_NONE if no request method was set (which should never happen),
          CGIREQ_POST if POST method was used, 
          CGIREQ_GET if GET method was used, or
          CGIREQ_UNKNOWN if some other method was used.

        int cgi_num_entries
        -------------------
          The number of name/value pairs received, or
          0 if none or an error occured.

        int cgi_errno
        -------------
          CGIERR_NONE if no errors occured,
          CGIERR_INCORRECT_TYPE if an incorrect "CONTENT_TYPE" was set by the
            server or browser,
          CGIERR_BAD_CONTENT_LENGTH if the "CONTENT_LENGTH" environment
            variable was not set to an integer value,
          CGIERR_CONTENT_LENGTH_DISCREPENCY if the "CONTENT_LENGTH" set does
            not match the amount of data actually received by the CGI,
          CGIERR_UNKNOWN_METHOD if the "REQUEST_METHOD" was not set to
            "POST" or "GET" (the two understood by cgi-util), or
          CGIERR_OUT_OF_MEMORY if space could not be allocated for the
            data being received.

        cgi_entry_type * cgi_entries
        ----------------------------
          This will contain a collection of names/values/types/lengths
            (the number of which is stored in "cgi_num_entries"), or
          NULL if an error occured.

        int cgi_content_type
        --------------------
          CGITYPE_UNKNOWN if an unknown "CONTENT_TYPE" was set,
          CGITYPE_APPLICATION_X_WWW_FORM_URLENCODED if the "CONTENT_TYPE"
            was set to "application/x-www-form-urlencoded" (the typical type),
          CGITYPE_MULTIPART_FORM_DATA if the "CONTENT_TYPE" was set to
            "multipart/form-data" (normal for file-upload).

      The return value of "cgi_init()" is the value of "cgi_errno".

    void cgi_quit(void)
    -------------------
      Clears memory allocated by "cgi_init()".  Since any pointers which
      may be freed are initially set to "NULL", and "free()" does nothing
      to NULL pointers, this can be safely called even if "cgi_init()"
      failed.

      The following variables are set:

        cgi_entry_type * cgi_entries
        ----------------------------
          Set to NULL.

        int cgi_num_entries
        -------------------
          Set to 0.

        int cgi_errno
        -------------
          Set to CGIERR_NONE.

        int cgi_request_method
        ----------------------
          Set to CGIREQ_NONE.

        char * query
        ------------
          Set to NULL.

        int cgi_content_type
        --------------------
          Set to CGITYPE_NONE.

      Since the function is void, nothing is returned.

    char * cgi_getentrystr(char * field_name)
    -----------------------------------------
      This function looks up a name/value pair named by "field_name"
      (the name of some form field) and returns the value as a character
      string pointer.

      In other words, it goes through all "cgi_entries" (from 0 to
      "cgi_num_entries - 1") and checks the "name" of each of them.
      For the first one it finds (if any) that exactly matches the
      string sent to this function ("field_name"), it will return the
      "value".

      This function returns a "NULL" if no name/value pair was found
      that matched the "field_name" requested.

      Example:
      --------
        If an HTML form has a text input box named "name":

            <input type="text" name="name">

        ...and someone types "john doe" into it and submits it to your
        CGI, then the string you receive from the function call:

            cgi_getentrystr("name");

        will be the text "john doe".

    char * cgi_getnentrystr(char * field_name, int n)
    -------------------------------------------------
      Identical to "cgi_getentrystr()", but for forms with multiple
      fields with the same name.  It returns the 'n'th field with
      the name 'field_name'.

      Use "cgi_numentries()" (see below) to determine how many (if any)
      fields have the name you're interested in.

      Example:
      --------
        If an HTML form has a number of checkboxes named "choice":

            <input type="checkbox" name="choice" value="choc">Chocolate
            <input type="checkbox" name="choice" value="vani">Vanilla

        ...you can receive the values ("choc" and/or "vani") for the
        checked items using this in your C code:

            for (i = 0; i < cgi_numentries("choice"); i++)
            {
              ... cgi_getnentrystr("choice", i) ...
            }

        If "n" is larger than or equal the number of fields named
        "field_name", or is less than 0, the global variable
        "cgi_errno" is set to "CGIERR_N_OUT_OF_BOUNDS".

    int cgi_getentryint(char * field_name)
    --------------------------------------
      Like "cgi_getentrystr()", this function looks for a name/value
      pair named by "field_name".  (In fact, this function is simply
      a wrapper around "cgi_getentrystr()".)

      It returns the integer value of the string value from the form.
      (In other words, it would return the integer 10 if the string
      was "10".)

      If the name/value pair is not found or the string is not an
      integer (for example, "abc" would return the integer 0),
      the global variable "cgi_errno" is set to "CGIERR_NOT_INTEGER".

    int cgi_getnentryint(char * field_name, int n)
    ----------------------------------------------
      Identical to "cgi_getentryint()", but for forms with multiple
      fields with the same name.  See "cgi_getnentrystr()" above
      and "cgi_getnumentries()" below.

    int cgi_getentrydouble(char * field_name)
    -----------------------------------------
      This function is more or less identical to "cgi_getentryint()",
      except it returns a double (floating point).

      If the name/value pair is not found or the string is not an
      integer (for example, "abc" would return the double 0.0),
      the global variable "cgi_errno" is set to "CGIERR_NOT_DOUBLE".

    int cgi_getnentrydouble(char * field_name, int n)
    -------------------------------------------------
      Identical to "cgi_getentrydouble()", but for forms with multiple
      fields with the same name.  See "cgi_getnentrystr()" above
      and "cgi_getnumentries()" below.

    int cgi_getentrybool(char * field_name, int def)
    ------------------------------------------------
      This function returns a "1" or a "0" depending on whether
      the value of the name/value pair you are looking for contains
      the text string "on" or "yes", or "off" or "no", respectively.

      If the name/value pair is not found or the string is not an
      integer (for example, "abc"), the global variable "cgi_errno"
      is set to "CGIERR_NOT_BOOL", and the value returned is "def"
      (a default specified when the function is called).

    int cgi_getnentrybool(char * field_name, int def, int n)
    --------------------------------------------------------
      Identical to "cgi_getentrybool()", but for forms with multiple
      fields with the same name.  See "cgi_getnentrystr()" above
      and "cgi_getnumentries()" below.

    int cgi_getnumentries(char * field_name)
    -----------------------------------------
      Returns how many fields were returned with the name "field_name".
      Use to determine what the maximum value for "n" should be when
      calling cgi_getn...() functions.

    const char * cgi_getcookie(const char * cookie_name)
    ----------------------------------------------------
      Searches the "HTTP_COOKIE" environment variable
      (sent by the browser during the HTTP request) for a particular
      cookie, and returns its value as a string.

      If the cookie is not found, this function returns NULL.

      This function sets the following global variable:

        int cgi_errno
        -------------
          CGIERR_NONE if no errors occured.
          CGIERR_NO_COOKIES if HTTP_COOKIE is not set.
          CGIERR_COOKIE_NOT_FOUND if the cookie wasn't found.
          CGIERR_OUT_OF_MEMORY if it could not create temporary space.

    int cgi_dump_no_abort(char * filename)
    --------------------------------------
      This opens a file and sends it to "stdout" (file descriptor 0),
      which results in the file being sent to the browser.  This is a
      very good way to include standard HTML headers and footers in
      your CGI.  Also, by not embeding the HTML into your C source,
      you no longer need to recompile the CGI after editing that HTML.
      Changes take effect immediately.

      This function sets the following global variable:

        int cgi_errno
        -------------
          CGIERR_NONE if no errors occured.
          CGIERR_CANT_OPEN if the file could not be opened.  (You should
            look at the C global "errno" to determine the exact error.)

      Once the file has been sent out (if it was), the function returns
      with the same value it set "cgi_errno" to.

    void cgi_dump(char * filename)
    ------------------------------
      This function does more or less the same as "cgi_dump_no_abort()"
      (in fact, it's actually a wrapper around that function).

      However, if it cannot open the file, rather than return with an
      error code, this function sends HTML text out stating that it can't
      open the file, including the C "strerror()" description of the error.
      It then aborts your CGI by calling "exit(0);".

      Since most of the time you will be including HTML files which are
      under your control (ie, they are part of the CGI package you are
      creating), you will most often wish to use this function, since it
      is simpler than "cgi_dump_no_abort()" and your HTML files should
      always be available to your CGI.

    void cgi_error(char * reason)
    -----------------------------
      This function displays an HTML error message, starting with the word
      "Error" as a level one header ("<h1>") and followed by the string you
      provide (the actual reason an error occured).

      This function then calls "exit(0);" to abort your CGI.

    int cgi_goodemailaddress(char * addr)
    -------------------------------------
      This function does its best to make sure that a string contains
      a valid-looking e-mail address.  (This is useful for when you
      get users who fill out a form and refuse to enter their e-mail
      address, or they don't understand what their own e-mail address is.
      For example, someone with the address "abc@aol.com" might think
      their address is "abc" or "abc@aol" or "abc.aol.col" or even
      "http://abc@aol.com".  A good rule: never trust users.)

      A valid e-mail address is in the form:

        text@text.text[.text...]

      In other words, alphanumeric characters ("a-z", "A-Z", "0-9"),
      'dashes' ("-"), underscores ("_") and 'dots' ("."), with exactly one
      'at' ("@") in the middle, and at least one 'dot' appearing after
      the 'at.'  'Dots' and 'ats' are not allowed at the beginning or
      end of the string.

      "cgi_goodemailaddress" returns a "1" if the address appears to be
      in the right format (this does NOT necessarily mean it is a VALID
      address), or "0" if it is not.

    char * cgi_strerror(int err)
    ----------------------------
      Like C's "strerror()" function which returns the text equivalent of
      an "errno" error code, this function returns the text equivalent of
      one of cgi-util's "cgi_errno" error code.


ERROR CODES
-----------
  The many error codes that cgi-util will set "cgi_errno" to are described
  above, but here's a reference:

    CGIERR_NONE
    -----------
      No error occured

    CGIERR_NOT_INTEGER
    ------------------
      The field looked-up by cgi_getentryint() did not exist, or its
      value was not an integer.

    CGIERR_NOT_DOUBLE
    -----------------
      The field looked-up by cgi_getentrydouble() did not exist, or its
      value was not a double.

    CGIERR_NOT_BOOL
    ---------------
      The field looked-up by cgi_getentrybool() did not exist, or its
      value was not "yes", "on", "no" or "off".

    CGIERR_UNKNOWN_METHOD
    ---------------------
      The "REQUEST_METHOD" environment variable was not set to
      "POST" or "GET" (the two understood by cgi-util).

    CGIERR_INCORRECT_TYPE
    ---------------------
      The "CONTENT_TYPE" environment variable was not set to
      "application/x-www-form-urlencoded".

    CGIERR_BAD_CONTENT_LENGTH
    -------------------------
      The "CONTENT_LENGTH" environment variable was not set to
      an integer value.

    CGIERR_CONTENT_LENGTH_DISCREPANCY
    ---------------------------------
      The "CONTENT_LENGTH" environment variable was set to a value
      different from the actual size of data received by cgi_init().

    CGIERR_CANT_OPEN
    ----------------
      The cgi_dump_no_abort() function could not open the file
      specified.  (Check C's "errno" value for the exact reason.)

    CGIERR_OUT_OF_MEMORY
    --------------------
      Space could not be allocated for the data being received by
      cgi_init().


THE TEST PROGRAMS
-----------------
  To understand the test program, first open the "test.html" or
  "filetest.html" HTML file in a web browser.  (You need to open it via the
  HTTP protocol, not simply opening it as a file!)

  With "test.html", you'll notice the following on the page:

    * A type-in field labelled "Name?"
    * A type-in field labelled "Age?"
    * A pull-down menu labelled "Sex?"
    * A submit button labelled "Ok"

  I won't go into the details of creating a form here, since there are
  many, many places where you can learn this.  (If you look at the source
  of "test.html", you'll see some comments which explain what is going on.)

  When you fill out the form and click the "Ok" submit button, the CGI
  will be invoked and you'll see something similar to:

    Hello.
    name=john doe
    age=55
    sex=Male
    Goodbye!

  As you can see, the input you place into the form is echoed back to you
  by this CGI.  To see how this is done, simply look at the source code:
  "test.c"!


  With "filetest.html", you'll see a simpler form:

    * A type-in filed labelled "Name?"
    * A file browse field labelled "File?" (typically these fields appear as
      a type-in form with a "Browse" button next to it)
    * A sumbit button labelled "Ok"

  Select a file from your local filesystem (type it into the type-in field
  or use the "Browse" button, for example), and then submit the form.
  The CGI will be invoked and you'll see something similar to:

    Hello.
    name=john doe
    filename=foo.bar
    This file is 10234 bytes long.
    Goodbye!

  As you can see, the file you uploaded using the form has been processed,
  and the size of the file (in bytes) is displayed by the CGI.  See
  "filetest.c" to see how this CGI works.


THE END
-------
  Hopefully this library will come in useful.  If you have questions or
  comments, please direct them to me:

    bill@newbreedsoftware.com

  If you wish to report a bug, use BugTrack:

    http://www.newbreedsoftware.com/bugtrack/


THANKS FOR USING cgi-util!

End of README.txt
