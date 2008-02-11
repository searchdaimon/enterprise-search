
#ifndef _UTF_8_STRINGS_H_
#define _UTF_8_STRINGS_H_

/**
 *	Bibliotek for å behandle strenger kodet med utf-8, med særskilt vekt på de europeiske tegnene.
 *
 *	(C) Copyright 2007, Boitho AS (Magnus Galåen)
 */

#include <stdlib.h>
#include <string.h>


// Konverterer en tekststreng til små bokstaver:
static inline void convert_to_lowercase_n( unsigned char *str, int n )
{
    if (str==NULL) return;

	//toDo: runarb: 31.10.07: tar ikke hensyn til n overalt der vi har ++i. Dette kan fære til at vi går over hvis vi ar en feil i utf-8 stringen

    int         i;

    for (i=0; i<n; i++)
        {
            if (str[i]>='A' && str[i]<='Z')
                str[i]+= 32;    // 'a' - 'A'
            else if (str[i]==0xc3 && str[i+1]>=0x80 && str[i+1]<=0x9e) //  && str[i+1]!='\0' (implicit)
                {
                    i++;
                    str[i]+= 32;
                }
        }
}

// Konverterer en tekststreng til små bokstaver:
static inline void convert_to_lowercase( unsigned char *str )
{
    if (str==NULL) return;

    int         i;

    for (i=0; str[i]!='\0'; i++)
        {
            if (str[i]>='A' && str[i]<='Z')
                str[i]+= 32;    // 'a' - 'A'
            else if (str[i]==0xc3 && str[i+1]>=0x80 && str[i+1]<=0x9e) //  && str[i+1]!='\0' (implicit)
                {
                    i++;
                    str[i]+= 32;
                }
        }
}


// Detekterer om en string består av kun lowercase-bokstaver (altså ingen store bokstaver):
static inline int detect_no_uppercase( unsigned char *str )
{
    if (str==NULL) return 1;

    int         i;

    for (i=0; str[i]!='\0'; i++)
        {
            if (str[i]>='A' && str[i]<='Z')
                return 0;
            else if (str[i]==0xc3 && str[i+1]>=0x80 && str[i+1]<=0x9e) //  && str[i+1]!='\0' (implicit)
		return 0;
        }

    return 1;
}

// Detekterer om fï¿½ste tegn i strengen er uppercase:
static inline int utf8_first_char_uppercase( unsigned char *str )
{
	if (str==NULL || str[0]=='\0') return 0;

	if (str[0]>='A' && str[0]<='Z') return 1;
	if (str[0]==0xc3 && str[1]>=0x80 && str[1]<=0x9e) return 1;

	return 0;
}


// Detekterer om første tegn i strengen er uppercase:
static inline int utf8_first_char_uppercase( unsigned char *str )
{
    if (str==NULL || str[0]=='\0') return 0;

    if (str[0]>='A' && str[0]<='Z') return 1;
    if (str[0]==0xc3 && str[1]>=0x80 && str[1]<=0x9e) return 1;

    return 0;
}


// Konverter latin-1 til utf8
static inline unsigned char* copy_latin1_to_utf8( unsigned char *str )
{
    int			str_len = strlen((const char*)str);
    unsigned char	*tempstring = malloc(str_len*4 +1);
    unsigned char	*returnstring;
    int			i, j;

    for (i=0, j=0; str[i]!='\0' && j<str_len*4; i++)
	{
	    if (str[i] >= 0xc0 && (str[i+1] < 0x80 || str[i+1] > 0xbf))
		{
		    tempstring[j++] = 0xc0 + (str[i]>>6);
		    tempstring[j++] = 0x80 + (str[i] & 0x3f);
		}
	    else
		{
		    tempstring[j++] = str[i];
		}
	}

    tempstring[j++] = '\0';

    returnstring = malloc(j);
    memcpy(returnstring, tempstring, j);

    free(tempstring);

    return returnstring;
}

#endif	// _UTF_8_STRINGS_H_
