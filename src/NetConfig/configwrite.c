#include <sys/types.h>

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "../common/exeoc.h"

/* header */
#define NET_IFCFG	   "ifcfg-eth1"
#define NETSCRIPT_DIR	   "/etc/sysconfig/network-scripts"

#define RESOLV_PATH	   "/etc/resolv.conf"
#define INIT_NETWORK_PATH  "/etc/init.d/network"
#define RUN_SUID	   1
#define SUID_USER	   0

#define MAX_LINE_LENGTH 500

#define CFG_RESOLV    1
#define CFG_NET     2

void stdin_to_cfgfile(int);
void write_line(FILE **, char *);
bool valid_key(const char * keys[], char *);
int sanity_check(const int, const char *);
bool ok_net_keyval(char *);
void show_usage(void);
int restart_network(void);
void open_cfgfile (FILE **, int);

const char * resolv_keys[] = {"nameserver", '\0'};
const char * net_keys[] = {"GATEWAY", "NAME", "BOOTPROTO",
    "DEVICE", "MTU", "NETMASK", "BROADCAST", "IPADDR",
    "NETWORK", "ONBOOT", '\0'};

/* header end */

int main(int argc, char **argv) {

#if RUN_SUID
    if (setuid(SUID_USER) != 0) {
        fprintf(stderr, "Unable to setuid(%d)\n", SUID_USER);
        exit(EXIT_FAILURE);
    }
#endif

    if (argc != 2)
        show_usage();

    if (strcmp("restart", argv[1]) == 0) {
        printf("Restarting network\n");
        exit(restart_network());
    }

    else if (strcmp("resolv", argv[1]) == 0)
        stdin_to_cfgfile(CFG_RESOLV);

    else if (strcmp("netconfig", argv[1]) == 0)
        stdin_to_cfgfile(CFG_NET);

    else
        show_usage();

    return 0;
}

void stdin_to_cfgfile(const int cfgfile) {

    // read config
    char line[MAX_LINE_LENGTH];
    int linepos = 0;
    line[0] = '\0';
    FILE * cfg_fh;
    open_cfgfile(&cfg_fh, cfgfile);

    while (true) {
        char buffer = fgetc(stdin);

        if (buffer == '\r') // ignore \r
            continue;

        if (buffer == '\n' || buffer == EOF) {
            line[linepos] = '\0';
            
            int checkret;
            if ((checkret = sanity_check(cfgfile, line)) >= 0) {
                write_line(&cfg_fh, line);
            }
            else {
                fprintf(stderr,
                        "Err %d, '%s' is not a valid config line. Ignoring.\n",
                        checkret, line);
            }
            line[0] = '\0';
            if (buffer == EOF)
                break; // stop reading
            
            linepos = 0;
        }
        else {
            line[linepos] = buffer;
            if (linepos >= MAX_LINE_LENGTH) {
                fprintf(stderr, "Line '%s'.. too long, giving up.\n", line);
                exit(EXIT_FAILURE);
            }

            linepos++;
        }
    }
    fclose(cfg_fh);
}

int sanity_check(const int cfgfile, const char * line) {
    if (cfgfile != CFG_RESOLV && cfgfile != CFG_NET) {
        fprintf(stderr, "Unknown cfgfile %d\n", cfgfile);
        exit(EXIT_FAILURE);
    }

    if (strlen(line) == 0) 
        return 1;

    
    char delimeter[2];
    if (cfgfile == CFG_RESOLV)
        delimeter[0] = ' ';
    else if (cfgfile == CFG_NET)
        delimeter[0] = '=';
    delimeter[1] = '\0';

    char *key, *value;
    char tokstr[strlen(line)+1];
    strncpy(tokstr, line, sizeof(tokstr));

    key   = strtok(tokstr, delimeter);
    value = strtok(tokstr, delimeter);

    if (key == NULL || value == NULL)
        return -2;

    switch (cfgfile) {
        case CFG_RESOLV:
            if (!valid_key(resolv_keys, key))
                return -3;
            break;

        case CFG_NET:
            if (!valid_key(net_keys, key))
                return -3;
            if (!ok_net_keyval(value))
                return -4;
            break;

        default:
            return -1;
    }
	
    return 1;
}

void write_line(FILE ** cfg_file, char * line) {
   fprintf(*cfg_file, "%s\n", line);
    
}

bool ok_net_keyval(char * value) {
    // we only want to check if
    // we have bad characters, so empty value is fine.
    if (!strlen(value)) 
        return true;

    int i;
    char c;
    while ((c = value[i]) != '\0') { 
        if (!isalnum(c) && c != '.' && c != ':')
            return false;
        i++;
    }
    return true;
}

bool valid_key(const char *keys[], char *key) {
    int i = 0;
    while (keys[i] != '\0') {
        if (strcmp(keys[i], key) == 0) 
         return true;
        i++;
    }
    return false;
}

/**
 * Restart network config
*/

int restart_network(void) {
    int exeocbuflen = 1024*1024*5;
    char * exeocbuf = malloc(exeocbuflen);
    int  return_value;

    char *netargs[] = {"/bin/sh", INIT_NETWORK_PATH, "restart", '\0'};
    

    if (!exeoc(netargs, exeocbuf, &exeocbuflen, &return_value)) {
        fprintf(stderr, "Could not execute network restart procedure\n");
        free(exeocbuf);
        exit(10);
    }

    printf(exeocbuf);
    free(exeocbuf);
    return return_value;
}



/**
 * Show usage and exit.
 */
void show_usage(void) {
    fprintf(stderr, "Usage: configwrite restart|resolv|netconfig\n");
    fprintf(stderr, "Write to stdin if resolv or netconfig parameter is used.\n");
    exit(EXIT_FAILURE);
}

void open_cfgfile (FILE ** fileh, const int cfgfile) {
    char path[512];

    switch (cfgfile) {

        case CFG_NET:
            snprintf(path, sizeof(path), "%s/%s", NETSCRIPT_DIR, NET_IFCFG);
            break;


        case CFG_RESOLV:
            strncpy(path, RESOLV_PATH, sizeof(path));
            break;

        default:
            fprintf(stderr, "Unknown conf_file id %d\n", cfgfile);
            exit(EXIT_FAILURE);
    }

    *fileh = fopen(path, "w");

    if (*fileh == NULL) {
        fprintf(stderr, "Unable to open config file %s for writing.\n", path);
        exit(EXIT_FAILURE);
    }
}


