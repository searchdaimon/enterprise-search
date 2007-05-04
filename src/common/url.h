
int gyldig_url(char word[]);
int url_havpri1(char word[]);
int url_havpri2(char word[]);
int isOkTttl(char word[]);

int url_isttl(char word[],char ttl[]);

int find_domain_no_subname (char url[],char domain[], int sizeofdomain);
int find_domain_no_www (char url[],char domain[], int sizeofdomain);
int find_TLD(char domain[], char TLD[],int TLDsize);
int find_domain (char url[],char domain[]);
int url_normalization (char url[], int urlsize);
