
/************************************************************************
 *                                                                      *
 * getpath.h:  Header File for getpath.c                                *
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

#include <stdio.h>

#define BOOLEAN int
#define TRUE  1
#define FALSE 0

//char *malloc(), *calloc();
#define MALLOC(x)      ((x *) malloc(sizeof(x)))
#define CALLOC(n, x)   ((x *) calloc(n, sizeof(x)))

char *getcwd(), *strcat();


/****************************************
 * File Path Descriptor                 *
 ***************************************/
 
 /* structure to hold path data for current file */


char *getpath();

