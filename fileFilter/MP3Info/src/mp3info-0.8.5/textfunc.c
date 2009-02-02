/*
    textfunc.c - Handles most console I/O and interface tasks

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

char *layer_text[] = {
	"I", "II", "III"
};

/*
 * Name of this one is quite obvious...
 */

void CenterText(int line, char *text) {
   mvprintw(line,(COLS/2)-((int)strlen(text)/2),text);
}

/* Convert hex digit to integer */

int xtoi(char *string) {
	char chr=toupper(string[0]);

	if(chr > '9')
		return (int) (chr - 'A' + 10);
	else 
		return (int) (chr - '0');
}



/*
 * Another one with an obvious name :-)
 */

void display_help() {

	 printf("%s %s\n\nUSAGE:\n\n",VERSION,COPYRIGHT);
	 printf("\tmp3info [ -h | -G ]\n"\
		"\n\tmp3info [-x] [-F] [-r a|m|v] [-p FORMAT_STRING] file(s)\n"\
		"\n\tmp3info [-d] file(s)\n"\
		"\n\tmp3info [-i]  [-t title] [-a artist] [-l album] [-y year]\n"\
		"\t\t[-c comment] [-n track] [-g genre] file(s)\n"\
		"\nOPTIONS\n"\
		"\t-a artist\tSpecify ID3 artist name\n"\
		"\t-c comment\tSpecify ID3 comment\n"\
		"\t-g genre\tSpecify ID3 genre (use -G for genre list)\n"\
		"\t-l album\tSpecify ID3 album name\n"\
		"\t-n track\tSpecify ID3 v1.1 track number\n"\
		"\t-t title\tSpecify ID3 track title\n"\
		"\t-y year\t\tSpecify ID3 copyright year\n\n"\
		"\t-G\t\tDisplay valid genres\n"\
		"\t-h\t\tDisplay this help page\n"\
		"\t-x\t\tDisplay technical attributes of the MP3 file\n"\
		"\t-r a|m|v\tReport bit rate of (VBR) files as:\n"\
		"\t\ta - Average bit rate (float)\n"\
		"\t\tm - Median bit rate (integer)\n"\
		"\t\tv - Simply  use  the  word 'Variable' (string) [default]\n\n"\
		"\t-i\t\tEdit ID3  tag  interactively\n"\
		"\t-d\t\tDelete ID3 tag (if one exists)\n"\
		"\t-f\t\tForce processing if file is not a standard MP3\n"\
		"\t-F\t\tFull Scan (see man page for details)\n"\
		"\n\n\t-p \"FORMAT_STRING\" Print FORMAT_STRING with substitutions\n"\
		"\n\t\tConversion Specifiers\n\n"\
		"\t\t%%f\tFilename without the path (string)\n"\
		"\t\t%%F\tFilename with the path (string)\n"\
		"\t\t%%k\tFile size in KB (integer)\n"\
		"\n\t\t%%a\tArtist (string)\n"\
		"\t\t%%c\tComment (string)\n"\
		"\t\t%%g\tMusical genre (string)\n"\
		"\t\t%%G\tMusical genre (integer)\n"\
		"\t\t%%l\tAlbum name (string)\n"\
		"\t\t%%n\tTrack (integer)\n"\
		"\t\t%%t\tTrack Title (string)\n"\
		"\t\t%%y\tYear (string)\n"\
		"\n\t\t%%C\tCopyright flag (string)\n"\
		"\t\t%%e\tEmphasis (string)\n"\
		"\t\t%%E\tCRC Error protection (string)\n"\
		"\t\t%%L\tMPEG Layer (string)\n"\
		"\t\t%%O\tOriginal material flag (string)\n"\
		"\t\t%%o\tStereo/mono mode (string)\n"\
		"\t\t%%p\tPadding (string)\n"\
		"\t\t%%v\tMPEG Version (float)\n"\
		"\n\t\t%%u\tNumber of good audio frames (integer)\n"\
		"\t\t%%b\tNumber of corrupt audio frames (integer)\n"\
		"\t\t%%Q\tSampling frequency in Hz (integer)\n"\
		"\t\t%%q\tSampling frequency in KHz (integer)\n"\
		"\t\t%%r\tBit  Rate  in  KB/s  (see also '-r')\n"\
		"\n\t\t%%m\tPlaying time: minutes only (integer)\n"\
		"\t\t%%s\tPlaying time: seconds only (integer)\n"\
		"\t\t%%S\tTotal playing time in seconds (integer)\n"\
		"\n\t\t%%%%\tA single percent sign\n"\
    		"\n\t\tEscape Sequences\n\n"\
		"\t\t\\n\tNewline\n"\
		"\t\t\\t\tHorizontal tab\n"\
		"\t\t\\v\tVertical tab\n"\
		"\t\t\\b\tBackspace\n"\
		"\t\t\\r\tCarriage Return\n"\
		"\t\t\\f\tForm Feed\n"\
		"\t\t\\a\tAudible Alert (terminal bell)\n"\
		"\t\t\\xhh\tAny  arbitrary character specified by the\n"\
		"\t\t\thexidecimal number hh\n"\
		"\t\t\\ooo\tAny arbitrary character specified by  the\n"\
		"\t\t\toctal number ooo\n"\
		"\t\t\\\\\tA single backslash character\n\n"\
		"This help screen is only a summary of command-line switches\n"\
		"See the man page for complete documentation.\n\n");

}

void display_genres(int alphagenreindex[],char *typegenre[]) {
	int i,j,index;
	printf("Extended MP3 ID3 Genres\n=======================\n");
	for(i=0;i<GENREROWS;i++) {
	   for(j=0;j<3;j++) {
	      index=(GENREROWS*j)+i;
	      if(index <= MAXGENRE) {
	         printf("%3d %-21s ",alphagenreindex[index],typegenre[alphagenreindex[index]]);
              }
           }
	   printf("\n");
	}
}

unsigned int get_genre (char *genre) {
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
		num_genre=256;
	}
	return num_genre;
}

void text_genre(unsigned char *genre,char *buffer) {
   int genre_num = (int) genre[0];

   if(genre_num <= MAXGENRE) {
	sprintf(buffer,"%s",typegenre[genre_num]);
   } else if(genre_num < 255) {
	sprintf(buffer,"(UNKNOWN) [%d]",genre_num);
   } else {
	buffer[0]='\0';
   }
}

void determine_tasks (char *format_string,int *want_id3,int *scantype, int *fullscan_vbr,int vbr_report) {

	char *format=format_string;
	char *percent;
	while((percent=strchr(format,'%'))) {
		percent++;
		while(*percent && (percent[0] != '%' && !isalpha(*percent))) percent++;
		switch (percent[0]) {
			case 't':
			case 'a':
			case 'l':
			case 'y':
			case 'c':
			case 'n':
			case 'g':
			case 'G': *want_id3=1; break;
			case 'm':
			case 's':
			case 'u':
			case 'S': *fullscan_vbr=1;
			case 'r': if(vbr_report != VBR_VARIABLE) *fullscan_vbr=1;
			case 'q':
			case 'Q':
			case 'e':
			case 'E':
			case 'C':
			case 'O':
			case 'v':
			case 'L':
			case 'p':
			case 'o': if(*scantype != SCAN_FULL)
					*scantype=SCAN_QUICK;
				  break;
			case 'b': *scantype=SCAN_FULL; break;
		}
		format=percent+1;
	}	
}


void format_output (char *format_string,mp3info *mp3, int vbr_report) {

	char genre[40]="";
	char mod[1000],*percent,*pos,*code;
	char *format=format_string;
	int modlen;

	while((percent=strchr(format,'%'))) {
		*percent=0;
		printf(format);
		*percent='%';
		code=percent+1;
		while(*code && (code[0] != '%' && !isalpha(*code))) code++;

		if(*code) {
			modlen=code-percent+1;
			if(modlen > 1000) {
				printf("Format modifier beginning at position %d too long!\n",(int)(percent-format));
				exit(5);
			}
			strncpy(mod,percent,modlen);
			mod[modlen]=0;
			mod[modlen-1]='s';
			switch (*code) {
				case 't': printf(mod,mp3->id3.title); break;
				case 'f': pos = (pos=strrchr(mp3->filename,'/')) ? 
						pos+1 : mp3->filename;
					  printf(mod,pos); break;
				case 'F': printf(mod,mp3->filename); break;
				case 'a': printf(mod,mp3->id3.artist); break;
				case 'l': printf(mod,mp3->id3.album); break;
				case 'k': mod[modlen-1] = 'd'; printf(mod,mp3->datasize / 1024); break;
				case 'y': printf(mod,mp3->id3.year); break;
				case 'c': printf(mod,mp3->id3.comment); break;
				case 'n': if(mp3->id3_isvalid && mp3->id3.track[0]) {
						mod[modlen-1]='d';
					  	printf(mod, (int) mp3->id3.track[0]);
					  }
					  break;
				case 'g': if(mp3->id3_isvalid) {
						text_genre(mp3->id3.genre,genre);
					  	printf(mod,genre);
					  }
					  break;
				case 'G': if(mp3->id3_isvalid) {
						mod[modlen-1]='d';
					  	printf(mod,(int) mp3->id3.genre[0]);
					  }
					  break;
				case 'r': if(mp3->header_isvalid) {
						if(mp3->vbr && (vbr_report == VBR_VARIABLE))
							printf(mod,"Variable");
						else if(vbr_report == VBR_AVERAGE) {
							mod[modlen-1]='f';
							printf(mod,mp3->vbr_average);
						} else {
							mod[modlen-1]='d';
							printf(mod,header_bitrate(&mp3->header));
						}
					  }
					  break;
				case 'q': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,header_frequency(&mp3->header)/1000);
					  }
					  break;
				case 'Q': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,header_frequency(&mp3->header));
					  }
					  break;
				case 'e': if(mp3->header_isvalid) {
						printf(mod,header_emphasis(&mp3->header));
					  }
					  break;
				case 'E': if(mp3->header_isvalid) {
						printf(mod,!mp3->header.crc ? "Yes" : "No");
					  }
					  break;
				case 'C': if(mp3->header_isvalid) {
						printf(mod,mp3->header.copyright ? "Yes" : "No");
					  }
					  break;
				case 'O': if(mp3->header_isvalid) {
						printf(mod,mp3->header.original ? "Yes" : "No");
					  }
					  break;
				case 'm': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,mp3->seconds / 60);
					  }
					  break;
				case 's': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,mp3->seconds % 60);
					  }
					  break;
				case 'S': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,mp3->seconds);
					  }
					  break;
				case 'v': if(mp3->header_isvalid) {
						mod[modlen-1]='f';
						printf(mod,mp3->header.version ? ((mp3->header.version==2) ? 2.5 : 1.0) : 2.0);
					  }
					  break;
				case 'L': if(mp3->header_isvalid) {
						printf(mod,layer_text[header_layer(&mp3->header)-1]);
					  }
					  break;
				case 'o': if(mp3->header_isvalid) {
						printf(mod,header_mode(&mp3->header));
					  }
					  break;
				case 'p': if(mp3->header_isvalid) {
						printf(mod,mp3->header.padding ? "Yes" : "No");
					  }
					  break;
				case 'u': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,mp3->frames);
					  }
					  break;
				case 'b': if(mp3->header_isvalid) {
						mod[modlen-1]='d';
						printf(mod,mp3->badframes);
					  }
					  break;
			        case '%': printf("%%"); break;
				default:  printf("%%%c",*(code=percent+1)); break;
			}
			format=code+1;
		}
		
	}
	printf(format);
}



void translate_escapes (char *string) {
	char *read=string;
	char *write=string;
	int val;

	while(*read) {
		if(*read == '\\') {
			read++;
			switch (*read) {
				case 'n': *(write++)='\n'; read++; break;
				case 't': *(write++)='\t'; read++; break;
				case 'v': *(write++)='\v'; read++; break;
				case 'b': *(write++)='\b'; read++; break;
				case 'r': *(write++)='\r'; read++; break;
				case 'f': *(write++)='\f'; read++; break;
				case 'a': *(write++)='\a'; read++; break;
				case 'X':
				case 'x': read++; /* HEX */
					  val=0;
					  if(isxdigit(*read)) val=xtoi(read++);
					  if(isxdigit(*read)) val=(val*16) + xtoi(read++);
					  *(write++)=(val % 256);
				default:  if(*read <= '7' && *read >= '0') { /* octal */
						val=xtoi(read++);
						if(*read <= '7' && *read >= '0') val=(val*8) + xtoi(read++);
						if(*read <= '7' && *read >= '0') val=(val*8) + xtoi(read++);
						*(write++)=(val % 256);
					  } else {
					  	*(write++)=*(read++); break;
					  }
			}
		} else {
			*write++=*read++;
		}
		
	}
	*write=0;
}

