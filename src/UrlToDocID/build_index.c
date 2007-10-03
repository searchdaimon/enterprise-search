
#include <math.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct
{
    unsigned char	sha1[20];
    unsigned int	DocID;
} record;


static int	buffer_size = 131072;
record		*buffer;

static int	outbuf_size = 65536;
off_t		*outbuf;

/*
typedef struct
{
    off_t	ptr;
} index_record;
*/

union byte4
{
    unsigned int	i;
    unsigned char	byte[4];
};


static unsigned int turn( unsigned int a )
{
    unsigned char b[4];
    unsigned char *p = (unsigned char*)(&a);

    b[3] = *p++;
    b[2] = *p++;
    b[1] = *p++;
    b[0] = *p;

    return *(unsigned int*)(&b[0]);
}


int main(int argc, char *argv[])
{
    int		index_size = 1048576;
    int		argnr = 1;
    int		i;

    if (argc < 3)
	{
	    printf("Usage: %s [-i size] <in-file> <out-file>\n\n", argv[0]);
	    printf("  -i size\tIndex size, must be a multiple of 2 (default=1048576).\n\n");
	    return 0;
	}

    while (argv[argnr][0]=='-')
	{
	    if (!strcmp(argv[argnr], "-i"))
		{
		    argnr++;
		    index_size = atoi(argv[argnr]);
		}

	    argnr++;
	}

    union byte4		mask, increment;
    mask.i = turn(index_size -1);
    increment.i = 0x80000000 / (index_size/2);

    for (i=0; i<4; i++)
	{
	    switch (mask.byte[i])
		{
		    case 0x00: case 0xff: break;
		    case 0x01: mask.byte[i] = 0x80; break;
		    case 0x03: mask.byte[i] = 0xc0; break;
		    case 0x07: mask.byte[i] = 0xe0; break;
		    case 0x0f: mask.byte[i] = 0xf0; break;
		    case 0x1f: mask.byte[i] = 0xf8; break;
		    case 0x3f: mask.byte[i] = 0xfc; break;
		    case 0x7f: mask.byte[i] = 0xfe; break;
		    default: fprintf(stderr, "Error: index_size must be multiple of 2!\n"); return -1;
		}
	}

//    printf("mask = %.8x\n", mask.i);
//    printf("increment = %.8x\n", increment.i);

    FILE	*infile = fopen(argv[argnr], "r"), *outfile = fopen(argv[argnr+1], "w");

    if (infile == NULL)
	{
	    perror("Could not open file for reading.\n\n");
	    return -1;
	}

    if (outfile == NULL)
	{
	    perror("Could not open file for writing.\n\n");
	    return -1;
	}

    buffer = malloc(buffer_size * sizeof(record));
    outbuf = malloc(outbuf_size * sizeof(off_t));

    unsigned int	current = 0;
    off_t		address = 0;
    int			outpos = 0;

//    printf("------- %.8x --------------------------------------------------------------------------------\n", current);
    outbuf[outpos++] = address;

    while (!feof(infile))
	{
	    int		num_items = fread(buffer, sizeof(record), buffer_size, infile);

	    printf("."); fflush(stdout);

	    for (i=0; i<num_items; i++)
		{
		    int		top = turn(*(unsigned int*)&buffer[i].sha1[0]);

		    while ((top&mask.i) > current)
			{
			    current+= increment.i;
//			    printf("------- %.8x -------------------------------------------------------------------------------- %i\n", current, address);
			    outbuf[outpos++] = address;

			    if (outpos >= outbuf_size)
				{
				    // Flush output-buffer:
				    fwrite(outbuf, sizeof(off_t), outbuf_size, outfile);
				    outpos = 0;
				    printf("o"); fflush(stdout);
				}
			}
/*
		    int		j;
		    for (j=0; j<20; j++)
			printf("0x%.2X,", buffer[i].sha1[j]);
		    printf("\t%.8X", buffer[i].DocID);
//		    printf("\t%.8x %.8x", top, top & mask.i);
		    printf("\n");
*/
		    address+= sizeof(record);
		}
	}

    fwrite(outbuf, sizeof(off_t), outpos, outfile);

    fclose(infile);
    fclose(outfile);
}
