/*
    mp3curs.c - curses/ncurses interface for mp3info

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


#include "mp3info.h"


/*
 * Check if Break is given, and if so, clean term and exit, else pad the input.
 */

char *checkinput(int c, char *string, int length) {
        if (c == CTRL_C) { endwin(); exit(0); }
        return string;
}

/*
 * Get the cursor to the right place, and get input
 */

char *curs_addparam(WINDOW *win, int line, int length, char *buf) {
        int c;
        char string[TEXT_FIELD_LEN];

        wmove(win,line,16);
        c = wgetnstr(win,string,length);
        strcpy(buf,checkinput(c,string,length));
        return buf;
}

/*
 * Bring up the curses window, then get and parse input, and build the tag.
 */

void tagedit_curs(char *filename, int filenum, int fileoutof, id3tag *tag) {
WINDOW *mainwin;
   char line[50], track_text[4], *genre, genre_text[30];
   unsigned int track_num, genre_num;
   line[0]='\0';

   initscr(); cbreak(); noecho();
   nonl();
   mainwin = newwin(9,COLS,5,0);
   intrflush(mainwin, FALSE);
   keypad(mainwin, TRUE);
   CenterText(1,VERSION);
/*   CenterText(2,"by Ricardo Cerqueira (C)2000"); */
#ifdef HAVE_TOUCHWIN
   touchwin(mainwin);
#endif
   box(mainwin, 0, 0);
   wmove(mainwin,1,1); 
   wprintw(mainwin,"Song Title:    %s",tag->title);
   wmove(mainwin,2,1); 
   wprintw(mainwin,"Artist Name:   %s",tag->artist);
   wmove(mainwin,3,1); 
   wprintw(mainwin,"Album Name:    %s",tag->album);
   wmove(mainwin,4,1); 
   wprintw(mainwin,"Year:          %s",tag->year);
   wmove(mainwin,5,1); 
   wprintw(mainwin,"Comment:       %s",tag->comment);
   wmove(mainwin,6,1); 
   if(tag->track[0] != '\0') {
	sprintf(track_text,"%d",tag->track[0]);
   } else {
	track_text[0]='\0';
   }
   wprintw(mainwin,"Track:         %s",track_text);
   wmove(mainwin,7,1);
   if(tag->genre[0] < 127) {
	genre=typegenre[tag->genre[0]];
   } else {
	genre="";
   }
   strcpy(genre_text,genre);
   wprintw(mainwin,"Genre:         %s",genre);
   wmove(mainwin,8,4);
   wprintw(mainwin," (%d/%d) %s ",filenum,fileoutof,filename);
#ifdef __WIN32__
   wmove(mainwin,8,COLS-23);
   wprintw(mainwin," Press ^BREAK to quit ");
#else
   wmove(mainwin,8,COLS-22);
   wprintw(mainwin," Press ^C to quit ");
#endif
   refresh();
   echo();
   curs_addparam(mainwin,1,30,line);
   strncpy(tag->title,line,strlen(line));
   curs_addparam(mainwin,2,30,line);
   strncpy(tag->artist,line,strlen(line));
   curs_addparam(mainwin,3,30,line);
   strncpy(tag->album,line,strlen(line));
   curs_addparam(mainwin,4,4,line);
   strncpy(tag->year,line,strlen(line));
   curs_addparam(mainwin,5,30,line);
   strncpy(tag->comment,line,strlen(line));
   curs_addparam(mainwin,6,30,line);
   strncpy(track_text,line,strlen(line) + (strlen(line) < strlen (track_text) ? 0 : 1));
/*   strncpy(track_text,line,3); */
   curs_addparam(mainwin,7,30,line);
   strncpy(genre_text,line,strlen(line) + (strlen(line) < strlen (genre_text) ? 0 : 1));

   endwin();
   if((track_num=atoi(track_text)) < 256) {
	tag->track[0] = (unsigned char) track_num;
   } else {
	printf("ERROR - '%s' is not a valid track number.\n",track_text);
   }
   unpad(genre_text);
   if((genre_num=get_genre(genre_text)) < 256) {
	tag->genre[0] = (unsigned char) genre_num;
   } else {
	printf("ERROR - '%s' is not a valid genre name or number.\nUse '-G' for a list of valid genres.\n",genre_text);
   }

}

