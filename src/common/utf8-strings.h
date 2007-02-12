
#ifndef _UTF_8_STRINGS_H_
#define _UTF_8_STRINGS_H_

/**
 *	Bibliotek for å behandle strenger kodet med utf-8, med særskilt vekt på de europeiske tegnene.
 *
 *	(C) Copyright 2007, Boitho AS (Magnus Galåen)
 */


// Konverterer en tekststreng til små bokstaver:
static inline char* convert_to_lowercase( unsigned char *str )
{
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


#endif	// _UTF_8_STRINGS_H_
