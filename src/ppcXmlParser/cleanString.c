#include <string.h>
#include <stdio.h>

//av og til begynner en xml feed med tabb eller spaces både foran og bak. Denne fjerner dette
void cleanString(char *key) {
int i;
        if (((int)key[0] == 10) || ((int)key[0] == 32)) { // tab og space
                printf("aa -%s-\n",key);
                i =0;
                while (((int)key[i] == 10) || ((int)key[i] == 32)) {
                        ++i;
                }
                strcpy(key,key +i);
                printf("ab -%s-\n",key);
                i = strlen(key);
                --i; //vi skal ikke test \0
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

