/*==========================================================================
 * wrfmwrap.c -- A simple setuid wrapper to run WebRFM as the 'REMOTE_USER'.
 * Version: 0.2a
 * Copyright (C) 1999  Yoram Last (ylast@mindless.com)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *==========================================================================
 *
 * Usage:
 *
 * 1) Set HTTPD_UID below to the UID your web server normally runs as.
 * 2) Set WEBRFM_SCRIPT below to the full path of your "main WebRFM script".
 * 3) Set UID_MIN and GID_MIN below to the desired minimal UID and GID
 *   that would be allowed to spawn WebRFM through this wrapper.
 * 4) Compile the resulting code to get an executable. For example, 
 *   gcc -o wrfmwrap wrfmwrap.c
 *   should work if you have gcc on your system.
 * 5) Move the resulting executable to its final destination. This should
 *   be a directory from which it can be executed as a setuid CGI program.
 * 6) Make sure the executable is owned by 'root'. Change its mode to be
 *   setuid and readable and executable by everybody.
 *   chmod 04755 wrfmwrap
 *   should do it. Rename the executable to whatever you like it to be
 *   called.
 *
 * Please see the accompanying documentation for further usage information.
 *
 *==========================================================================
 *
 * WARNING: Once you set up this program as a setuid root program, you are
 * basically allowing your HTTPD user (the one having HTTPD_UID as its UID)
 * to run WebRFM with an arbitrary UID. Therefore, you must make sure that
 * arbitrary users can't run programs with HTTPD_UID as their UID. This
 * means that if you allow users to run CGI programs, you must use some
 * wrapper such as the Apache 'suEXEC' wrapper that would make CGI programs
 * run in the user context of the user owning the program (and not in the
 * user context of the server).
 * 
 *==========================================================================
 */

/*
 * HTTPD_UID -- Define as the UID under which your web server normally
 *               runs.  This is the only UID that will be allowed to
 *               execute WebRFM through this wrapper.
 */
#ifndef HTTPD_UID
#define HTTPD_UID 500
#endif

/*
 * WEBRFM_SCRIPT -- Define as the full path to the main WebRFM script.
 */
#ifndef WEBRFM_SCRIPT
#define WEBRFM_SCRIPT "/bin/umount"
#endif


/*
 ***********************************************************************
 *
 * NOTE! : DO NOT edit the code below unless you know what you are doing.
 *         Editing this code might open up your system in unexpected 
 *         ways to would-be crackers. Alter it at your own risk!
 *
 ***********************************************************************
*/


#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <grp.h>
#include <sys/types.h>

int mysetenv(const char *name, const char *value)
{
  char *envstr;
  int strlength;

  if (
      (strlength = strlen(name) + strlen(value) + 2) &&
      ((envstr = malloc(strlength)) != NULL) &&
      (sprintf(envstr, "%s=%s", name, value) == strlength - 1)
     )
  {
    return putenv(envstr);
  }
  return 1;
}

int main(int argc, char **argv)
{
  uid_t uid;
  uid_t euid;
  char *user_name;

  uid = getuid();
  euid = geteuid();

    if (uid != HTTPD_UID) {
	fprintf(stderr,"DAVRAP: uid not httpd uid (%i), but %i.\n",HTTPD_UID,uid);
    }	
    else if (euid != 0) {
	fprintf(stderr,"DAVRAP: euid not 0, but %i.\n",euid);
    }
    else if (setgid(0) != 0) {
        fprintf(stderr,"DAVRAP: Can't setgid()\n");
        perror("setgid");
    }
    else if (setuid(0) != 0) {
        fprintf(stderr,"DAVRAP: Can't setuid()\n");
    }
    else {
	fprintf(stderr,"DAVRAP: OK, exe %s\n",WEBRFM_SCRIPT);
            execv(WEBRFM_SCRIPT,argv);
    }

}
