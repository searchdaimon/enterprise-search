#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Hack for å løse utf-8-problemet ved Forskningsparken:
static inline void utf8_hack( char *str )
{
    char	*match;

    match = strcasestr( str, "%C3%" );

    while (match!=NULL)
	{
	    if (match[4]=='\0') return;
	    if (match[5]=='\0') return;

	    if ((match[4]=='8'||match[4]=='9'||match[4]=='A'||match[4]=='B'||match[4]=='a'||match[4]=='b')
		&& ((match[5]>='0' && match[5]<='9') || (match[5]>='a' && match[5]<='f') || (match[5]>='A' && match[5]<='F')))
		{
		    char		v[3];
		    long int		val;
		    char		hexval[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
		    int			i;

		    v[0] = match[4];
		    v[1] = match[5];
		    v[2] = '\0';
		    val = strtol(v, NULL, 16) + 0x40;

		    match[1] = hexval[(val >> 4) & 0xf];
		    match[2] = hexval[val & 0xf];

		    for (i=6; match[i]!='\0'; i++)
			match[i-3] = match[i];
		    match[i-3] = '\0';

		    crawlWarn("utf8_hack found invalid character.");
		}

	    match = strcasestr( match+1, "%C3%" );
	}
}


int cleanresourceWinToUnix(char resource[]) {

        char *cp;

        //gjør om på formatet slik at \\ blir //
        if (strchr(resource,'\\') != NULL) {
                crawlWarn("collection \"%s\" contains \"\\\" characters. Corect format \
                        is //host/shares not \\\\host\\shares. Will convert \"\\\" to \"/\"",resource);

                while((cp = strchr(resource,'\\')) != NULL) {
                        (*cp) = '/';
                }

        }

	//fp char bug fiks:
    	//utf8_hack(resource);
}

int cleanresourceUnixToWin(char resource[]) {

        char *cp;

        //gjør om på formatet slik at 
        if (strchr(resource,'/') != NULL) {

                while((cp = strchr(resource,'/')) != NULL) {
                        (*cp) = '\\';
                }

        }

	//fp char bug fiks:
    	//utf8_hack(resource);

	#ifdef DEBUG
	printf("new %s\n",resource);
	#endif
}

