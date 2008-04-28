/*
<boitho>
base64 dekoder, som også kan dekode text med linjeskift
Skrevet av Runarb

Lånt fra perls MIME::Base64
</boitho>


Copyright 1997-2004 Gisle Aas

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.


The tables and some of the code that used to be here was borrowed from
metamail, which comes with this message:

  Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

  Permission to use, copy, modify, and distribute this material
  for any purpose and without fee is hereby granted, provided
  that the above copyright notice and this permission notice
  appear in all copies, and that the name of Bellcore not be
  used in advertising or publicity pertaining to this
  material without the specific, prior written permission
  of an authorized representative of Bellcore.	BELLCORE
  MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
  OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
  WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE  76 /* size of encoded lines */

static const char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define XX      255	/* illegal base64 char */
#define EQ      254	/* padding */
#define INVALID XX

#ifndef NATIVE_TO_ASCII
#   define NATIVE_TO_ASCII(ch) (ch)
#endif

static const unsigned char index_64[256] = {
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,62, XX,XX,XX,63,
    52,53,54,55, 56,57,58,59, 60,61,XX,XX, XX,EQ,XX,XX,
    XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,XX, XX,XX,XX,XX,
    XX,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,XX, XX,XX,XX,XX,

    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};

int base64_decode(char *out, const char *in, int maxlen) {

	/*
	SV* sv
	PROTOTYPE: $

	PREINIT:
	STRLEN len;
	register unsigned char *str = (unsigned char*)SvPVbyte(sv, len);
	unsigned char const* end = str + len;
	char *r;
	unsigned char c[4];

	CODE:
	{
	    //always enough, but might be too much
	    STRLEN rlen = len * 3 / 4;
	    RETVAL = newSV(rlen ? rlen : 1);
	}
        SvPOK_on(RETVAL);
        r = SvPVX(RETVAL);
	*/
	char *r = out;
	const char *str = in;
	int len = strlen(str);
	const char * end = str + len;
	unsigned char c[4];

	#ifdef DEBUG
	printf("len %i\n",len);
	printf("str (%u)< end (%u). end - str = %i\n",(unsigned int)str,(unsigned int)end,(end-str));
	#endif

	while (str < end) {
	    int i = 0;
            do {
		unsigned char uc = index_64[(int)NATIVE_TO_ASCII(*str++)];
		if (uc != INVALID)
		    c[i++] = uc;

		if (str == end) {
		    if (i < 4) {
			if (i)
			    fprintf(stderr,"Premature end of base64 data\n");
			if (i < 2) goto thats_it;
			if (i == 2) c[2] = EQ;
			c[3] = EQ;
		    }
		    break;
		}
            } while (i < 4);
	
	    if (c[0] == EQ || c[1] == EQ) {
		fprintf(stderr,"Premature padding of base64 data\n");
		break;
            }
	    /* printf("c0=%d,c1=%d,c2=%d,c3=%d\n", c[0],c[1],c[2],c[3]);*/

	    *r++ = (c[0] << 2) | ((c[1] & 0x30) >> 4);

	    if (c[2] == EQ)
		break;
	    *r++ = ((c[1] & 0x0F) << 4) | ((c[2] & 0x3C) >> 2);

	    if (c[3] == EQ)
		break;
	    *r++ = ((c[2] & 0x03) << 6) | c[3];
	}

	
      thats_it:
	//SvCUR_set(RETVAL, r - SvPVX(RETVAL));
	*r = '\0';
	//returnerer lengde
	return (r - out);
	/*
	OUTPUT:
	RETVAL
	*/

}
