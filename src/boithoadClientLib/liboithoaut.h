int boitho_authenticat(const char username_in[],char password_in[]);
int boithoad_listUsers(char **respons_list[],int *nrofresponses);
int boithoad_groupsForUser(const char username_in[],char **respons_list[],int *nrofresponses);
void boithoad_respons_list_free(char *respons_list[]);
int boithoad_listGroups(char **respons_list[],int *nrofresponses);
int boithoad_getPassword(const char username_in[], char password[]);
