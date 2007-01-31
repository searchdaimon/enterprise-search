struct AuthenticatedUserFormat {
        char username[64];
        char userid[128];
        char *memberOfgroups;
	char password[64];
};

