
#include <string.h>

#include "../common/sha1.h"
#include "search_index.h"


int main(int argc, char *argv[])
{
    if (argc!=4)
	{
	    printf("Usage: %s <db> <index> <url>\n\n");
	    return -1;
	}

    SHA1Context		sha1;
    unsigned char	hash[20];

    if (SHA1Reset(&sha1))
	{
	    printf("SHA1Reset error.\n");
	    return -1;
	}

    if (SHA1Input(&sha1, (const unsigned char*)argv[3], strlen(argv[3])))
	{
	    printf("SHA1Input error.\n");
	    return -1;
	}

    if (SHA1Result(&sha1, hash))
	{
	    printf("SHA1Result error.\n");
	    return -1;
	}

    printf("url( %s ) => sha1( ", argv[3]);

    int		i;
    for (i=0; i<20; i++)
	{
	    printf("%.2X ", hash[i]);
	}
    printf(")\n");
//    unsigned char	sha1[20] = {0x00, 0x00, 0x02, 0xA7, 0xF6, 0xE0, 0xF4, 0xC5, 0xA0, 0x85, 0x00, 0xDF, 0x6B, 0xDF, 0x40, 0xA5, 0x04, 0x09, 0xC0, 0x48};
//    unsigned char	sha1[20] = {0x00,0x0E,0x80,0x1A,0x78,0xFE,0xA4,0x44,0x84,0x3B,0xBF,0xF9,0xD6,0x4C,0x8C,0xE2,0x35,0x59,0x5D,0xA3};
//    unsigned char	sha1[20] = {0x00,0x0B,0xAD,0x52,0x96,0x27,0x5A,0xB9,0xA0,0xE1,0x30,0x8E,0x93,0xFC,0x19,0xAE,0x64,0x9B,0xCC,0xFB};
    // 01591abb
    urldocid_data	*data = urldocid_search_init(argv[2], argv[1]);
//015x27a9

//    unsigned int	DocID = urldocid_search_index(data, hash);
    unsigned int	DocID;

    if (!getDocIDFromUrl(data, argv[3], &DocID))
	printf("Not found\n");
    else
	printf("DocID: %i\n", DocID);

    urldocid_search_exit(data);
}
