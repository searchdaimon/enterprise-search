#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "../../../../websearch/src/common/exeoc.h"

#define DO_SUID 1
#define UID_USER 507
#define INIT_DIR "/etc/init.d/"

void exec_and_exit(char *service, char *param);
int is_valid_service(char *service);
int is_valid_param(char *param);
int str_in_list(const char *list[], char *str);
int str_equals(const char *param, char *string);
void show_usage();

// header end

const char *valid_services[] = {"crawlManager", "boitho-bbdn", "searchdbb", '\0'};
const char *valid_params[] = {"start", "stop", "restart", "status", '\0'};

int main(int argc, char **argv) {
	if (DO_SUID) {
	    if (setuid(UID_USER) != 0) {
		printf("Unable to setuid(%d)\n", UID_USER);
		exit(2);
	    }
	}
	
	if (argc == 3) {
		char *service = argv[1];
		char *param   = argv[2];

		if (is_valid_service(service) && is_valid_param(param)) {
		    exec_and_exit(service, param);
		}
		else {
		    if (!is_valid_service(service)) 
			printf("Service %s is not valid.\n", service);
		    
		    else if (!is_valid_param(param)) 
			printf("Parameter %s is not valid.\n", param);
		    
		    show_usage();
		}
	}
	else {
	    show_usage();
	}
	
	fflush(stdout);
	return(0);
}

/**
 * Execute service and exit.
 *
 * Attributes:
 *	service - name of service
 *	param   - parameter to service
 */
void exec_and_exit(char *service, char *param) {
    char exeocbuf[1024];
    int exeocbuflen;


    char exec_string[512];
    snprintf(exec_string, sizeof(exec_string), 
		    "%s%s %s", INIT_DIR, service, param);
    
    char *shargs[] = {"/bin/sh", "-c", exec_string, '\0'};
    exeocbuflen = sizeof(exeocbuf);

    //printf("kjorer %s %s %s \n", shargs[0], shargs[1], shargs[2]);

    int exec_return;
    if (!exeoc(shargs, exeocbuf, &exeocbuflen, &exec_return)) {
	printf("Unable to execute service\n");
	exit(EXIT_FAILURE);
    }

    printf(exeocbuf);
    exit(exec_return);
    
}

int is_valid_service(char *service) {
    return str_in_list(valid_services, service);
}

int is_valid_param(char *param) {
    return str_in_list(valid_params, param);
}


/**
 * Return true/false if given string is in given list.
 *
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
    printf("Usage: ./initwrapper service start|stop|restart|status\n");
    exit(1);
}

