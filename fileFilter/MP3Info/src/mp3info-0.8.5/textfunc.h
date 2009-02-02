/*
    textfunc.h - headers for textfunc.h

    Copyright (C) 2000-2006 Cedric Tefft <cedric@phreaker.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  ***************************************************************************

  This program is based in part on MP3Info 0.5 by Ricardo Cerqueira <rmc@rccn.net>

*/

#define CTRL_C		'\003'
#define TEXT_FIELD_LEN	30
#define INT_FIELD_LEN	4

void CenterText(int line, char *text);
char *checkinput(int c, char *string, int length);
char *curs_addparam(WINDOW *win, int line, int length, char *buf);
void buildtag(char *buf, id3tag *tag);
void display_help();
void display_genres(int alphagenreindex[],char *typegenre[]);
unsigned int get_genre (char *genre);
void print_header(mp3header *header,off_t filesize);
void translate_escapes (char *string);
void determine_tasks (char *format_string,int *want_id3,int *scantype, int *fullscan_vbr,int vbr_report);
void format_output (char *format_string,mp3info *mp3, int vbr_report);




