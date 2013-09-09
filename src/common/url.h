#ifndef _URL__H_
#define _URL__H_

int find_domain_no_subname (const char url[],char domain[], int sizeofdomain);
int find_domain_no_www (const char url[],char domain[], int sizeofdomain);
int find_domain (const char url[],char domain[],const int domainsize);
int find_domains_subname(const char url[],char domain[], int sizeofdomain);
int find_domain_path_len (char url[]);
int find_TLD(char domain[], char TLD[],int TLDsize);

int url_havpri1(char word[]);
int url_havpri2(char word[]);
int url_normalization (char url[], int urlsize);
int url_depth(char url[]);
int url_nrOfSubDomains(char url[]);
int url_isttl(char word[],char ttl[]);

int isWikiUrl(char url[]);
int gyldig_url(char word[]);
int isOkTttl(char word[]);
int legalUrl(char word[]);

#endif
