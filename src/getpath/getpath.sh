#! /bin/sh
# This is a shell archive.  Remove anything before this line, then unpack
# it by saving it into a file and typing "sh file".  To overwrite existing
# files, type "sh file -c".  You can also feed this as standard input via
# unshar, or by typing "sh <file", e.g..  If this archive is complete, you
# will see the following message at the end:
#  "End of shell archive."
# Contents:  getpath.doc getpath.c getpath.h sample.c
# Wrapped by dtinker@utgpu.UUCP on Thu May 19 17:06:15 1988
PATH=/bin:/usr/bin:/usr/ucb ; export PATH
if test -f getpath.doc -a "${1}" != "-c" ; then 
  echo shar: Will not over-write existing file \"getpath.doc\"
else
echo shar: Extracting \"getpath.doc\" \(479 characters\)
sed "s/^X//" >getpath.doc <<'END_OF_getpath.doc'
X/*
X * getpath.doc
X */
X
X/*
X This little utility function parses a DOS file name and returns a structure
X containing the drive, path, name and extension of the file.  If the drive 
X and path are omitted in the input filename, getpath supplies the current
X working directory by default.  The function was written for the Aztec C (tm)
X compiler, but should be compatible with any standard C compiler.  A small
X program, sample.c is included to show the operation of the function.
X*/
END_OF_getpath.doc
if test 479 -ne `wc -c <getpath.doc`; then
    echo shar: \"getpath.doc\" unpacked with wrong size!
fi
# end of overwriting check
fi
if test -f getpath.c -a "${1}" != "-c" ; then 
  echo shar: Will not over-write existing file \"getpath.c\"
else
echo shar: Extracting \"getpath.c\" \(8092 characters\)
sed "s/^X//" >getpath.c <<'END_OF_getpath.c'
X
X/************************************************************************
X *                                                                      *
X *                getpath.c     -----    get DOS path                   *
X *                                                                      *
X *                 Version 1.00  ---   May 19, 1988                     *
X *                                                                      *
X *              Copyright (c) 1988 by David O. Tinker                   *
X *            All Rights Reserved except as noted below.                *
X *                                                                      *
X * This software may be used without charge for non-commercial purposes *
X * provided this Copyright notice is not removed. Permission is granted *
X * to make alterations to this software, provided credit is given to    *
X * the original author in any altered version.                          *
X *                                                                      *
X *                      David O. Tinker                                 *
X *                      Department of Biochemistry                      *
X *                      University of Toronto                           *
X *                      Toronto, Canada, M5S 1A8                        *
X *                      E-mail:                                         *
X *                      dtinker@utgpu.UUCP                              *
X *                      dtinker@utoronto.BITNET                         *
X *                                                                      *
X ***********************************************************************/
X
X
X#include "getpath.h"
X
X/************************************************
X *                                              *
X * path_t *getpath(filename)                    *
X * char *filename;                              *
X *                                              *
X * Instantiate structure to hold path data      *
X *                                              *
X * Input: DOS File Name string,                 *
X *        Format: [d:][\path]name[.ext]         *
X *                                              *
X * Returns: Structure of type path_t            *
X *          (see getpath.h                      *
X *                                              *
X * Calling:                                     *
X *      char *file_name;                        *
X *      path_t *path, *getpath();               *
X *      ...                                     *
X *      path = getpath(file_name);              *
X *                                              *
X * Library Functions Required:                  *
X *      *getcwd(2);                             *
X *      (get current working directory)         *
X *      *strcat(2);                             *
X *                                              *
X * Bugs:                                        *
X *      Does not check for legal filename       *
X *                                              *
X *                                              *
X * Author:      David O. Tinker                 *
X *                                              *
X ***********************************************/
X
Xpath_t *getpath(filename)
Xchar *filename;
X{
X        path_t *new;                    /* structure to hold data           */
X        char *file_disk,                /* disk location of active file     */
X             *path,                     /* path to active file              */
X             *file_name,                /* name of active file (no .ext)    */
X             *file_ext,                 /* extension of active file         */
X             *fil_loc,                  /* temporary cwd buffer             */
X             *curr_path;                /* current working directory        */
X        int i, j = 0, k, name_len, no_of_dirs = 0;
X        BOOLEAN path_p = FALSE;
X        char *directory[10],       /* 10 subdirectories ought be enough ! */
X             buff[13];             /* buffer to hold file & directory names */
X        void error();
X
X        name_len = strlen(filename);
X
X        /* get current working directory */
X
X        fil_loc= getcwd(NULL,64);
X        curr_path = CALLOC((strlen(fil_loc) + 2), char);
X        curr_path[0] = '\134';
X        curr_path = strcat(curr_path, fil_loc);
X        free((char *)fil_loc);
X
X        /* find disk and directory level of filename */
X
X        file_disk = CALLOC(3, char);
X        for (i = 0; i < name_len; i++) {
X                if((filename[i] == '\072') || (filename[i] == '\057')
X                   || (filename[i] == '\134'))
X                        path_p = TRUE;
X                if(filename[i] == '\072') { /* set file_disk name */
X                        j = i;
X                        file_disk[0] = filename[--j];
X                        file_disk[1] = filename[i];
X                        j += 2;
X                }
X                if((filename[i] == '\057') || (filename[i] == '\134'))
X                        ++no_of_dirs;
X        }
X
X        if(no_of_dirs) {
X                buff[0] = '\134';
X                buff[1] = '\0';
X                directory[0] = CALLOC(2, char);
X                strcpy(directory[0], buff);
X        }
X
X        for(i = 1, k = 0; i < no_of_dirs; i++) {
X                if(i == 1)
X                        ++j;
X                buff[k] = filename[j];
X                ++j;
X                while((filename[j] != '\057') && (filename[j] != '\134')) {
X                        ++k;
X                        buff[k] = filename[j];
X                        ++j;
X                        if(filename[j] == '\0') break;
X                }
X                buff[++k] = '\0';
X                directory[i] = CALLOC((strlen(buff) + 1), char);
X                strcpy(directory[i], buff);
X                k = 0;
X        }
X        for(i = 0, k = 1; i < no_of_dirs; i++) {
X                k += strlen(directory[i]);
X        }
X        if(no_of_dirs) {
X                ++j;
X                path = CALLOC(k, char);
X                strcpy(path, directory[0]);
X                for(i = 1; i < no_of_dirs; i++)
X                        path = strcat(path, directory[i]);
X        }
X        else 
X                path = CALLOC(1, char);
X
X        /* parse remaining characters in filename for name and ext */
X        for(i = 0, k = 0; i < 8; i++) {
X                buff[k] = filename[j];
X                ++k;
X                ++j;
X                if((filename[j] == '.') || (filename[j] == '\0')) {
X                        if(filename[j] == '.') ++j;
X                        break;
X                }
X        }
X        buff[k] = '\0';
X        file_name = CALLOC((strlen(buff) + 1), char);
X        strcpy(file_name, buff);
X        if(filename[j] != '\0'){
X                for(i = 0, k = 0; i < 3; i++) {
X                        buff[k] = filename[j];
X                        ++k;
X                        ++j;
X                        if(filename[j] == '\0') break;
X                }
X                buff[k] = '\0';
X                file_ext = CALLOC((strlen(buff) + 1), char);
X                strcpy(file_ext, buff);
X        }
X        else
X                file_ext = CALLOC(1, char);
X                
X        if(!path_p) {
X                path = CALLOC((strlen(curr_path) + 1), char);
X                strcpy(path, curr_path);
X        }
X
X        free((char *)curr_path);
X        
X        if((new = MALLOC(path_t)) == NULL) {
X                error("Can't instantiate Path");
X                return(NULL);
X        }
X
X        else {
X                new->fil_disk = file_disk;
X                new->fil_path = path;
X                new->fil_name = file_name;
X                new->fil_ext = file_ext;
X                return(new);
X        }
X        /* all done */
X}
X
X
X/****************************************************************
X * General Error Handling Utility                               *
X ***************************************************************/
X
Xvoid error(string)
Xchar *string;
X{
X        printf("ERROR : %s\n", string);
X        exit(1);
X}
X
END_OF_getpath.c
if test 8092 -ne `wc -c <getpath.c`; then
    echo shar: \"getpath.c\" unpacked with wrong size!
fi
# end of overwriting check
fi
if test -f getpath.h -a "${1}" != "-c" ; then 
  echo shar: Will not over-write existing file \"getpath.h\"
else
echo shar: Extracting \"getpath.h\" \(2347 characters\)
sed "s/^X//" >getpath.h <<'END_OF_getpath.h'
X
X/************************************************************************
X *                                                                      *
X * getpath.h:  Header File for getpath.c                                *
X *                                                                      *
X *                 Version 1.00  ---   May 19, 1988                     *
X *                                                                      *
X *              Copyright (c) 1988 by David O. Tinker                   *
X *            All Rights Reserved except as noted below.                *
X *                                                                      *
X * This software may be used without charge for non-commercial purposes *
X * provided this Copyright notice is not removed. Permission is granted *
X * to make alterations to this software, provided credit is given to    *
X * the original author in any altered version.                          *
X *                                                                      *
X *                      David O. Tinker                                 *
X *                      Department of Biochemistry                      *
X *                      University of Toronto                           *
X *                      Toronto, Canada, M5S 1A8                        *
X *                      E-mail:                                         *
X *                      dtinker@utgpu.UUCP                              *
X *                      dtinker@utoronto.BITNET                         *
X *                                                                      *
X ***********************************************************************/
X
X#include <stdio.h>
X
X#define BOOLEAN int
X#define TRUE  1
X#define FALSE 0
X
Xchar *malloc(), *calloc();
X#define MALLOC(x)      ((x *) malloc(sizeof(x)))
X#define CALLOC(n, x)   ((x *) calloc(n, sizeof(x)))
X
Xchar *getcwd(), *strcat();
X
X
X/****************************************
X * File Path Descriptor                 *
X ***************************************/
X 
X /* structure to hold path data for current file */
X
Xstruct path_primitive
X        {
X                char *fil_disk;
X                char *fil_path;
X                char *fil_name;
X                char *fil_ext;
X        };
X
Xtypedef struct  path_primitive  path_t;
Xpath_t *getpath();
X
END_OF_getpath.h
if test 2347 -ne `wc -c <getpath.h`; then
    echo shar: \"getpath.h\" unpacked with wrong size!
fi
# end of overwriting check
fi
if test -f sample.c -a "${1}" != "-c" ; then 
  echo shar: Will not over-write existing file \"sample.c\"
else
echo shar: Extracting \"sample.c\" \(502 characters\)
sed "s/^X//" >sample.c <<'END_OF_sample.c'
X/* sample.c  -- example of use of getpath.c */
X
X#include "getpath.h"
X
Xmain()
X{
X     path_t *file_path;
X     char   file_name[81];
X
X     printf("\n Enter a DOS file name: [d:][\path]name[.ext]\n");
X     scanf("%s", file_name);
X     file_path = getpath(file_name);
X     printf("\n file disk = %s\n", file_path->fil_disk);
X     printf(" file path = %s\n", file_path->fil_path);
X     printf(" file name = %s\n", file_path->fil_name);
X     printf(" file ext  = %s\n", file_path->fil_ext);
X
X     exit(0);
X}
X
END_OF_sample.c
if test 502 -ne `wc -c <sample.c`; then
    echo shar: \"sample.c\" unpacked with wrong size!
fi
# end of overwriting check
fi
echo shar: End of shell archive.
exit 0

