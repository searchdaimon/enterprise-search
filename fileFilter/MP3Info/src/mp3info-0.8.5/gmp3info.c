/*
    GMP3Info - Displays and allows editing of MP3 ID3 tags and various
               technical aspects of MP3 files (GTK interface)

    gmp3info.c - main part of GTK version of MP3Info

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

  rmc, Feb/2001 - 	Added a file open dialog
			Added the Tech Info Window
			Added an about box
			GMP3Info can now be launched without an argument
*/

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#define __MAIN
#include "mp3info.h"
#undef __MAIN

gchar *gtext_genre(unsigned char genre);
unsigned char gget_genre (char *genre);
int kill_if_esc(GtkWidget *widget, GdkEventKey *event, gpointer obj);
void create_techinfo_win(void);
void open_mp3_file(void);
int load_mp3(char *filename);
void read_new_mp3(GtkWidget *button, GtkWidget *file);
void quick_popup(char *title, char *message);
void about_mp3info(void);

GtkWidget	*id3win,*id3win_frame_global;
GtkWidget	*id3win_frame_title,*id3win_frame_artist,*id3win_frame_album;
GtkWidget	*id3win_frame_year,*id3win_frame_comment, *id3win_frame_albyear;
GtkWidget	*id3win_frame_genre,*id3win_frame_track,*id3win_frame_comtrack;
GtkWidget	*id3win_text_title,*id3win_text_artist,*id3win_text_album;
GtkWidget	*id3win_text_year,*id3win_text_comment,*id3win_combo_genre;
GtkWidget	*id3win_text_track;
GtkWidget	*id3win_frame_buttons,*id3win_ok_button,*id3win_cancel_button;
GtkWidget       *id3win_menu_bar, *id3win_menu_file, *id3win_menu_fcont;
GtkWidget	*id3win_menu_info, *id3win_menu_open, *id3win_menu_help;
GtkWidget	*id3win_menu_hcont, *id3win_menu_about;
GList 		*genrelist=NULL;
gchar		track[4]="",fbuf[4];

unsigned char genre=255;
FILE  *fp;
unsigned char sig[2];
unsigned int track_num;
mp3info mp3;
int read_only=0;

void exit_save( GtkWidget *widget, GtkWidget *data) {
   	char tmp[31];

	if (mp3.file) {
	   if(!read_only) {
	   	strcpy(mp3.id3.title,gtk_entry_get_text(GTK_ENTRY(id3win_text_title)));
	   	strcpy(mp3.id3.artist,gtk_entry_get_text(GTK_ENTRY(id3win_text_artist)));
	   	strcpy(mp3.id3.album,gtk_entry_get_text(GTK_ENTRY(id3win_text_album)));
	   	strcpy(mp3.id3.year,gtk_entry_get_text(GTK_ENTRY(id3win_text_year)));
	   	strcpy(mp3.id3.comment,gtk_entry_get_text(GTK_ENTRY(id3win_text_comment)));
	        strcpy(tmp,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(id3win_combo_genre)->entry)));
	        mp3.id3.genre[0]=gget_genre(tmp);
	   	strcpy(tmp,gtk_entry_get_text(GTK_ENTRY(id3win_text_track)));
		mp3.id3.track[0]=atoi(tmp);
/*		if (mp3.id3.track[0] > 255)
			mp3.id3.track[0]=255;
*/
	        write_tag(&mp3);
	   }

   	fclose(mp3.file);
	}
   	gtk_main_quit();
}

int main(int argc, char *argv[] ) {
    
	int i;
	char track_text[4];


        gtk_init (&argc, &argv);


        
        id3win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(id3win),"MP3info - An ID3 tag editor");
        gtk_window_set_policy (GTK_WINDOW(id3win), FALSE,FALSE,FALSE);
        gtk_widget_set_usize(id3win,450,360);
        gtk_container_border_width(GTK_CONTAINER(id3win),5);
        gtk_signal_connect(GTK_OBJECT(id3win), "delete_event", (GtkSignalFunc) gtk_exit, NULL);
        id3win_frame_global=gtk_vbox_new(FALSE,5);

	/* rmcc was here */

	id3win_menu_bar = gtk_menu_bar_new();
	gtk_widget_show(id3win_menu_bar);
	gtk_box_pack_start(GTK_BOX(id3win_frame_global), id3win_menu_bar, FALSE, TRUE, 1);

	id3win_menu_file = gtk_menu_item_new_with_label("File");
	gtk_widget_show(id3win_menu_file);
	gtk_container_add(GTK_CONTAINER(id3win_menu_bar), id3win_menu_file);
	id3win_menu_fcont = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(id3win_menu_file), id3win_menu_fcont);

	id3win_menu_open = gtk_menu_item_new_with_label("Open New    ");
	gtk_widget_show(id3win_menu_open);
	gtk_container_add(GTK_CONTAINER(id3win_menu_fcont), id3win_menu_open);
	gtk_signal_connect(GTK_OBJECT(id3win_menu_open), "activate",
	                   GTK_SIGNAL_FUNC(open_mp3_file),
	                   NULL);

	id3win_menu_info = gtk_menu_item_new_with_label("Info        ");
	gtk_widget_show(id3win_menu_info);
	gtk_container_add(GTK_CONTAINER(id3win_menu_fcont), id3win_menu_info);
	gtk_signal_connect(GTK_OBJECT(id3win_menu_info), "activate",
	                   GTK_SIGNAL_FUNC(create_techinfo_win),
	                   NULL);

	id3win_menu_help = gtk_menu_item_new_with_label("      Help");
	gtk_widget_show(id3win_menu_help);
	gtk_container_add(GTK_CONTAINER(id3win_menu_bar), id3win_menu_help);
	gtk_menu_item_right_justify(GTK_MENU_ITEM(id3win_menu_help));
	id3win_menu_hcont = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(id3win_menu_help), id3win_menu_hcont);

	id3win_menu_about = gtk_menu_item_new_with_label("About    ");
	gtk_widget_show(id3win_menu_about);
	gtk_container_add(GTK_CONTAINER(id3win_menu_hcont), id3win_menu_about);
	gtk_signal_connect(GTK_OBJECT(id3win_menu_about), "activate",
	                   GTK_SIGNAL_FUNC(about_mp3info),
	                   NULL);


	/* rmcc has left the building */
        
        id3win_frame_title=gtk_frame_new("Title");
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_title),5);
        id3win_text_title=gtk_entry_new_with_max_length(30);
        gtk_container_add(GTK_CONTAINER(id3win_frame_title),id3win_text_title);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_title,TRUE,TRUE,0);
        
        id3win_frame_artist=gtk_frame_new("Artist");
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_artist),5);
        id3win_text_artist=gtk_entry_new_with_max_length(30);
        gtk_container_add(GTK_CONTAINER(id3win_frame_artist),id3win_text_artist);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_artist,TRUE,TRUE,0);
        
        id3win_frame_albyear=gtk_hbox_new(FALSE,30);
        id3win_frame_album=gtk_frame_new("Album");
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_album),5);
        id3win_text_album=gtk_entry_new_with_max_length(30);
        gtk_container_add(GTK_CONTAINER(id3win_frame_album),id3win_text_album);
        gtk_box_pack_start(GTK_BOX(id3win_frame_albyear),id3win_frame_album,TRUE,TRUE,0);
        
        id3win_frame_year=gtk_frame_new("Year");
        gtk_widget_set_usize(id3win_frame_year,2,0);
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_year),5);
        id3win_text_year=gtk_entry_new_with_max_length(4);
        gtk_container_add(GTK_CONTAINER(id3win_frame_year),id3win_text_year);
        gtk_box_pack_start(GTK_BOX(id3win_frame_albyear),id3win_frame_year,TRUE,TRUE,0);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_albyear,TRUE,TRUE,0);
        
        id3win_frame_comtrack=gtk_hbox_new(FALSE,30);
        id3win_frame_comment=gtk_frame_new("Comment");
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_comment),5);
        id3win_text_comment=gtk_entry_new_with_max_length(30);
        gtk_container_add(GTK_CONTAINER(id3win_frame_comment),id3win_text_comment);
        gtk_box_pack_start(GTK_BOX(id3win_frame_comtrack),id3win_frame_comment,TRUE,TRUE,0);
        
        id3win_frame_track=gtk_frame_new("Track");
        gtk_widget_set_usize(id3win_frame_track,2,0);
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_track),5);
        id3win_text_track=gtk_entry_new_with_max_length(3);
        gtk_container_add(GTK_CONTAINER(id3win_frame_track),id3win_text_track);
        gtk_box_pack_start(GTK_BOX(id3win_frame_comtrack),id3win_frame_track,TRUE,TRUE,0);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_comtrack,TRUE,TRUE,0);
        
        id3win_frame_genre=gtk_frame_new("Genre");
        gtk_container_border_width(GTK_CONTAINER(id3win_frame_genre),5);
        id3win_combo_genre=gtk_combo_new();
        for(i=0;i<MAXGENRE+2;i++) {
        	genrelist = g_list_append(genrelist, typegenre[galphagenreindex[i]]);
        }
        gtk_combo_set_popdown_strings(GTK_COMBO(id3win_combo_genre),genrelist);
        gtk_container_add(GTK_CONTAINER(id3win_frame_genre),id3win_combo_genre);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_genre,TRUE,TRUE,0);
        
        id3win_frame_buttons=gtk_hbox_new(TRUE,30);
        id3win_ok_button=gtk_button_new_with_label("OK");
        gtk_box_pack_start(GTK_BOX(id3win_frame_buttons),id3win_ok_button,TRUE,TRUE,0);
	if(read_only) {
	  gtk_widget_set_sensitive (id3win_text_title, FALSE);
	  gtk_widget_set_sensitive (id3win_text_artist, FALSE);
	  gtk_widget_set_sensitive (id3win_text_album, FALSE);
	  gtk_widget_set_sensitive (id3win_text_year, FALSE);
	  gtk_widget_set_sensitive (id3win_text_comment, FALSE);
	  gtk_widget_set_sensitive (id3win_text_track, FALSE);
	  gtk_widget_set_sensitive (id3win_combo_genre, FALSE);	
	} else {
	        id3win_cancel_button=gtk_button_new_with_label("Cancel");
        	gtk_box_pack_start(GTK_BOX(id3win_frame_buttons),id3win_cancel_button,TRUE,TRUE,0);
	        gtk_signal_connect (GTK_OBJECT (id3win_cancel_button), "clicked", GTK_SIGNAL_FUNC (gtk_exit), NULL);
        }

        gtk_widget_set_usize(id3win_frame_buttons,30,20);
        gtk_box_pack_start(GTK_BOX(id3win_frame_global),id3win_frame_buttons,TRUE,TRUE,0);
        
        gtk_container_add(GTK_CONTAINER(id3win),id3win_frame_global);
        
        gtk_signal_connect (GTK_OBJECT (id3win_ok_button), "clicked", GTK_SIGNAL_FUNC (exit_save), NULL);
        
        gtk_widget_show_all(id3win);
        
	memset(&mp3,0,sizeof(mp3info));

	if(argc==2) { 
	   if (! load_mp3(argv[1])) {
		exit(0);
	   }
           gtk_entry_set_text(GTK_ENTRY(id3win_text_title),mp3.id3.title);
           gtk_entry_set_text(GTK_ENTRY(id3win_text_artist),mp3.id3.artist);
           gtk_entry_set_text(GTK_ENTRY(id3win_text_album),mp3.id3.album);
           gtk_entry_set_text(GTK_ENTRY(id3win_text_year),mp3.id3.year);
           gtk_entry_set_text(GTK_ENTRY(id3win_text_comment),mp3.id3.comment);
	   if(mp3.id3.track[0] > 0)
		sprintf(track_text,"%d",(int)mp3.id3.track[0]);
	   else
		track_text[0]=0;
	   gtk_entry_set_text(GTK_ENTRY(id3win_text_track),track_text);
           gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(id3win_combo_genre)->entry), gtext_genre(mp3.id3.genre[0]));
    
	} else {
	   open_mp3_file();
	}

        gtk_main();
	return(0);
			
}

gchar *gtext_genre(unsigned char genre) {
   int genre_num = (int) genre;

   if(genre_num <= MAXGENRE) {
	return((gchar *)typegenre[genre_num]);
   } else {
	return((gchar *)"");
   }
}

unsigned char gget_genre (char *genre) {
	int num_genre=0;

        if(genre[0] == '\0') { return 255; }

	sscanf(genre,"%u",&num_genre);
	if(num_genre == 0) {
		if(genre[0] != '0') {
			while(num_genre++ <= MAXGENRE) {
				if(!strcasecmp(genre,typegenre[num_genre-1])) {
					return num_genre-1;
				}
			}
			num_genre=256;
		}		
	}

	if(num_genre < 0 || num_genre > MAXGENRE) {
		num_genre=255;
	}
	return (unsigned char) num_genre;
}

/* rmcc was here */

void create_techinfo_win(void) {
	GtkWidget	*infowin_main, *infowin_main_frame;
	GtkWidget	*infowin_exit_button, *infowin_button_frame;
	GtkWidget	*infowin_text_media, *infowin_text_bitrate;
	GtkWidget	*infowin_text_emphasis, *infowin_text_crc;
	GtkWidget	*infowin_text_copyright, *infowin_text_original;
	GtkWidget	*infowin_text_padding, *infowin_text_length;
	GtkWidget	*infowin_text_frequency;
	int		ibuf,ibuf2;
	char		buf[50];
	char 		*layer_text[] = { "I", "II", "III" };


	infowin_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_usize(infowin_main, 230, 300);
	gtk_container_border_width(GTK_CONTAINER(infowin_main), 15);
	gtk_window_set_title(GTK_WINDOW(infowin_main), "MP3 Technical Info");
	gtk_window_set_policy(GTK_WINDOW(infowin_main), FALSE, FALSE, FALSE);
	gtk_signal_connect_object(GTK_OBJECT(infowin_main), "key_press_event",
                               GTK_SIGNAL_FUNC(kill_if_esc),
                               (gpointer) infowin_main);
	infowin_main_frame=gtk_vbox_new(FALSE,5);

	if (mp3.header_isvalid) {
	
 		sprintf(buf,"Media Type: MPEG %s Layer %s",mp3.header.version ? ((mp3.header.version==2) ? "2.5" : "1.0") : "2.0", layer_text[header_layer(&mp3.header)-1]);
		infowin_text_media=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_media,TRUE,TRUE,0);
	
		sprintf(buf,"Bitrate: %i KB/s",header_bitrate(&mp3.header));
		infowin_text_bitrate=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_bitrate,TRUE,TRUE,0);
	
		sprintf(buf,"Frequency: %iKHz",header_frequency(&mp3.header)/1000);
		infowin_text_frequency=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_frequency,TRUE,TRUE,0);

		ibuf=mp3.seconds / 60;
		ibuf2=mp3.seconds % 60;
		sprintf(buf,"Length: %i:%02i",ibuf,ibuf2);
		infowin_text_length=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_length,TRUE,TRUE,0);
	
		sprintf(buf,"Emphasis: %s",header_emphasis(&mp3.header));
		infowin_text_emphasis=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_emphasis,TRUE,TRUE,0);
		
		sprintf(buf,"CRC: %s",!mp3.header.crc ? "Yes" : "No");
		infowin_text_crc=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_crc,TRUE,TRUE,0);
		
		sprintf(buf,"Copyright: %s",mp3.header.copyright ? "Yes" : "No");
		infowin_text_copyright=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_copyright,TRUE,TRUE,0);
	
		sprintf(buf,"Original: %s",mp3.header.original ? "Yes" : "No");
		infowin_text_original=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_original,TRUE,TRUE,0);
	
		sprintf(buf,"Padding: %s",mp3.header.padding ? "Yes" : "No");
		infowin_text_padding=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_padding,TRUE,TRUE,0);

	} else {
		
		sprintf(buf,"This file's header\nis invalid.\n\nNo information\navailable");
		infowin_text_media=gtk_label_new(buf);
		gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_text_media,TRUE,TRUE,0);

	}
	
        infowin_button_frame=gtk_hbox_new(TRUE,30);
	gtk_widget_set_usize(infowin_button_frame,30,20);
	gtk_box_pack_start(GTK_BOX(infowin_main_frame),infowin_button_frame,TRUE,TRUE,0);


	infowin_exit_button = gtk_button_new_with_label("Exit");
	gtk_widget_show(infowin_exit_button);
	gtk_box_pack_end(GTK_BOX(infowin_button_frame), infowin_exit_button, FALSE, TRUE, 0);
	gtk_signal_connect_object(GTK_OBJECT(infowin_exit_button), "pressed",
	                          GTK_SIGNAL_FUNC(gtk_widget_destroy),
	                          GTK_OBJECT(infowin_main));

	gtk_container_add(GTK_CONTAINER(infowin_main),infowin_main_frame);
	gtk_widget_show_all(infowin_main);
	

	
}

void open_mp3_file(void) {
	GtkWidget	*filewin_main;

	filewin_main=gtk_file_selection_new("Use another MP3 File");
	gtk_signal_connect_object(GTK_OBJECT(filewin_main), "key_press_event",
                (GtkSignalFunc) kill_if_esc, (gpointer) filewin_main);
	gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(filewin_main)->cancel_button), "pressed",
	                          GTK_SIGNAL_FUNC(gtk_widget_destroy),
	                          GTK_OBJECT(filewin_main));
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filewin_main)->ok_button), "clicked",
                        (GtkSignalFunc) read_new_mp3,
                        filewin_main);
	gtk_widget_show(filewin_main);



}

void read_new_mp3(GtkWidget *button, GtkWidget *file) {
	char *filename;

	filename = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(file));
	if (load_mp3(filename) == 1) {
		gtk_widget_destroy(file);
		gtk_entry_set_text(GTK_ENTRY(id3win_text_title),mp3.id3.title);
        	gtk_entry_set_text(GTK_ENTRY(id3win_text_artist),mp3.id3.artist);
        	gtk_entry_set_text(GTK_ENTRY(id3win_text_album),mp3.id3.album);
        	gtk_entry_set_text(GTK_ENTRY(id3win_text_year),mp3.id3.year);
        	gtk_entry_set_text(GTK_ENTRY(id3win_text_comment),mp3.id3.comment);
        	gtk_entry_set_text(GTK_ENTRY(id3win_text_track),(char *)mp3.id3.track);
        	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(id3win_combo_genre)->entry), gtext_genre(mp3.id3.genre[0]));
	}

}

int load_mp3(char *filename) {

	mp3info nmp3;

	memset(&nmp3,0,sizeof(mp3info));
	
	if ( !( nmp3.file=fopen(filename,"rb+") ) ) {
	    if((nmp3.file=fopen(filename,"rb")))
		read_only=1;
	    else {
		if (id3win) {
			quick_popup("Error in file", "Error opening MP3 file.");
		} else {
			g_print("Error opening MP3 file.");
		}
		return 0;
	    }
	}

	nmp3.filename=filename;
	get_mp3_info(&nmp3,SCAN_QUICK,0);
	if(!nmp3.header_isvalid && !nmp3.id3_isvalid) {
		if (id3win) {
			quick_popup("Error in file", "File is corrupt or is not a standard MP3 file!");
		} else {
			g_print("File is corrupt or is not a standard MP3 file!\n");
		}
		return 0;
	} else {
		mp3=nmp3;
		return 1;
	}
	return 0;

}

int kill_if_esc(GtkWidget *widget, GdkEventKey *event, gpointer obj) {

	if(event && event->type == GDK_KEY_PRESS && event->keyval == GDK_Escape) {
	     gtk_widget_destroy(GTK_WIDGET(obj));
	     gtk_signal_emit_stop_by_name(GTK_OBJECT(widget), "key_press_event");
	}

	return(TRUE);
}

void quick_popup(char *title, char *message) {
	GtkWidget *popupwin, *popupwin_main, *popupwin_msg, *popupwin_ok;
	char *markup;

	popupwin = gtk_dialog_new();
	gtk_widget_realize(popupwin);
	gtk_container_border_width(GTK_CONTAINER(popupwin), 12);
	gtk_window_set_title(GTK_WINDOW(popupwin), title);
	popupwin_main = GTK_DIALOG(popupwin)->vbox;
	popupwin_msg=gtk_label_new(NULL);
	markup = g_markup_printf_escaped("<span foreground=\"blue\">%s</span>",message);
	gtk_label_set_markup(GTK_LABEL(popupwin_msg),markup);
	g_free (markup);
	gtk_box_pack_start(GTK_BOX(popupwin_main),popupwin_msg,TRUE,TRUE,0);
	popupwin_ok = gtk_button_new_with_label(" OK ");
	gtk_signal_connect_object(GTK_OBJECT(popupwin_ok), "clicked",
                    GTK_SIGNAL_FUNC(gtk_widget_destroy),
                    GTK_OBJECT(popupwin));
	gtk_signal_connect_object(GTK_OBJECT(popupwin), "key_press_event",
                (GtkSignalFunc) kill_if_esc, (gpointer) popupwin);
	gtk_box_pack_end(GTK_BOX(popupwin_main), popupwin_ok, TRUE, FALSE, 0);
	gtk_widget_show_all(popupwin);

}

void about_mp3info(void) {
	quick_popup(VERSION,"MP3Info\n"
	"			An ID3 Tag Editor\n\n\n"
	"			Original author: Ricardo Cerqueira <rmc@plug.pt>\n"
	"			Current maintainer: Cedric Tefft <cedric@phreaker.net>\n"
	"			(C) 1999-2006 Ricardo Cerqueira, Cedric Tefft\n\n");
}

/* rmcc has left the building */
