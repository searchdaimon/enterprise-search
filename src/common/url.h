
int gyldig_url(char word[]);
int url_havpri1(char word[]);
int url_havpri2(char word[]);
int isOkTttl(char word[]);

int url_isttl(char word[],char ttl[]);

//int find_domain_no_subname (const char url[],char domain[], int sizeofdomain);
int find_domain_no_subname (const char url[],char domain[], int sizeofdomain);
int find_domain_no_www (char url[],char domain[], int sizeofdomain);
int find_TLD(char domain[], char TLD[],int TLDsize);
//int find_domain (char url[],char domain[]);
int find_domain (char url[],char domain[],int domainsize);
int url_normalization (char url[], int urlsize);
int isWikiUrl(char url[]);
int find_domain_path_len (char url[]);
int url_depth(char url[]);
int url_nrOfSubDomains(char url[]);
int find_domains_subname(const char url[],char domain[], int sizeofdomain);
