#include <string.h>
#include <stdio.h>

// sometimes an xml feed starts with a tab or spaces both before and after. This removes that
void cleanString(char *key) {
int i;
        if (((int)key[0] == 10) || ((int)key[0] == 32)) { // tab and space
                printf("aa -%s-\n",key);
                i =0;
                while (((int)key[i] == 10) || ((int)key[i] == 32)) {
                        ++i;
                }
                strcpy(key,key +i);
                printf("ab -%s-\n",key);
                i = strlen(key);
                --i; // we should not test \0
                printf("1 i %i\n",i);
                while (((int)key[i] == 10) || ((int)key[i] == 32)) {
                        --i;
                }
                ++i;
                printf("2 i %i\n",i);
                key[i] = '\0';
                printf("ac -%s-\n",key);
        }

}

