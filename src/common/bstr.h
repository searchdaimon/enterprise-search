
#ifndef _bstr__H_
#define _bstr__H_

#include <stddef.h>

void strsandr (char text[], char find[], char replace[]);
void strcasesandr (char textbuff[], int texbufftsize,char find[], char replace[]);
void strscpy(char *dest, const char *src, size_t n);
void chomp(char string[]);
int split(const char *Input, char *Delim, char ***List);
int btolower(int c);
void saafree(char **List);
void ntobr (char textbuff[], int texbufftsize);

#endif
