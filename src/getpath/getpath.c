
/************************************************************************
 *                                                                      *
 *                getpath.c     -----    get DOS path                   *
 *                                                                      *
 *                 Version 1.00  ---   May 19, 1988                     *
 *                                                                      *
 *              Copyright (c) 1988 by David O. Tinker                   *
 *            All Rights Reserved except as noted below.                *
 *                                                                      *
 * This software may be used without charge for non-commercial purposes *
 * provided this Copyright notice is not removed. Permission is granted *
 * to make alterations to this software, provided credit is given to    *
 * the original author in any altered version.                          *
 *                                                                      *
 *                      David O. Tinker                                 *
 *                      Department of Biochemistry                      *
 *                      University of Toronto                           *
 *                      Toronto, Canada, M5S 1A8                        *
 *                      E-mail:                                         *
 *                      dtinker@utgpu.UUCP                              *
 *                      dtinker@utoronto.BITNET                         *
 *                                                                      *
 ***********************************************************************/


#include "getpath.h"

/************************************************
 *                                              *
 * path_t *getpath(filename)                    *
 * char *filename;                              *
 *                                              *
 * Instantiate structure to hold path data      *
 *                                              *
 * Input: DOS File Name string,                 *
 *        Format: [d:][\path]name[.ext]         *
 *                                              *
 * Returns: Structure of type path_t            *
 *          (see getpath.h                      *
 *                                              *
 * Calling:                                     *
 *      char *file_name;                        *
 *      path_t *path, *getpath();               *
 *      ...                                     *
 *      path = getpath(file_name);              *
 *                                              *
 * Library Functions Required:                  *
 *      *getcwd(2);                             *
 *      (get current working directory)         *
 *      *strcat(2);                             *
 *                                              *
 * Bugs:                                        *
 *      Does not check for legal filename       *
 *                                              *
 *                                              *
 * Author:      David O. Tinker                 *
 *                                              *
 ***********************************************/

path_t *getpath(filename)
char *filename;
{
        path_t *new;                    /* structure to hold data           */
        char *file_disk,                /* disk location of active file     */
             *path,                     /* path to active file              */
             *file_name,                /* name of active file (no .ext)    */
             *file_ext,                 /* extension of active file         */
             *fil_loc,                  /* temporary cwd buffer             */
             *curr_path;                /* current working directory        */
        int i, j = 0, k, name_len, no_of_dirs = 0;
        BOOLEAN path_p = FALSE;
        char *directory[10],       /* 10 subdirectories ought be enough ! */
             buff[13];             /* buffer to hold file & directory names */
        void error();

        name_len = strlen(filename);

        /* get current working directory */

        fil_loc= getcwd(NULL,64);
        curr_path = CALLOC((strlen(fil_loc) + 2), char);
        curr_path[0] = '\134';
        curr_path = strcat(curr_path, fil_loc);
        free((char *)fil_loc);

        /* find disk and directory level of filename */

        file_disk = CALLOC(3, char);
        for (i = 0; i < name_len; i++) {
                if((filename[i] == '\072') || (filename[i] == '\057')
                   || (filename[i] == '\134'))
                        path_p = TRUE;
                if(filename[i] == '\072') { /* set file_disk name */
                        j = i;
                        file_disk[0] = filename[--j];
                        file_disk[1] = filename[i];
                        j += 2;
                }
                if((filename[i] == '\057') || (filename[i] == '\134'))
                        ++no_of_dirs;
        }

        if(no_of_dirs) {
		//toDo: \134 for dos, \057 for unix. Bår kunne ha begge dele med et kompilarorflag
                //buff[0] = '\134';
                buff[0] = '\057';
		buff[1] = '\0';
                directory[0] = CALLOC(2, char);
                strcpy(directory[0], buff);
        }

        for(i = 1, k = 0; i < no_of_dirs; i++) {
                if(i == 1)
                        ++j;
                buff[k] = filename[j];
                ++j;
                while((filename[j] != '\057') && (filename[j] != '\134')) {
                        ++k;
                        buff[k] = filename[j];
                        ++j;
                        if(filename[j] == '\0') break;
                }
                buff[++k] = '\0';
                directory[i] = CALLOC((strlen(buff) + 1), char);
                strcpy(directory[i], buff);
                k = 0;
        }
        for(i = 0, k = 1; i < no_of_dirs; i++) {
                k += strlen(directory[i]);
        }
        if(no_of_dirs) {
                ++j;
                path = CALLOC(k, char);
                strcpy(path, directory[0]);
                for(i = 1; i < no_of_dirs; i++)
                        path = strcat(path, directory[i]);
        }
        else 
                path = CALLOC(1, char);

        /* parse remaining characters in filename for name and ext */
        for(i = 0, k = 0; i < 8; i++) {
                buff[k] = filename[j];
                ++k;
                ++j;
                if((filename[j] == '.') || (filename[j] == '\0')) {
                        if(filename[j] == '.') ++j;
                        break;
                }
        }
        buff[k] = '\0';
        file_name = CALLOC((strlen(buff) + 1), char);
        strcpy(file_name, buff);
        if(filename[j] != '\0'){
                for(i = 0, k = 0; i < 3; i++) {
                        buff[k] = filename[j];
                        ++k;
                        ++j;
                        if(filename[j] == '\0') break;
                }
                buff[k] = '\0';
                file_ext = CALLOC((strlen(buff) + 1), char);
                strcpy(file_ext, buff);
        }
        else
                file_ext = CALLOC(1, char);
                
        if(!path_p) {
                path = CALLOC((strlen(curr_path) + 1), char);
                strcpy(path, curr_path);
        }

        free((char *)curr_path);
        
        if((new = MALLOC(path_t)) == NULL) {
                error("Can't instantiate Path");
                return(NULL);
        }

        else {
                new->fil_disk = file_disk;
                new->fil_path = path;
                new->fil_name = file_name;
                new->fil_ext = file_ext;
                return(new);
        }
        /* all done */
}


/****************************************************************
 * General Error Handling Utility                               *
 ***************************************************************/

void error(string)
char *string;
{
        printf("ERROR : %s\n", string);
        exit(1);
}

