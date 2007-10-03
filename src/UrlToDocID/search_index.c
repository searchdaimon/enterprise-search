
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
	    perror("Could not open index-file.");
	    free(data);
	    return NULL;
	}

    data->dbf = fopen(db_filename, "r");

    if (data->dbf==NULL)
	{
	    perror("Could not open database.");
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

    fseek(data->indexf, pos, SEEK_SET);
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
    bsize/= sizeof(record);

    fseek(data->dbf, adr[0], SEEK_SET);
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
