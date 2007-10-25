
#ifndef _UTF_8_STRINGS_H_
#define _UTF_8_STRINGS_H_

/**
 *	Bibliotek for å behandle strenger kodet med utf-8, med særskilt vekt på de europeiske tegnene.
 *
 *	(C) Copyright 2007, Boitho AS (Magnus Galåen)
 */

#include <stdlib.h>


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


#endif	// _UTF_8_STRINGS_H_
