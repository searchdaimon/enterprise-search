
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <asm/div64.h>

#include "../common/sha1.h"
#include "search_index.h"



typedef struct
{
    unsigned char	sha1[20];
    unsigned int	DocID;
} record;


static inline unsigned int turn( unsigned int a )
{
    unsigned char b[4];
    unsigned char *p = (unsigned char*)(&a);

    b[3] = *p++;
    b[2] = *p++;
    b[1] = *p++;
    b[0] = *p;

    return *(unsigned int*)(&b[0]);
}


urldocid_data* urldocid_search_init(char *index_filename, char *db_filename)
{
    urldocid_data	*data = malloc(sizeof(urldocid_data));

    data->indexf = fopen(index_filename, "r");

    if (data->indexf==NULL)
	{
	    fprintf(stderr,"urldocid_search_init: Could not open index-file \"%s\".\n",index_filename);
	    perror(index_filename);

	    free(data);
	    return NULL;
	}

    data->dbf = fopen(db_filename, "r");

    if (data->dbf==NULL)
	{
	    fprintf(stderr,"urldocid_search_init: Could not open database \"%s\".",db_filename);
	    perror("");
	    fclose(data->indexf);
	    free(data);
	    return NULL;
	}

    struct stat		fileinfo;
    fstat(fileno(data->indexf), &fileinfo);

    data->index_size = fileinfo.st_size;
    data->one_unit = 0x80000000 / (data->index_size / 2);
//    printf("One unit = %.8x\n", data->one_unit);

    return data;
}

void urldocid_search_exit(urldocid_data *data)
{
    fclose(data->dbf);
    fclose(data->indexf);
    free(data);
}

unsigned int urldocid_search_index(urldocid_data *data, unsigned char *sha1)
{
    off_t	pos = turn(*(unsigned int*)&sha1[0]);
    off_t	adr[2];
    record	*block;
    unsigned long long int	bsize;
/*
    {
	int	x;
	for (x=0; x<20; x++)
	    printf("%.2X ", sha1[x]);
	printf("\n");
	printf("%.8x ", pos);
    }
*/
    pos/= data->one_unit;
    pos/= sizeof(off_t);
    pos*= sizeof(off_t);

//    printf("%.8x\n", pos);
//    printf("sizeof(off_t) = %i\n", sizeof(off_t));

	#ifdef DEBUG
		printf("urldocid_search_index: seek to %"PRId64"\n",data->indexf);
	#endif

    fseeko(data->indexf, pos, SEEK_SET);
    fread(adr, sizeof(off_t), 2, data->indexf);
/*
    {
	int	x;
	for (x=0; x<16; x++)
	    printf("%.2x ", ((unsigned char*)adr)[x]);
	printf("\n");

	for (x=0; x<8; x++)
	    printf("%.2x ", ((unsigned char*)&adr[0])[x]);
	printf("\n");
	for (x=0; x<8; x++)
	    printf("%.2x ", ((unsigned char*)&adr[1])[x]);
	printf("\n");
    }
*/
    bsize = adr[1] - adr[0];
//    printf("size: %i\n", bsize);
/*
    {
	printf("size: ");
	int	x;
	for (x=0; x<8; x++)
	    printf("%.2x ", ((unsigned char*)&bsize)[x]);
	printf("\n");
    }
*/
    block = malloc(bsize);
    //fungerer ikke på web1, se http://www.captain.at/howto-udivdi3-umoddi3.php
    //bsize/= sizeof(record);
    do_div(bsize,sizeof(record));

    fseeko(data->dbf, adr[0], SEEK_SET);
    fread(block, sizeof(record), bsize, data->dbf);

//    printf("ok\n");

    int		i;
    for (i=0; i<bsize; i++)
	{
	    int		j;
	    char	equal = 1;
	    for (j=0; j<20 && equal; j++)
		if (sha1[j] != block[i].sha1[j]) equal = 0;
/*
if (i<10) {
	int	x;
	for (x=0; x<20; x++)
	    printf("%.2X ", block[i].sha1[x]);
	printf("\n");
}
*/
	    if (equal)
		{
		    unsigned int 	DocID = block[i].DocID;

		    free(block);
		    return DocID;
		}
	}

//    printf("Not found.\n");
    free(block);
    return 0;
}


char getDocIDFromUrl(urldocid_data *data, char *url, unsigned int *DocID)
{
    SHA1Context		sha1;
    unsigned char	hash[20];

    if (SHA1Reset(&sha1))
	{
	    printf("SHA1Reset error.\n");
	    return 0;
	}

    if (SHA1Input(&sha1, (const unsigned char*)url, strlen(url)))
	{
	    printf("SHA1Input error.\n");
	    return 0;
	}

    if (SHA1Result(&sha1, hash))
	{
	    printf("SHA1Result error.\n");
	    return 0;
	}

//    printf("url( %s ) => sha1( ", argv[3]);
/*
    int		i;
    for (i=0; i<20; i++)
	{
	    printf("%.2X ", hash[i]);
	}
    printf(")\n");
*/
//    unsigned char	sha1[20] = {0x00, 0x00, 0x02, 0xA7, 0xF6, 0xE0, 0xF4, 0xC5, 0xA0, 0x85, 0x00, 0xDF, 0x6B, 0xDF, 0x40, 0xA5, 0x04, 0x09, 0xC0, 0x48};
//    unsigned char	sha1[20] = {0x00,0x0E,0x80,0x1A,0x78,0xFE,0xA4,0x44,0x84,0x3B,0xBF,0xF9,0xD6,0x4C,0x8C,0xE2,0x35,0x59,0x5D,0xA3};
//    unsigned char	sha1[20] = {0x00,0x0B,0xAD,0x52,0x96,0x27,0x5A,0xB9,0xA0,0xE1,0x30,0x8E,0x93,0xFC,0x19,0xAE,0x64,0x9B,0xCC,0xFB};
    // 01591abb
//015x27a9
    *DocID = urldocid_search_index(data, hash);

    if (*DocID==0) return 0;

    return 1;
}


