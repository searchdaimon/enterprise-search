#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "debug.h"


int gyldig_url(char word[]) {
                static char lasturl[201] = {""};
                int lengt;
                char *cpnt;
		struct updateFormat updatePost;

		


                lengt = strlen(word);

                //finner siste bokstav
                if (word[lengt -1] == '/') {
                        cpnt = &word[lengt -1];
                }
                else {
                        if ((cpnt = strrchr(word,'.')) == NULL){
                                cpnt = NULL;
                        }
                }
                        //printf("cpnt %s\n",cpnt);


                //temp: for nå tar vi bare korte urler
                if (lengt > 150) {
			debug("To long. Was %i bytes",lengt);
			return 0;
                }
                else if (cpnt == NULL) {
                        debug("bad url. cpnt is NULL");
			return 0;
                }
                else if (
                        !(
                        (strcmp(cpnt,"/") == 0)
                        || (strstr(cpnt,"?") != NULL)
                        || (strcmp(cpnt,".cfm") == 0)
                        || (strcmp(cpnt,".htm") == 0)
                        || (strcmp(cpnt,".html") == 0)
                        || (strcmp(cpnt,".shtml") == 0)
                        || (strcmp(cpnt,".shtm") == 0)
                        || (strcmp(cpnt,".php") == 0)
                        || (strcmp(cpnt,".asp") == 0)
                        || (strcmp(cpnt,".stm") == 0)
                        || (strcmp(cpnt,".ece") == 0)
                        || (strcmp(cpnt,".aspx") == 0)
                        || (strcmp(cpnt,".do") == 0)
                        || (strcmp(cpnt,".jsp") == 0)
                        )
                ) {
                        //tar bar de filendelsene vi vil ha
                        debug("bad ending \"%s\"",cpnt);
			return 0;
                }
                else if (strchr(word,'#') != NULL) {
                        //printf("has #: %s\n",word);
			return 0;
                }
                else if (strchr(word,'%') != NULL) {
                        //printf("has %: %s\n",word);
			return 0;
                }
                else if (strcmp(word,lasturl) == 0) {
                        debug("last and ne the same %s == %s",word,lasturl);
			return 0;
                }
		else if (strstr(word,"..") != NULL) {
			debug("hav ..");
			return 0;
		}
		else if (strstr(word,"/./") != NULL) {
			debug("hav /./");
			return 0;
		}
		else if (strstr((word + 7),"http:/") != NULL) {
			//for å hindre http://go.kvasir.sol.no/fs/http:/www.nettfinn.no/
			debug("hav http:/");
			return 0;
		}
		else if (strstr((word + 7),"http://") != NULL) {
			debug("hav multiple http://");
			return 0;
		}
		else if (strstr((word + 7),"https://") != NULL) {
			debug("hav https://");
			return 0;
		}
		else if (strstr((word + 7),"ftp://") != NULL) {
			debug("hav ftp://");
			return 0;
		}
		else if (strstr((word + 7),"//") != NULL) {
			debug("hav //");
			return 0;
		}
		else if (strstr(word,"/default") != NULL) {
			debug("hav /default",word);
			return 0;
		}
		else if (strstr(word,"/index") != NULL) {
			debug("hav /index",word);
			return 0;
		}
		else if (strstr(word,"/www1") != NULL) {
			debug("hav /www1",word);
			return 0;
		}
		else if (strstr(word,"/www2") != NULL) {
			debug("hav /www2",word);
			return 0;
		}
		else if (strstr(word,"/www3") != NULL) {
			debug("hav /www3",word);
			return 0;
		}
		else if (strstr(word,"/www4") != NULL) {
			debug("hav /index",word);
			return 0;
		}
                else if (lengt > sizeof(updatePost.url)) {
                        debug("url to long at %i char",strlen(word));
			return 0;
                }
                else {
                        //printf("link: %s\n",word);

			return 1;
		}

}


int url_havpri1(char word[]) {

	if (
		(strstr((word + 7),".no/") != NULL)
		|| (strstr((word + 7),".gov/") != NULL)
		|| (strstr((word + 7),".mil/") != NULL)
		|| (strstr((word + 7),".edu/") != NULL)
		//|| (strstr((word + 7),".org/") != NULL)
		) {
                //printf("hav .pri/\n");
        	return 1;
        }
	else {
		return 0;
	}
}

int url_havpri2(char word[]) {

	if (
		(strstr((word + 7),".se/") != NULL)
		|| (strstr((word + 7),".dk/") != NULL)
		|| (strstr((word + 7),".uo.uk/") != NULL)
		|| (strstr((word + 7),".com/") != NULL)
		) {
                //printf("hav .pri/\n");
        	return 1;
        }
	else {
		return 0;
	}
}

