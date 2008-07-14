
#ifndef _UTF_8_STRINGS_H_
#define _UTF_8_STRINGS_H_

/**
 *	Bibliotek for å behandle strenger kodet med utf-8, med særskilt vekt på de europeiske tegnene.
 *
 *	(C) Copyright 2007-2008, SearchDaimon AS (Magnus Galåen)
 */

#include <stdlib.h>
#include <string.h>


typedef unsigned char	utf8_byte;


// Konverterer en tekststreng til små bokstaver:
static inline void convert_to_lowercase_n( utf8_byte *str, int n )
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

// Konverterer en tekststreng til små bokstaver: (*DEPRECATED* bruk utf8_strtolower istedet)
static inline void convert_to_lowercase( utf8_byte *str )
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

// Konverterer en tekststreng til små bokstaver:
static inline void utf8_strtolower( utf8_byte *str )
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
static inline int detect_no_uppercase( utf8_byte *str )
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

// Detekterer om første tegn i strengen er uppercase:
static inline int utf8_first_char_uppercase( utf8_byte *str )
{
    if (str==NULL || str[0]=='\0') return 0;

    if (str[0]>='A' && str[0]<='Z') return 1;
    if (str[0]==0xc3 && str[1]>=0x80 && str[1]<=0x9e) return 1;

    return 0;
}


// Konverter latin-1 til utf8
static inline unsigned char* copy_latin1_to_utf8( utf8_byte *str )
{
    int			str_len = strlen((const char*)str);
    unsigned char	*tempstring = (unsigned char*)malloc(str_len*4 +1);
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

    returnstring = (unsigned char*)malloc(j);
    memcpy(returnstring, tempstring, j);

    free(tempstring);

    return returnstring;
}


// Valid kun med 8859-1-kompatible tegn
static inline int utf8_strcasecmp( const utf8_byte *s1, const utf8_byte *s2 )
{
    int		i;

    for (i=0; s1[i]!='\0'; i++)
	{
	    unsigned char	a=s1[i], b=s2[i];

	    if (a==b) continue;

	    if (a>='A' && a<='Z') a+= 32;	// 'a' - 'A'
	    if (b>='A' && b<='Z') b+= 32;

	    if (i>0)
		{
		    if (s1[i-1]==0xc3 && a>=0x80 && a<=0x9e) a+= 32;
		    if (s1[i-1]==0xc3 && b>=0x80 && b<=0x9e) b+= 32;
		}

	    if (a<b) return -1;
	    if (a>b) return +1;
	}

    if (s2[i]=='\0') return 0;
    return -1;	// len(s1) < len(s2)
}


// Is 'f' defined as a letter in unicode?
inline char U_isletter( int f );


static inline int convert_U_utf8( utf8_byte *c, int val )
{
    int		pos = 0;

    if (val < 128)
	{
	    c[pos++] = (char)val;
	}
    else if (val < 2048)
	{
	    c[pos++] = (char)(192 + ((val>>6) & 0x1f));
	    c[pos++] = (char)(128 + (val & 0x3f));
	}
    else if (val < 65536)
	{
	    c[pos++] = (char)(224 + ((val>>12) & 0xf));
	    c[pos++] = (char)(128 + ((val>>6) & 0x3f));
	    c[pos++] = (char)(128 + (val & 0x3f));
	}
    else
	{
	    c[pos++] = (char)(240 + ((val>>18) & 0x7));
	    c[pos++] = (char)(128 + ((val>>12) & 0x3f));
	    c[pos++] = (char)(128 + ((val>>6) & 0x3f));
	    c[pos++] = (char)(128 + (val & 0x3f));
	}

    c[pos] = '\0';

    return pos;
}


static inline size_t utf8_strlen( const utf8_byte *s )
{
    int		i, sz;

    for (i=0,sz=0; s[i]!='\0'; sz++)
	{
	    if (s[i]<=0x7f) { i++; }
	    else if (s[i]>=0xc0 && s[i]<=0xdf && s[i+1]>=0x80 && s[i+1]<=0xbf) { i+= 2; }
	    else if (s[i]>=0xe0 && s[i]<=0xef && s[i+1]>=0x80 && s[i+1]<=0xbf
		&& s[i+2]>=0x80 && s[i+2]<=0xbf) { i+= 3; }
	    else if (s[i]>=0xf0 && s[i]<=0xf7 && s[i+1]>=0x80 && s[i+1]<=0xbf
		&& s[i+2]>=0x80 && s[i+2]<=0xbf && s[i+3]>=0x80 && s[i+3]<=0xbf) { i+= 4; }
	    else i++; /* warning: invalid byte sequence */
	}

    return sz;
}


#endif	// _UTF_8_STRINGS_H_




























