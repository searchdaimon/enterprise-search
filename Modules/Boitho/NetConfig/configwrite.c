/*
 * Wrapper to replace network config file, without being root. Based on fdisk wrapper
 * written by Eirik A. Nyhaard 15.12.2006
 *
 * Written by: Dagur Valberg Johannsson
 * 		06.03.2007
 */

#include <sys/types.h>

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "../../../../websearch/src/common/exeoc.h"

// header
#define NET_IFCFG "netconf.test"
#define NETSCRIPT_DIR "/tmp"
#define MAX_INPUT_LENGHT 2047
#define SUID 0
#define INIT_NETWORK_PATH "/etc/rc.d/init.d/network restart"

char * read_config(void);
void validate_input(char *input);
void write_config(char *input);
void restart_network(void);

// code
int main(int argc, char **argv) {
	char *input;
	if (SUID) {
	    if (setuid(0) != 0) {
		printf("Unable to setuid(0)\n");
		exit(2);
	    }
	}
	
	if (argc == 2) {
		
		if (strcmp("restart", argv[1]) == 0) {
			printf("Restarting network\n");
			restart_network();
			exit(0);
		}
		else {
			printf(
				"Usage: \n\
				To write to config file: \n\
				\tstart configwrite with no parameters, write to stdin. \n\
				To restart network: \n\
				\t./configwrite restart\n");	
			exit(1);
		}
	}
	
	input = read_config();
	validate_input(input);
	write_config(input);
	free(input);
	fflush(stdout);
	return(0);
}


char * read_config(void) {
	char *input = malloc(sizeof(char[MAX_INPUT_LENGHT + 1]));
	char buffer;
	int i = 0;
	while ( (buffer = fgetc(stdin)) != EOF ) {
		if (i > (MAX_INPUT_LENGHT)) {
			fprintf(stdout, "Input to long, aborting.\n");
			exit(3);
		}
		input[i] = buffer;
		i++;
	}
	input[i] = '\0';
	return input;
}


/**
 * Die if input contains unrelated characters.
 *
 * Attributes:
 * 	char *input - String of input.
 */
void validate_input(char *input) {
	int i = 0;
	for ( ; i < MAX_INPUT_LENGHT; i++) {
		char in = input[i];
		if (in == '\0') break;
		if (!isalnum(in)   && !isspace(in)
		     && in != '='  && in != '.' 
		     && in != '\n' && in != '"') 
		{
		    fprintf(stdout, "Input contains invalid character '%c'\n", in);
		    exit(4);
		}
	}
}

/**
 * Write config file.
 *
 * Attributes:
 * 	content - Array with text to write to config.
*/

void write_config(char* input) {
	char path[512];
	FILE *config_file;
	int i;
	snprintf(path, sizeof(path), "%s/%s", NETSCRIPT_DIR, NET_IFCFG);
	
	config_file = fopen(path, "w");

	if (config_file == NULL) {
	    fprintf(stdout, "Unable to open config file for writing.");
	    exit(5);
	}
	
	for (i = 0; i < MAX_INPUT_LENGHT; i++) {
	    if (input[i] == '\0') break;
	    fprintf(config_file, "%c", input[i]);
	}
}


/**
 * Restart network config
 *
*/

void restart_network(void) {
	char exeocbuf[2048];
	int  exeocbuflen;

	char *netargs[] = {"/bin/sh", "-c", INIT_NETWORK_PATH, '\0'};

	exeocbuflen = sizeof(exeocbuf);
	if (!exeoc(netargs, exeocbuf, &exeocbuflen)) {
	    printf("Could not execute network restart procedure\n");
	    exit(10);
	}

	printf("%s\n", exeocbuf);
	
	//system(RK_RESTART_CMD); //TODO: Fix.
}

