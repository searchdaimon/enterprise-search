#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "../common/exeoc.h"

#define DO_SUID 0
#define UID_USER 0
#define YUM_PATH "/usr/bin/yum"
#define YUM_FLAGS "-y" //asume yes

#define RPM_PATH "/tmp/rpm"

int pkg_exists(char * rpm_path);
void exec_and_exit(char *service, char *param);
int validate_pkg(char *pkg);
int is_valid_action(char *action);
int str_in_list(const char *list[], char *str);
int str_equals(const char *param, char *string);
void show_usage();

// header end

const char *valid_actions[] = {"check-update", "update", "clean", "localinstall", '\0'};

int main(int argc, char **argv) {
#if DO_SUID
	if (setuid(UID_USER) != 0) {
	    printf("Unable to setuid(%d)\n", UID_USER);
	    exit(EXIT_FAILURE);
	}
#endif
	
	if (argc >= 2) {
		char *yum_arg;
		char *action = argv[1];
		//char *param   = argv[2];

		if (is_valid_action(action)) {
		    yum_arg = (argc == 3) ? argv[2] : NULL;

		    if (str_equals(action, "localinstall")) {
			validate_pkg(yum_arg);
			
			char rpm_path[512];
			snprintf(rpm_path, sizeof(rpm_path), "%s/%s", RPM_PATH, yum_arg);
			
			if (!pkg_exists(rpm_path)) {
			    printf("Package %s does not exist.\n", rpm_path);
			    exit(EXIT_FAILURE);
			}

		    }

		    else if (str_equals(action, "clean")) {
			yum_arg = "all";
		    }
		    
		    exec_and_exit(action, yum_arg);
		}
		else {
		    printf("Action %s is not valid.\n", action);
		}
	}
		    
	show_usage();
	
	return 0;
}

int pkg_exists(char * rpm_path) {
    struct stat sbuf;
    return (stat(rpm_path, &sbuf) == -1) ? 0 : 1;
}

/**
 * Execute service and exit.
 *
 */
void exec_and_exit(char *action, char *yum_arg) {
    char exeocbuf[1024];
    int exeocbuflen;
    void *args;

    if (yum_arg == NULL) {
	char *no_yum_arg[] = {YUM_PATH, YUM_FLAGS, action, '\0'};
	args = no_yum_arg;
    }
    else {
	char *args_pkg[] = {YUM_PATH, YUM_FLAGS, action, yum_arg, '\0'};
	args = args_pkg;
    }

    exeocbuflen = sizeof(exeocbuf);
    //printf("kjorer %s %s %s \n", shargs[0], shargs[1], shargs[2]);

    int exec_return;
    if (!exeoc(args, exeocbuf, &exeocbuflen, &exec_return)) {
	printf("Unable to execute yum\n");
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
    for ( ; i < sizeof(pkg); i++) {
	char in = pkg[i];
	if (!(isalnum(in) || in == '.' || in == '-')) { // example input: webadmin-0.1.rpm
	    printf("Package name (%s) contains invalid character %c\n", pkg, in);
	    exit(EXIT_FAILURE);
	}
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
    printf("Usage: ./yumwrapper check-update|update|clean|localinstall [package]\n");
    exit(1);
}

