/***
 * This program takes two linkdb files and merges them into one. Made 05-15-2007.
 * Usage: ./mergeLinkDB linkdb1 linkdb2 output
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

int main (int argc, char *argv[]) {
	
	FILE * linkfile_a; 
	FILE * linkfile_b; 
        FILE *outfh;

        char *outfile = strdup("/tmp/sdweb-XXXXXX");
        int outfd;

	if (argc < 3) {
   		fprintf(stderr, "Programmet tar inn to linkdb filer og slaar de sammen");
		fprintf(stderr, "til ny (sortert) fil\n\nUsage: ./mergeLinkDB output linkdb\n");
		exit(EXIT_FAILURE);
    	}
	
	//skriver ut lit status slik at vi kan se progresjon hvis vi kjører dette fra et skript
	printf("merging %s < %s\n",argv[1],argv[2]);

	linkfile_a = open_db_file(argv[1]);
	linkfile_b = open_db_file(argv[2]);



	//lager en temperert navn
        if ((outfd = mkstemp(outfile)) == -1) {
                perror("mkstemp");
		exit(EXIT_FAILURE);
        }


	//åpner filen med det temerere navnet
        if ((outfh = fdopen(outfd,"w")) == NULL) {
                perror("mkstemp");
		exit(EXIT_FAILURE);
        }

	#ifdef debug
        printf("tmp outfile: \"%s\"\n",outfile);
	#endif

	/*
	char * outfile = argv[3];
	if ((output = fopen(outfile, "wb")) == NULL) {
		fprintf(stderr, "Unable to write to file %s", outfile);
		perror(outfile);
		exit(EXIT_FAILURE);
	}
	*/

	run_merge(linkfile_a, linkfile_b, outfh);

	//må lokke før vi kopierer. Hvis ikke kan det være data i bufferen som ikke blir med.
	fclose(outfh);
	close(outfd);

	//sletter den gamle filen
	if (unlink(argv[1]) != 0) {
		perror(argv[1]);
	}

	//flytter den nye filen slik at den blir output filen
	if (rename(outfile,argv[1]) == -1) {
		perror("rename");
		exit(EXIT_FAILURE);
	}

	free(outfile);

	return 1;
}

/**
 * Main merge loop.
 */
void run_merge(FILE * file_a, FILE * file_b, FILE * output) {
	int block_out;

	struct linkdb_block * block_a
		= malloc(sizeof(struct linkdb_block));
	struct linkdb_block * block_b
		= malloc(sizeof(struct linkdb_block));

	block_out = BLOCK_BOTH;

	while (1) {
		if (feof(file_a) || feof(file_b)) {
			dump_remaining(file_a, file_b, output);
			break;
		}

		read_next_blocks(block_a, block_b, 
			  file_a, file_b, block_out);
		
		block_out = find_next_block(block_a, block_b);
	
		// Write to output
		switch(block_out) {
			case BLOCK_A: 
				write_out(output, block_a); 
				break;

			case BLOCK_B: 
				write_out(output, block_b);
				break;

			case BLOCK_BOTH:
				write_out(output, block_a);
				write_out(output, block_b);
		}
	}

	free(block_a);
	free(block_b);

	fclose(file_a);
	fclose(file_b);
}

/**
 * Determines what block(s) should be written
 * to output.
 */
int find_next_block(struct linkdb_block * block_a, struct linkdb_block * block_b) {
	int block_out;
	int order = int_compare(block_a->DocID_to, block_b->DocID_to);
	switch (order) {
		case -1:
			block_out = BLOCK_A;
			break;
		case 0:
			block_out = find_by_from(block_a, block_b);
			break;
		case 1:
		block_out = BLOCK_B;
			break;
	}
	return block_out;
}

/**
 * Reads next block(s) from db file(s)
 */
void read_next_blocks(struct linkdb_block * block_a, struct linkdb_block * block_b, 
				FILE * file_a, FILE * file_b, int block_out) {		
	switch(block_out) {
		case BLOCK_A:
			fread(block_a, sizeof(*block_a), 1, file_a);
			break;
		case BLOCK_B:
			fread(block_b, sizeof(*block_b), 1, file_b);
			break;
		case BLOCK_BOTH:
			fread(block_a, sizeof(*block_a), 1, file_a);
			fread(block_b, sizeof(*block_b), 1, file_b);
	}
}

/**
 * Determines what block(s) should be written based on DocID_from.
 */
int find_by_from(struct linkdb_block * block_a, struct linkdb_block * block_b) {
	int order = int_compare(block_a->DocID_from, block_b->DocID_from);
	int block_out;
	switch(order) {
		case -1:
			block_out = BLOCK_A;
			break;
		case 0:
			block_out = BLOCK_BOTH;
			break;
		case 1:
			block_out = BLOCK_B;
			break;
	}

	return block_out;
}

/**
 * Dump all remaining content of the file that is not at eof.
 */
void dump_remaining(FILE * file_a, FILE * file_b, FILE * out) {
	FILE * in;
	if (feof(file_a) && feof(file_b))
		return;

	in = (feof(file_a)) ? file_b : file_a;

	struct linkdb_block block;
	while (!feof(in)) {
		fread(&block, sizeof(block), 1, in);
		write_out(out, &block);
	}
}

int int_compare(unsigned int a, unsigned int b) {
	if (a < b)
		return -1;
	if (a > b)
		return 1;

	return 0;
}

void write_out(FILE * output, struct linkdb_block * db_block) {
	fwrite(db_block, sizeof(struct linkdb_block), 1, output);
}

/**
 * Helper function to open a db file.
 */
FILE * open_db_file(const char * filename) {
    FILE * fh;
    if ((fh = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "Unable to open file. ");
		perror(filename);
		exit(EXIT_FAILURE);
    }

    return fh;
}


