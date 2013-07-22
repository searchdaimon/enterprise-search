#ifndef UNISTD_H
#define UNISTD_H
#include <dos.h>
#include <io.h>
extern int optind;
extern char *optarg;
extern int opterr;
int	getopt(int argc, char *argv[], char *optionS);
#endif
