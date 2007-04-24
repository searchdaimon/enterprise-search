#include <sys/types.h>

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "../common/exeoc.h"

/* header */
// netscript
#define NET_IFCFG	   "ifcfg-eth1"
#define NETSCRIPT_DIR	   "/etc/sysconfig/network-scripts"

// resolv
#define RESOLV_PATH	   "/etc/resolv.conf"

// generic
#define INIT_NETWORK_PATH  "/etc/rc.d/init.d/network restart"
#define MAX_INPUT_LENGHT   2047
#define RUN_SUID	   1
#define SUID_USER	   0

#define RESOLVCONF_FILE    1
#define NETCONFIG_FILE     2

char *read_config(void);
void validate_input(char *input);
void write_config(char *input, int conf_file);
int  restart_network(void);
void show_usage(void);
/* header end */

int main(int argc, char **argv) {
	char *input;
#if RUN_SUID
	if (setuid(SUID_USER) != 0) {
	    fprintf(stderr, "Unable to setuid(%d)\n", SUID_USER);
	    exit(EXIT_FAILURE);
	}
#endif

	int conf_file;
	
	if (argc == 2) {
		
		if (strcmp("restart", argv[1]) == 0) {
		    printf("Restarting network\n");
		    int return_value = restart_network();
		    exit(return_value);
		}

		else if (strcmp("resolv", argv[1]) == 0) {
		    conf_file = RESOLVCONF_FILE;
		    

		}

		else if (strcmp("netconfig", argv[1]) == 0) {
		    conf_file = NETCONFIG_FILE;
		}

		else {
		    show_usage();
		}
	}
	else {
	    show_usage();
	}
	
	input = read_config();
	validate_input(input);
	write_config(input, conf_file);
	free(input);
	return 0;
}

/**
 * Show usage and exit.
 */
void show_usage(void) {
    fprintf(stderr, "Usage: configwrite restart|resolv|netconfig\n");
    fprintf(stderr, "Write to stdin if resolv or netconfig parameter is used.\n");
    exit(EXIT_FAILURE);
}


/**
 * Reads content from stdin.
 *
 * Returns:
 *	char* input - String with stdin content.
 */
char * read_config(void) {
	char *input = malloc(sizeof(char[MAX_INPUT_LENGHT + 1]));
	char buffer;
	int i = 0;
	while ( (buffer = fgetc(stdin)) != EOF ) {
		if (i > (MAX_INPUT_LENGHT)) {
			fprintf(stderr, "Input to long, aborting.\n");
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
		    fprintf(stderr, "Input contains invalid character '%c'\n", in);
		    exit(4);
		}
	}
}

/**
 * Write config file.
 *
 * Attributes:
 * 	content   - Array with text to write to config.
 * 	conf_file - What config file to write to. Values are defined in the header.
 *
 * Valid config files:
 *	NETCONFIG_FILE  - Config file for fedora 3 network config.
 *	RESOLVCONF_FILE - Resolv file.
*/

void write_config(char* input, int conf_file) {
	char path[512];
	FILE *fileh;
	int i;

	switch (conf_file) {

	case NETCONFIG_FILE:
	    snprintf(path, sizeof(path), "%s/%s", NETSCRIPT_DIR, NET_IFCFG);
	    break;


	case RESOLVCONF_FILE:
	    strncpy(path, RESOLV_PATH, sizeof(path));
	    break;

	default:
	    fprintf(stderr, "Unknown conf_file id %d\n", conf_file);
	    exit(EXIT_FAILURE);
	}
	
	fileh = fopen(path, "w");

	if (fileh == NULL) {
	    fprintf(stderr, "Unable to open config file %s for writing.\n", path);
	    exit(EXIT_FAILURE);
	}

	for (i = 0; i < MAX_INPUT_LENGHT; i++) {
	    if (input[i] == '\0') break;
	    fprintf(fileh, "%c", input[i]);
	}
}


/**
 * Restart network config
 *
*/

int restart_network(void) {
	char exeocbuf[2048];
	int  exeocbuflen;
	int  return_value;

	char *netargs[] = {"/bin/sh", "-c", INIT_NETWORK_PATH, '\0'};

	exeocbuflen = sizeof(exeocbuf);
	if (!exeoc(netargs, exeocbuf, &exeocbuflen, &return_value)) {
	    fprintf(stderr, "Could not execute network restart procedure\n");
	    exit(EXIT_FAILURE);
	}

	printf("%s\n", exeocbuf);
	return return_value;
}

