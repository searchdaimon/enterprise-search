#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include "../boithoadClientLib/liboithoaut.h"

int main() {

        char username[64];
        char *password;
        int i = 0;

        printf("username: ");

        while((username[i] = getchar()) != '\n') {
                i++;
        }
        username[i] = '\0';

        password = getpass("Password: ");

        if (boitho_authenticat(username,password)) {
                printf("Main: ok\n");
        }
        else {
                printf("Main: sorry\n");
        }
}

