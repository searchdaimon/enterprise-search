#include <stdio.h>
#include <string.h>
#include <time.h>
#include "strftime.h"

static const char *monthAbbr[]={
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
size_t strftime(char *s,size_t max, const char *format,
		const struct tm *tm) {
	size_t i=0,j=0;
	while(j<max && format[i]) {
		if (format[i]!='%') {
			s[j]=format[i];
			i++;j++;
		} else {
			i++;
			switch (format[i]) {
				case '%': s[j++]='%';break;
				case 'm': if ((j+2) >=max) return 0;
						  j+=sprintf(s+j,"%02d",tm->tm_mon+1);
						  break;	 	  
				case 'd': if ((j+2) >=max) return 0;
						  j+=sprintf(s+j,"%02d",tm->tm_mday);
						  break;
				case 'y': if ((j+2) >=max) return 0;
						  j+=sprintf(s+j,"%02d",tm->tm_year%100);
						  break;
				case 'Y': if ((j+4) >=max) return 0;
						  j+=sprintf(s+j,"%d",tm->tm_year+1900);	  
						  break;
				case 'b': if ((j+3)>=max) return 0;
							 strcpy(s+j,monthAbbr[tm->tm_mon]); 
						  	 j+=3;
						  break;
				case 'l': if ((j+2) >= max) return 0;
							  { int hour = tm->tm_hour;
								  if (hour>12) hour -=12;
								  if (hour == 0) hour = 12;
								  j+=sprintf(s+j,"%2d",hour);
								  break;
							  }
				case 'p':  if ((j+2) >= max) return 0;
						  if (tm->tm_hour >11) {
							  strcpy(s+j,"PM");
						  } else {
							  strcpy(s+j,"AM");
						  }
						   j+=2;
						  break;
				case 'H':  if ((j+2) >= max) return 0;
						   j+=sprintf(s+j,"%02d",tm->tm_hour);
							break;	   
				case 'M':if ((j+2) >= max) return 0;
						   j+=sprintf(s+j,"%02d",tm->tm_min);
							break;
				case 'S':if ((j+2) >= max) return 0;
						   j+=sprintf(s+j,"%02d",tm->tm_sec);
							break;
				default:			
							;
			}
			i++;
		}	
	}
	if (j>=max) return 0;
	else {
		s[j]=0;
		return j;
	}
}		
