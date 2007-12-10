/**
 * Package for installing packages with yum and rpm on fedora 4.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "../common/exeoc.h"

#define DO_SUID 1
#define SUID_USER 0

#define YUM_PATH    "/usr/bin/yum"
#define YUM_FLAGS   "-y" //asume yes
#define RPM_PATH    "/bin/rpm"
#define RPM_FLAGS   "-Uvh"

// dir where uploaded packages are
#define RPM_DIR_PATH "/tmp/rpm" 

#define PROGRAM_RPM 1
#define PROGRAM_YUM 2

int pkg_exists(char * rpm_path);
void exec_and_exit(int program, char *service, char *param);
int validate_pkg(char *pkg);
int is_valid_action(char *action);
int str_in_list(const char *list[], char *str);
int str_equals(const char *param, char *string);
void show_usage();

// header end

const char *valid_actions[] = {"check-update", "update", "clean", "install", "list", '\0'};

int main(int argc, char **argv) {
#if DO_SUID
	if (setuid(SUID_USER) != 0) {
	    printf("Unable to setuid(%d)\n", SUID_USER);
	    exit(EXIT_FAILURE);
	}
#endif

	if (argc < 2) {
	    printf("Too few arguments provided.\n");
	    show_usage();
	}

	
	char *second_arg;
	char *action = argv[1];
	int program_to_exec = PROGRAM_YUM;

	if (!is_valid_action(action)) {
	    printf("Action %s is not valid.\n", action);
	    show_usage();
	}
		
	second_arg = (argc == 3) ? argv[2] : NULL;
	if (str_equals(action, "install")) {
	    if (second_arg == NULL) {
		printf("You need to provide a package name.\n");
		show_usage();
	    }
	    validate_pkg(second_arg);
		
		
	    char rpm_path[512];
	    snprintf(rpm_path, sizeof(rpm_path), "%s/%s", RPM_DIR_PATH, second_arg);
		
	    if (!pkg_exists(rpm_path)) {
		printf("Package %s does not exist.\n", rpm_path);
		exit(EXIT_FAILURE);
	    }

	    second_arg = rpm_path;
	    program_to_exec = PROGRAM_RPM;

	}

	else if (str_equals(action, "clean")) {
	    second_arg = "all";
	}
	    
	exec_and_exit(program_to_exec, action, second_arg);
	return 0;
}

int pkg_exists(char * rpm_path) {
    struct stat sbuf;
    return (stat(rpm_path, &sbuf) == -1) ? 0 : 1;
}

/**
 * Execute program and exit.
 */
void exec_and_exit(int program, char *action, char *second_arg) {
    char exeocbuf[200000];
    int  exeocbuflen;

    void *args;

    switch (program) {
    case PROGRAM_RPM: 
	{
	char * my_args[] = {RPM_PATH, RPM_FLAGS, second_arg, '\0'};
	args = my_args;
	break;
	}

    case PROGRAM_YUM:
	if (second_arg == NULL) {
	    char * my_args[] = {YUM_PATH, YUM_FLAGS, action, '\0'};
	    args = my_args;
	}
	else {
	    char * my_args[] = {YUM_PATH, YUM_FLAGS, action, second_arg, '\0'};
	    args = my_args;
	}
	break;

    default:
	fprintf(stderr, "exec_and_exec: Unknown program %d\n", program);
	exit(EXIT_FAILURE);
    }

    exeocbuflen = sizeof(exeocbuf);
    //printf("kjorer %s %s %s \n", shargs[0], shargs[1], shargs[2]);

    int exec_return;
    if (!exeoc(args, exeocbuf, &exeocbuflen, &exec_return)) {
	printf("Unable to execute program\n");
	exit(EXIT_FAILURE);
    }

    printf(exeocbuf);
    exit(exec_return);
    
}

int is_valid_action(char *action) {
    return str_in_list(valid_actions, action);
}

int validate_pkg(char *pkg) {
    int i = 0;
    char in;
    while ((in = pkg[i]) != '\0') {
	if (!(isalnum(in) || in == '.' || in == '-')) { // example input: webadmin-0.1.rpm
	    printf("Package name (%s) contains invalid character %c\n", pkg, in);
	    exit(EXIT_FAILURE);
	}
	i++;
    }

    return 1;
}

/**
 * Return true/false if given string is in given list.
 */
int str_in_list(const char *list[], char *str) {
    int i = 0;
    while (list[i] != '\0') {

	if (str_equals(list[i], str)) {
	    return 1;
	}
	i++;
    }
    return 0;
}

int str_equals(const char *param, char *string) {
    return (strcmp(param, string) == 0);
}

/**
 * Show program usage and exit.
 */
void show_usage() {
    printf("Usage: ./yumwrapper check-update|update|clean|list|localinstall [package]\n");
    exit(1);
}

//rpm -Uvh /tmp/asdf.rpm
