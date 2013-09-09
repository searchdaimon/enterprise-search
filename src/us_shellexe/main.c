#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "../crawlManager2/usersystem.h"
#include "../3pLibs/keyValueHash/hashtable.h"
#include "../common/bstr.h"
#include "../common/exeoc.h"


char *run_command (const usersystem_data_t *data, const char *arg1, const char *arg2) {

 	char *command;
	pid_t ret;
	int exeocbuflen = (25 * 200) + 512; // XXX: Find a better way //(*documentfinishedbufsize);
	char *exeocbuf;
	char *execvarg[] = {NULL, NULL, NULL, NULL};


	if ((exeocbuf = malloc(exeocbuflen)) == NULL) {
		perror("Malloc exeocbuf");
		return NULL;
	}

	if (data->parameters == NULL) {
		fprintf(stderr,"No parameters given\n");
	}

        command = hashtable_search(data->parameters, "command");

        if (command == NULL) {
		fprintf(stderr,"Can't find parameter \"command\"\n");
		return NULL;
	}
	if (command[0] == '\0') {
		fprintf(stderr,"Parameter \"command\" was emty\n");
		return NULL;
	}


	execvarg[0] = command;
	execvarg[1] = arg1;
	execvarg[2] = arg2;

	printf("us_shellexe running: %s \"%s\" \"%s\"\n", execvarg[0], execvarg[1], execvarg[2]);

	if (exeoc(execvarg,exeocbuf,&exeocbuflen,&ret) == 0) {
		perror("exeoc");
		return NULL;
	}


	return exeocbuf;
}

int
shellexe_list_users(usersystem_data_t *data, char ***users, int *n_users)
{

        int Count, TokCount;
        char **Data;
	char *exeocbuf;

	if ((exeocbuf = run_command(data, "listusers", NULL)) == NULL) {
		return 0;
	}
	
	
	TokCount = split(exeocbuf, "\n", &Data);

	*users = malloc((TokCount+1) * sizeof(char *));
	if (*users == NULL) {
		perror("Malloc *users");
		return 0;
	}

	Count = 0;
	(*n_users) = 0;
	while( (Data[Count] != NULL) ) {

		
		printf("User: \"%s\"\n",Data[Count]);

		if (Data[Count][0] != '\0') {
			(*users)[(*n_users)++] = strdup(Data[Count]);
		}
		++Count;
	}
	(*users)[(*n_users) ] = NULL;

	free(exeocbuf);

	return 1;
}

int
shellexe_list_groupsforuser(usersystem_data_t *data, const char *user, char ***groups, int *n_groups)
{

	printf("shellexe_list_groupsforuser\n");

        int Count, TokCount;
        char **Data;
	char *exeocbuf;

	if ((exeocbuf = run_command(data, "groupsforuser", user)) == NULL) {
		return 0;
	}
	
	
	TokCount = split(exeocbuf, "\n", &Data);

	*groups = malloc((TokCount+1) * sizeof(char *));
	if (*groups == NULL) {
		perror("Malloc *groups");
		return 0;
	}

	Count = 0;
	(*n_groups) = 0;
	while( (Data[Count] != NULL) ) {

		
		printf("groups: \"%s\"\n",Data[Count]);

		if (Data[Count][0] != '\0') {
			(*groups)[(*n_groups)++] = strdup(Data[Count]);
		}
		++Count;
	}
	(*groups)[(*n_groups) ] = NULL;

	free(exeocbuf);

	return 1;

}

/* XXX */
int
shellexe_authenticate_user(usersystem_data_t *data, char *user, char *password)
{
	printf("shellexe_authenticate_user\n");
}

char *
shellexe_get_name(void)
{
	printf("shellexe_get_name\n");
        return strdup("Executes shell command");
}


usersystem_t usersystem_info = {
	US_TYPE_SHELL,
	shellexe_authenticate_user,
	shellexe_list_users,
	shellexe_list_groupsforuser,
	shellexe_get_name,
};
