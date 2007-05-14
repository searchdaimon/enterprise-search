#include "../common/define.h"

#define BLOCK_A    1
#define BLOCK_B    2
#define BLOCK_BOTH 3


void run_merge(FILE * file_a, FILE * file_b, FILE * output);
int find_next_block(struct linkdb_block * block_a, struct linkdb_block * block_b);
int find_next_block(struct linkdb_block * block_a, struct linkdb_block * block_b);
void read_next_blocks(struct linkdb_block * block_a, struct linkdb_block * block_b, 
				FILE * file_a, FILE * file_b, int block_out);
int find_by_from(struct linkdb_block * block_a, struct linkdb_block * block_b);
void dump_remaining(FILE * file_a, FILE * file_b, FILE * out);
int int_compare(unsigned int a, unsigned int b);
void write_out(FILE * output, struct linkdb_block * db_block);
FILE * open_db_file(const char * filename);
