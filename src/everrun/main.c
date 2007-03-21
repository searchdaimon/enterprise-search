#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {

	int i;
	char command[128];

	if (argc < 2) {
                printf("Dette programet kjører en komando igjen og igjen\n\n\teverrun kamando\n\n");
                exit(0);
        }

	command[0] = '\0';
	for(i=1;i<argc;i++) {
		strcat(command,argv[i]);
		strcat(command," ");
		//printf("%s %s\n",argv[i],command);
	}
	printf("do: %s\n",command);

	while (1) {	
		system(command);
		//dont hamer
		sleep(5);
	}
}
