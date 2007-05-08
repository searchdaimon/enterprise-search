
#ifndef _SEARCH_AUTOMATON_H_
#define _SEARCH_AUTOMATON_H_

#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include "../common/utf8-strings.h"


struct automaton;

typedef struct automaton
{
    int				terminal;
    unsigned char		*str;
    struct automaton		*next[256];
} automaton;


/*
// Konverterer A-Z og utf-8 ÆØÅ (+de andre latin-1 -tegnene) til små bokstaver:
static inline char* convert_to_lowercase( unsigned char *str )
{
    int		i;

    for (i=0; str[i]!='\0'; i++)
	{
	    if (str[i]>='A' && str[i]<='Z')
		str[i]+= 32;	// 'a' - 'A'
	    else if (str[i]==0xc3 && str[i+1]>=0x80 && str[i+1]<=0x9e) //  && str[i+1]!='\0' (implicit)
		{
		    i++;
		    str[i]+= 32;
		}
	}
}
*/

	    /** BEGIN PSEUDOCODE: **/
/*
	    if A->str and words[i][p] don't share any characters (n=0):		A+B
	    or if A->str and words[i][p] share first n characters, where n<len(A->str) and n<len(words[i][p]): A+B

	    if first n characters of A->str and words[i][p] are equal, and A->str[n+1]=='\0':
		(also if A->str==NULL):
		if (A->next[word[i][p+n]] == NULL:	B
		else:	D

	    if first n characters of A->str and words[i][p] are equal, and words[i][p+n+1]=='\0':	A+C

	    if A->str equals words[i][p]:	C

	    A:
		create new node N1:
		    N1->str = &(A->str[n+1])
		    N1->next <-> A->next
		    A->next[A->str[n]] = N1
		    A->str[n] = '\0'	//--
		    N1->pos = A->pos
		    A->pos = -1
	    B:
		create new node N2:
		    N2->str = &(words[i][p+n+1])
		    A->next[words[i][p+n]] = N2
		    N2->pos = i
	    C:
		    A->pos = i
	    D:
		    p+= strlen(A->str)
		    A = A->next[word[i][p+n]]
*/
	    /** END PSEUDOCODE. **/


static inline automaton* new_automaton()
{
    int		i;
    automaton	*A = malloc(sizeof(automaton));

    for (i=0; i<256; i++)
	A->next[i] = NULL;

    A->terminal = -1;
    A->str = NULL;

    return A;
}

/*
void print_automaton( char *prefix, automaton *A )
{
    int		i;

    if (A->terminal>=0)
	printf("%s%s\n", prefix, A->str);

    for (i=0; i<255; i++)
	{
	    if (A->next[i]!=NULL)
		{
		    char	*new_prefix = malloc(strlen(prefix)+strlen((char*)A->str)+2);
		    strcpy(new_prefix, prefix);
		    strcat(new_prefix, (char*)A->str);
		    new_prefix[strlen(new_prefix)+1] = '\0';
		    new_prefix[strlen(new_prefix)] = (unsigned char)i;

		    print_automaton(new_prefix, A->next[i]);

		    free(new_prefix);
		}
	}
}
*/

static inline automaton* build_automaton( int num_args, unsigned char **words )
{
    automaton	*base = new_automaton();
    int		i, j;

    convert_to_lowercase(words[0]);
    base->str = (unsigned char*)strdup( (char*)words[0] );
//    printf("Adding %s\n", (char*)words[0]);
    base->terminal = 0;

    for (i=1; i<num_args; i++)
	{
	    automaton		*A = base;
	    int			p=0, n;
	    char		terminated = 0;

	    convert_to_lowercase(words[i]);
//	    printf("Adding %s\n", (char*)words[i]);

	    while (words[i][p]!='\0' && !terminated)
		{
		    for (n=0; A->str[n]==words[i][p+n] && A->str[n]!='\0' && words[i][p+n]!='\0'; n++);

		    // If A->str and words[i][p] share their first n characters, and len(A->str)>n:
		    if ((A->str[n]!=words[i][p+n]) && (A->str[n]!='\0'))
			{
			    automaton		*N = new_automaton();
			    unsigned char	*temp_str;

			    N->str = (unsigned char*)strdup( (char*)&(A->str[n+1]) );

			    for (j=0; j<256; j++)
				{
				    automaton	*temp_a;
				    temp_a = N->next[j];
				    N->next[j] = A->next[j];
				    A->next[j] = temp_a;
				}

			    A->next[A->str[n]] = N;

//			    printf("[%s] => [%c]->[%s]\n", &(A->str[n]), A->str[n], &(A->str[n+1]));

			    temp_str = A->str;
			    temp_str[n] = '\0';
			    A->str = (unsigned char*)strdup((char*)temp_str);
			    free(temp_str);

			    N->terminal = A->terminal;
			    A->terminal = -1;
			}

		    // If A->str and words[i][p] share their first n characters, and n< length of both,
		    // OR if n==len(A->str) and the path for words[i] stops here:
		    if ((A->str[n]!=words[i][p+n]) && (A->str[n]!='\0') && (words[i][p+n]!='\0')
		    || ((A->str[n]=='\0') && (words[i][p+n]!='\0') && (A->next[words[i][p+n]]==NULL)))
			{
			    automaton	*N = new_automaton();
//			    printf("[%c]->[%s]\n", words[i][p+n], &(words[i][p+n+1]));

			    N->str = (unsigned char*)strdup( (char*)&(words[i][p+n+1]) );
			    A->next[words[i][p+n]] = N;
			    N->terminal = i;
			    terminated = 1;
			}

		    // If A->str and words[i][p] are equal or len(words[i][p])==n
		    if (words[i][p+n]=='\0')
			{
//			    printf("terminal\n");
			    A->terminal = i;
			    terminated = 1;
			}

		    A = A->next[words[i][p+n]];
		    p+= n;
		    if (words[i][p]!='\0') p++;
//		    printf("-- next iteration --\n");
		}
	}
/*
    printf("Added words to automata:\n");
    print_automaton("", base);
    printf("---\n");
*/
    return base;
}


static inline void free_automaton( automaton* A )
{
    int		i;

    for (i=0; i<256; i++)
	{
	    if (A->next[i] != NULL)
		free_automaton(A->next[i]);
	}

    free(A->str);

    free(A);
}


static inline int search_automaton( automaton *base, const char *word )
{
    automaton		*A = base;
    int			i, n;
//    unsigned char	*word = (unsigned char*)_word;
    unsigned char	c, last=255;

//    printf("%s(%i)(%i) ?\n", word, (int)base, (int)word);

    for (n=0; A!=NULL; n++)
	{
	    for (i=0; word[n]!='\0' && A->str[i]!='\0'; n++,i++)
		{
		    c = word[n];
		    if ((c>='A' && c<='Z') || (last==0xc3 && c>=0x80 && c<=0x9e))
			c+= 32;	// 'a' - 'A'
		    last = c;

//		    printf("(%c,%c)", A->str[i], c);
		    if (c!=A->str[i])
			return -1;
		}

//	    printf("%c", word[n]);

	    if (word[n]=='\0')
		{
		    if (A->str[i]=='\0')
			return A->terminal;
		    else
			return -1;
		}

	    c = word[n];
	    if ((c>='A' && c<='Z') || (last==0xc3 && c>=0x80 && c<=0x9e))
		c+= 32;	// 'a' - 'A'
	    last = c;

	    A = A->next[c];
	}

    return -1;
}



#endif	// _SEARCH_AUTOMATON_H_
