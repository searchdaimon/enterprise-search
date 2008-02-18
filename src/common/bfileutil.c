#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

//#include "bfileutil.h"


/*
ripped from http://www.opensource.apple.com/darwinsource/tarballs/other/distcc-31.0.81.tar.gz
eks use: mkdir_p("/tmp/aaa/bb/",755) 
*/
/* -*- c-file-style: "java"; indent-tabs-mode: nil; fill-column: 78 -*-
 *
 * distcc -- A simple distributed compiler system
 *
 * Copyright (C) 2003, 2005 by Apple Computer, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/**
 * Create <code>path_to_dir</code>, including any intervening directories.
 * Behaves similarly to <code>mkdir -p</code>.
 * Permissions are set to <code>mode</code>.
 *
 * <code>path_to_dir</code> must not be <code>NULL</code>
 **/


int bmkdir_p(const char *path_to_dir,int mode)
{
    char *path = strdup(path_to_dir);
    char current_path[PATH_MAX];
    char *latest_path_element;

    current_path[0] = '\0';
    latest_path_element = strtok(path, "/");

    while ( strlen(current_path) < PATH_MAX - NAME_MAX - 1 ) {
        if ( latest_path_element == NULL ) {
            break;
        } else {
            strcat(current_path, "/");
            strcat(current_path, latest_path_element);

            // 504 == \770 == ug=rwx,o-rwx
            if ( mkdir(current_path, mode) == 0 ) {
		#ifdef DEBUG
                	fprintf(stderr,"info: Created directory %s\n", current_path);
		#endif
            } else {
                if ( errno == EEXIST ) {
			#ifdef DEBUG
                    	fprintf(stderr,"info: Directory exists: %s\n", current_path);
			#endif
                } else {
                    fprintf(stderr,"Unable to create directory %s: %s",
                                 current_path, strerror(errno));
                    return 0;
                }
            }

            latest_path_element = strtok(NULL, "/");
        }
    }

    free(path);

    if ( latest_path_element == NULL ) {
        return 1;
    } else {
        return 0;
    }
}

char *sfindductype(char filepath[]) {
        int len = strlen(filepath);
        int i;
        char *cp;
        char *type;

        for(i=len;i>0;i--) {
                //printf("s %i %c\n",i,filepath[i]);

                if (filepath[i] == '/') {
                        return NULL;
                }
                else if (filepath[i] == '.') {
                        //printf("found. %i\n",i);
                        cp = (char *)((int)filepath + i +1);
                        //printf("cp %u, filepath %u\n",(unsigned int)cp,(unsigned int)filepath);
                        //printf("cp \"%s\"\n",cp);
                        type = malloc(strlen(cp) +1);
                        strcpy(type,cp);
                        //printf("fant type %s, cp %s\n",type,cp);
                        return type;
                        //return cp;
                }
        }
        return NULL;
}


/*
	rekursiv sletting av en mappe. Skal bare selltte den mappen, og undermapper. Skal ikke
	kunne da en dagur og gi den "xx / yy", og den sletter /
*/
int rrmdir(char dir[]) {

        DIR *dirp;
        struct dirent *dp;
	char path[PATH_MAX];

	printf("rrmdir: opening dir \"%s\"\n",dir);

	if ((dirp = opendir( dir )) == NULL) {
		perror(dir);
		return 0;
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_name[0] == '.') {
                        continue;
                }

		snprintf(path,sizeof(path),"%s/%s",dir,dp->d_name);

		if (dp->d_type == DT_DIR) { 
			printf("rrmdir: dir: %s\n",path);
			rrmdir(path);
		}
		else {
			printf("rrmdir: file: %s\n",path);
		}

		
		//gjør selve slettingen
		if (remove(path) != 0) {
			perror("remove");
		}
		
	}	

	closedir(dirp);

	//sletter seg selv, frykter vi kan få problemer med . og .. her, må testes
	//remove( dir );
}


// Ax:
int readfile_into_buf( char *filename, char **buf )
{
    FILE	*file = fopen(filename, "r");
    int		i;

    if (!file)
	{
	    fprintf(stderr, "Could not open %s\n", filename);
	    return 0;
	}

    // Get filesize:
    struct stat	fileinfo;
    fstat( fileno( file ), &fileinfo );

    int		size = fileinfo.st_size;
    *buf = (char*)malloc(sizeof(char)*size);

    for (i=0; i<size;)
	{
	    i+= fread( (void*)&((*buf)[i]), sizeof(char), size-i, file );
	}

    fclose(file);

    return size;
}

