/*
    MP3Info - Displays and allows editing of MP3 ID3 tags and various
              technical aspects of MP3 files.

    mp3info.c - main part of console version of MP3Info
    
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


#define __MAIN
#include "mp3info.h"
#undef __MAIN
#include <sys/stat.h>

char FILENAME_FORMAT_STRING[]="File: %F\n";
char ID3_FORMAT_STRING[]="Title:   %-30t Track: %n\nArtist:  %a\nAlbum:   %-30l Year:  %y\nComment: %-30c Genre: %g [%G]\n";
char TECH_FORMAT_STRING[]="Media Type:  MPEG %2.1v Layer %L\nAudio:       %r KB/s, %qKHz (%o)\nEmphasis:    %e\nCRC:         %E\nCopyright:   %C\nOriginal:    %O\nPadding:     %p\nLength:      %m:%02s\n";

int main(int argc, char *argv[]) {
   FILE  *fp;
   int c, i, interactive = 0, view_only=1,delete_tag=0,file_open,retcode=0;
   int want_id3=1,scantype=SCAN_NONE,fullscan_vbr=0;
   int show_techinfo=0,force_mode=0,quickscan=1;
   int new_track=0,new_genre=0,firstfilearg;
   id3tag new_tag;
   char *print_format=NULL;
   char error_msg[256];
   unsigned int g,n;
   int vbr_report=VBR_VARIABLE;
   mp3info mp3;


   new_tag.title[0]=new_tag.artist[0]=new_tag.album[0]=new_tag.year[0]=
   new_tag.comment[0]=new_tag.track[0]=new_tag.genre[0]=1;
   /* use something REALLY unlikely... -- so we could clear the tag... */

   if (argc < 2 ) /* Only command is given. Short help */ {
	   printf("%s %s\n"\
		  "\n  MP3Info comes with ABSOLUTELY NO WARRANTY.  This is free software, and\n"\
		  "  you are welcome to redistribute it under certain conditions.\n"\
		  "  See the file 'LICENSE' for more information.\n"\
		  "\nUse 'mp3info -h' for a usage summary or see the mp3info man page for a\n"\
		  "complete description.\n",VERSION,COPYRIGHT);
	   return 0;
   }



   while ((c=getopt(argc,argv,"vhGidfxFt:a:l:y:c:n:g:p:r:"))!=-1) {

	switch(c) {
		case 'v': /* View mode is now automatic when no changes are
			     made to the ID3 tag. This switch is accepted 
			     only for backward compatibility */
			break;
		case 'h':
			display_help(); return 0;
			break;
		case 'G':
			display_genres(alphagenreindex,typegenre); return 0;
			break;
		case 'i':
			view_only=0;
			interactive = 1;
			break;
		case 'd':
			view_only=0;
			delete_tag=1;
			break;
		case 'p':
			print_format=optarg;
			translate_escapes(print_format);
			want_id3=0;
			break;
	        case 'f':
			force_mode=1;
			break;
		case 'x':
			show_techinfo=1;
			break;
		case 't':
			strncpy(new_tag.title,optarg,TEXT_FIELD_LEN);
			view_only=0;
			break;
		case 'a':
			strncpy(new_tag.artist,optarg,TEXT_FIELD_LEN);
			view_only=0;
			break;
		case 'l':
			strncpy(new_tag.album,optarg,TEXT_FIELD_LEN);
			view_only=0;
			break;
		case 'y':
			strncpy(new_tag.year,optarg,INT_FIELD_LEN);
			view_only=0;
			break;
		case 'c':
			strncpy(new_tag.comment,optarg,TEXT_FIELD_LEN);
			view_only=0;
			break;
		case 'n':
			n=atoi(optarg);
			if(n <= 255) {
				new_tag.track[0] = (unsigned char) n;
				new_track=1;
				view_only=0;
			} else {
				fprintf(stderr,"Error: '%s' is not a valid track number.\n",optarg);
				fprintf(stderr,"Valid track numbers are integers from 0 to 255.\n");
				fprintf(stderr,"Use a value of '0' to remove the track number field\n");
				retcode |= 6;
				return retcode;
			}
			break;
		case 'g':
			g=get_genre(optarg);
			if(g <= 255) {
				new_tag.genre[0] = (unsigned char) g;
				new_genre=1;
				view_only=0;
			} else {
				fprintf(stderr,"Error: '%s' is not a recognized genre name or number.\n",optarg);
				fprintf(stderr,"Use the '-G' option to see a list of valid genre names and numbers\n");
				retcode |= 6;
				return retcode;
			}
			sscanf(optarg,"%u",&g);
			break;
		case 'r':
			switch(optarg[0]) {
				case 'a': vbr_report=VBR_AVERAGE; break;
				case 'm': vbr_report=VBR_MEDIAN; break;
				case 'v': vbr_report=VBR_VARIABLE; break;
				default:
					fprintf(stderr,"Error: %s is not a valid option to the VBR reporting switch (-r)\n",optarg);
					fprintf(stderr,"Valid options are 'a', 'm' and 'v'.  Run '%s -h' for more info.\n",argv[0]);
					retcode |= 6;
					return retcode;
			}
			break;
		case 'F': quickscan=0; break;
	}
   }

   if(!view_only)
	scantype=SCAN_QUICK;

   if(print_format) {
   	determine_tasks(print_format,&want_id3,&scantype,&fullscan_vbr,vbr_report);
   } else if(view_only) {
   	determine_tasks(ID3_FORMAT_STRING,&want_id3,&scantype,&fullscan_vbr,vbr_report);
	if(show_techinfo)
		determine_tasks(TECH_FORMAT_STRING,&want_id3,&scantype,&fullscan_vbr,vbr_report);
   }

   
   if(!quickscan && (scantype == SCAN_QUICK))
   	scantype=SCAN_FULL;

   firstfilearg=optind;

   for(i=optind;i < argc; i++) { /* Iterate over all filenames */
      file_open=0;
      if (view_only == 1) { 
        if ( !( fp=fopen(argv[i],"rb") ) ) {
  	        sprintf(error_msg,"Error opening MP3: %s",argv[i]);
                perror(error_msg);
		retcode |= 1;
        } else {
		file_open=1;
	}
      } else {
        if ( !( fp=fopen(argv[i],"rb+") ) ) {
  	        sprintf(error_msg,"Error opening MP3: %s",argv[i]);
                perror(error_msg);
	        retcode |= 1;
        } else {
		file_open=1;
	}
      }

      if(file_open == 1) {
	memset(&mp3,0,sizeof(mp3info));
	mp3.filename=argv[i];
	mp3.file=fp;
	get_mp3_info(&mp3,scantype,fullscan_vbr);
	
	if((scantype != SCAN_NONE) && !mp3.header_isvalid && !force_mode) {
		fprintf(stderr,"%s is corrupt or is not a standard MP3 file.\n",mp3.filename);
		retcode |= 2;
	}

	if(view_only) {
	   if(want_id3 && !mp3.id3_isvalid)
		fprintf(stderr,"%s does not have an ID3 1.x tag.\n",mp3.filename);

	    if(print_format) {
		format_output(print_format,&mp3,vbr_report);
            } else {

		if(mp3.id3_isvalid || (show_techinfo && mp3.header_isvalid))
  	          	format_output(FILENAME_FORMAT_STRING,&mp3,vbr_report);
            	
       		if(mp3.id3_isvalid)
			format_output(ID3_FORMAT_STRING,&mp3,vbr_report);

            	if(show_techinfo && mp3.header_isvalid)
			format_output(TECH_FORMAT_STRING,&mp3,vbr_report);

       	    	printf("\n");

	    }
   
	} else if(mp3.header_isvalid || force_mode) {

	     if(new_tag.title[0]!=1) {
		  strncpy(mp3.id3.title,new_tag.title,TEXT_FIELD_LEN);
	     }

	     if(new_tag.artist[0]!=1) {
		  strncpy(mp3.id3.artist,new_tag.artist,TEXT_FIELD_LEN);
	     }

	     if(new_tag.album[0]!=1) {
		  strncpy(mp3.id3.album,new_tag.album,TEXT_FIELD_LEN);
	     }

	     if(new_tag.comment[0]!=1) {
			strncpy(mp3.id3.comment,new_tag.comment,TEXT_FIELD_LEN);
	     }

	     if(new_track) {
		  mp3.id3.track[0]=new_tag.track[0];
		  if(new_tag.track[0] == '\0') {
		     pad(mp3.id3.comment,TEXT_FIELD_LEN);
		  }
	     }

	     if(new_tag.year[0]!=1) {
		  strncpy(mp3.id3.year,new_tag.year,INT_FIELD_LEN);
	     }

	     if(new_genre) {
		  mp3.id3.genre[0]=new_tag.genre[0];
	     }
  		  
	     if( interactive ) {
	 	  tagedit_curs(mp3.filename,i-firstfilearg+1,argc-firstfilearg,&(mp3.id3));
	     }
                  

               
             /* Finally! Get it done! */
             if(!delete_tag) {
	          write_tag(&mp3);
             }

	 } else {
		fprintf(stderr,"Use the -f switch to add ID3 info to this file anyway.\n");
	 }

         fclose(mp3.file);

         if(delete_tag && mp3.id3_isvalid) {
		truncate(mp3.filename,mp3.datasize);
	 }


      }

   }

   if(optind == argc) {
	fprintf(stderr,"No MP3 files specified!\n");
	retcode |= 8;
   }


   return retcode;

}


