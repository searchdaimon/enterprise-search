// TODO: Rydde i koden!
//runarb:
#include "bstr.h"

#include "langdetect.h"
#include "crc32.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <assert.h>

#define MALLOC(a) _d_malloc(a, __LINE__)
#define FREE(a) _d_free(a, __LINE__)


void* _d_malloc( size_t size, int line )
{
    void	*ptr = malloc(size);

    printf("malloc [line: %i] [ptr: %i]\n", line, ptr);

    return ptr;
}

void _d_free( void *ptr, int line )
{
    printf("free [line: %i] [ptr: %i]\n", line, ptr);
    free(ptr);
}


double sqr( double d )
{
    return d*d;
}


struct
{
    int		ant;
    char	code[3];
} typedef char3;

char3 new_char3( const char *c )
{
    char3	c3;

    c3.ant = 0;
    c3.code[0] = c[0];
    c3.code[1] = c[1];
    c3.code[2] = c[2];

    return c3;
}

struct
{
    int		size, maxsize;
    char3	*data;
} typedef _c_vector_char3;

_c_vector_char3 _c_vector_char3_init()
{
    _c_vector_char3	V;

    V.size = 0;
    V.maxsize = 0;
    V.data = NULL;

    return V;
}


void _c_vector_char3_push_back( _c_vector_char3 *V, char3 D )
{
    char3		*_data;

    if (V->size == V->maxsize)
	{
	    if (V->maxsize == 0)
		V->maxsize = 1;
	    else
		V->maxsize *= 2;

	    _data = (char3*)malloc( sizeof(char3) * V->maxsize );
	    memcpy( _data, V->data, sizeof(char3) * V->size );

	    if (V->data != NULL)
		free( V->data );

	    V->data = _data;
	}

    V->data[V->size] = D;
    V->size++;
}


struct
{
    unsigned int	key;
    _c_vector_char3	iso639_code;
} typedef _dict_elem;


int _dict_elem_compare( const void *_a, const void *_b )
{
    const unsigned int	a_key = *(unsigned int*)_a;
    const _dict_elem	*b = _b;

    if (a_key < b->key) return 1;
    if (a_key > b->key) return -1;
    return 0;
}


typedef struct _mapnode _mapnode;

struct _mapnode
{
    enum { Red, Black }	color;
    _mapnode		*parent, *left_child, *right_child;
    unsigned int	key;
    _c_vector_char3	iso639_code;
};

struct
{
    _mapnode		*root;
    int			size;
} typedef _maproot;


_maproot* _map_init()
{
    _maproot	*M = (_maproot*)malloc(sizeof(_maproot));

    M->root = NULL;
    M->size = 0;

    return M;
}

void _map_insert( _maproot *M, unsigned int key, char3 code )
{
    _mapnode		*it = M->root, *last_it = M->root;

    if (M->root == NULL)
	{
	    M->root = (_mapnode*)malloc(sizeof(_mapnode));
	    M->root->color = Black;
	    M->root->parent = NULL;
	    M->root->left_child = NULL;
	    M->root->right_child = NULL;
	    M->root->key = key;
	    M->root->iso639_code = _c_vector_char3_init();
	    _c_vector_char3_push_back( &(M->root->iso639_code), code );
	    M->size++;

	    return;
	}

    while (it!=NULL)
	{
	    if (it->key == key)
		{
		    _c_vector_char3_push_back( &(it->iso639_code), code );
		    return;
		}

	    last_it = it;

	    if (it->key < key)
		it = it->left_child;
	    else
		it = it->right_child;
	}

    it = (_mapnode*)malloc(sizeof(_mapnode));
    it->color = Red;
    it->parent = last_it;
    it->left_child = NULL;
    it->right_child = NULL;
    it->key = key;
    it->iso639_code = _c_vector_char3_init();
    _c_vector_char3_push_back( &(it->iso639_code), code );

    if (last_it->key < key)
        last_it->left_child = it;
    else
	last_it->right_child = it;

    M->size++;

/*
    // Red-Black-Tree:
    while (it != *root && it->parent->color == Red)
	{
	}
*/
}


_dict_elem	*_D;
int		_D_size;
int		langdetectSkip;


void langdetectInit()
{
    // First: Open files and read words into dictionary.
    _maproot	*dictionary = _map_init();
    char	*path = "data/stopwords/";
    DIR		*dir = opendir(path);
    int		total_num_words=0;
    int		i;

    #ifndef NOWARNINGS
    printf("langdetectInit\n");
    #endif

    langdetectSkip = 0;

    if (!dir)
	{
	    printf("langdetect: Could not find directory '%s'. Languagedetection will be disabled.\n", path);
	    langdetectSkip = 1;
	    return;
	}
/*
    //runarb: girbedre feilmelding her
    if (!dir){
	printf("can't find stopwords. langdetect will not be avalibal!\n"); 
	perror(path);
	exit(1);
    }
*/ 

    struct dirent	*entry;
    while ((entry = readdir(dir)) != NULL)
	{
	    // [A-Z][A-Z][A-Z].txt
	    if ((entry->d_name[0] >= 'A' && entry->d_name[0] <= 'Z')
		&& (entry->d_name[1] >= 'A' && entry->d_name[1] <= 'Z')
		&& (entry->d_name[2] >= 'A' && entry->d_name[2] <= 'Z')
		&& (entry->d_name[3] == '.')
		&& (entry->d_name[4] == 't')
		&& (entry->d_name[5] == 'x')
		&& (entry->d_name[6] == 't'))
		{
		    char	filename[32];
		    char3	iso639;

		    strncpy(filename, path, 31);
		    strncat(filename, entry->d_name, 31 - strlen(filename));

		    #ifdef DEBUG
		    printf("Reading stopwords from %s\n", filename);
		    #endif

		    iso639.code[0] = entry->d_name[0];
		    iso639.code[1] = entry->d_name[1];
		    iso639.code[2] = entry->d_name[2];

		    FILE	*ordliste = fopen(filename, "r");
		    assert(ordliste!=NULL);
		    char	s[64];

		    while (fscanf(ordliste, "%63s", &s)!=EOF)
			{
//			    printf("%s %i (%.3s)\n", s, crc32boitho(s), iso639.code);
			    _map_insert( dictionary, crc32boitho(s), iso639 );
			    total_num_words++;
			}

		    fclose(ordliste);
		}
	}

    closedir(dir);
    

    // Second: Convert dictionary to array:

    #ifdef DEBUG
    printf("%i unique words (%i total).\n", dictionary->size, total_num_words);
    #endif

    _D = (_dict_elem*)malloc(sizeof(_dict_elem) * dictionary->size);

    _D_size = dictionary->size;
    _map_save_and_delete( _D, 0, dictionary->root );
    free( dictionary );

//    for (i=0; i<20; i++)
//	printf("%u %.3s (+%i)\n", _D[i].key, _D[i].iso639_code.data[0].code, _D[i].iso639_code.size -1);
}

int _map_save_and_delete( _dict_elem *D, int _pos, _mapnode *it )
{
    int		pos = _pos;

    if (it->left_child != NULL)
	pos = _map_save_and_delete( D, pos, it->left_child );

    D[pos].key = it->key;
    D[pos].iso639_code = it->iso639_code;
//    printf("%i %.3s\n", it->key, it->iso639_code.data[0].code);
    pos++;

    if (it->right_child != NULL)
	pos = _map_save_and_delete( D, pos, it->right_child );

    free( it );

    return pos;
}

void langdetectDetect(struct wordsFormat words[],int nrofWords, char lang[])
{
    if (langdetectSkip)
	{
	    strscpy(lang,"ENG",4);
	    return;
	}

    _dict_elem		*ptr;
    _c_vector_char3	V = _c_vector_char3_init();
    int			i, j, k;
    int			current;
    int			alle_stoppord=0;

    for (i=0; i<nrofWords; i++)
	{
	    #ifdef DEBUG
//		printf("%s - ",words[i].word);
	    #endif

//	    void *bsearch(const void *key, const void *base, size_t nmemb,
//	                  size_t size, int (*compar)(const void *, const void *));
	    ptr = (_dict_elem*)bsearch( &(words[i].WordID), &(_D[0]), _D_size, sizeof(_dict_elem), _dict_elem_compare );

//	    printf("%u %u ", words[i].WordID, words[i].position);

	    if (ptr == NULL)
		{
//			printf("(not found)\n");
		}
	    else
		{
		    alle_stoppord++;

		    #ifdef DEBUG
			//dette fungerer bare når vi har på preserv words
			//printf("%s - %u %u ", words[i].word, words[i].WordID, words[i].position);
			printf("( ");
		    #endif

		    for (j=0; j<ptr->iso639_code.size; j++)
			{
			    current = -1;
			    for (k=0; k<V.size && current<0; k++)
				{
				    if ((V.data[k].code[0] == ptr->iso639_code.data[j].code[0])
					&& (V.data[k].code[1] == ptr->iso639_code.data[j].code[1])
					&& (V.data[k].code[2] == ptr->iso639_code.data[j].code[2]))
					{
					    V.data[k].ant++;
					    current = k;
					}
				}

			    if (current<0)
				{
				    _c_vector_char3_push_back( &V, ptr->iso639_code.data[j] );
				    V.data[V.size-1].ant = 1;
				    current = V.size -1;
				}

			    #ifdef DEBUG
				printf("%.3s[%i] ", ptr->iso639_code.data[j].code, V.data[current].ant);
			    #endif

			}

		    #ifdef DEBUG
			printf(")\n");
		    #endif

		    if (alle_stoppord%100 == 0)
			{
			    double	total = 0, beste = 0, nest_beste = 0, denne;
			    int		beste_i, nest_beste_i;

			    #ifndef NOWARNINGS
			    printf("Mellomresultat: ");
			    #endif

			    beste_i = 0;
			    nest_beste_i = 0;
			    for (k=0; k<V.size; k++)
				{
				    denne = sqr(((double)V.data[k].ant)/((double)alle_stoppord));
				    total+= denne;

				    if (denne > beste)
					{
					    if (beste > nest_beste)
						{
						    nest_beste_i = beste_i;
						    nest_beste = beste;
						}

					    beste_i = k;
		    			    beste = denne;
					}
				    else if (denne > nest_beste)
					{
					    nest_beste_i = k;
					    nest_beste = denne;
					}
				}

			    #ifdef DEBUG
			    for (k=0; k<V.size; k++)
				{
				    denne = sqr(((double)V.data[k].ant)/((double)alle_stoppord)) / total;

				    printf("%.3s[%i, %.2f%%] ", V.data[k].code, V.data[k].ant, denne*100);
				}
			    #endif

			    beste/= total;
			    nest_beste/= total;

			    #ifndef NOWARNINGS
			    printf(" => %.3s (%.2f%%), %.3s (%.2f%%)\n", V.data[beste_i].code, beste*100,
				V.data[nest_beste_i].code, nest_beste*100);
			    #endif
			    if (beste > 0.55 && beste > nest_beste*2)
				{
				    #ifndef NOWARNINGS
				    printf("langdetectDetect: [%.3s] Bra treff! Vi beholder denne og avslutter her.\n", V.data[beste_i].code);
				    #endif
				    //runarb:lang = (char*)strndup(V.data[beste_i].code, 3);
				    strncpy(lang,V.data[beste_i].code,3);
				    lang[3] = '\0';

				    goto langdetectDetectEnd;
				    //return;
				}
			}
		}
	}

    #ifndef NOWARNINGS
    printf("langdetectDetect: Dokument ferdig parset. ");
    #endif
    if (V.size == 0)
	{
	    #ifndef NOWARNINGS
	    printf("(ingen stoppord funnet)\n");
	    #endif

	    //runarb:lang = (char*)strndup("   ", 3);
	    strcpy(lang,"");
	}
    else
	{
	    double	total = 0, beste = 0, denne;
	    int		beste_i;

	    beste_i = 0;
	    for (k=0; k<V.size; k++)
		{
		    denne = sqr(((double)V.data[k].ant)/((double)alle_stoppord));
		    total+= denne;

		    if (denne > beste)
			{
			    beste_i = k;
			    beste = denne;
			}
		}

	    #ifdef DEBUG
	    for (k=0; k<V.size; k++)
		{
		    denne = sqr(((double)V.data[k].ant)/((double)alle_stoppord)) / total;

		    printf("%.3s[%i, %.2f%%] ", V.data[k].code, V.data[k].ant, denne*100);
		}
	    #endif

	    #ifndef NOWARNINGS
	    printf(" => %.3s (%.2f%%)\n", V.data[beste_i].code, (beste*100)/total);
	    #endif
	    //runarb: lang = (char*)strndup(V.data[beste_i].code, 3);
 	    strncpy(lang,V.data[beste_i].code,3);
            lang[3] = '\0';

	}


	langdetectDetectEnd:	
    	//runarb: 28 mars 2008
	free(V.data);	

}

void langdetectDestroy()
{
    int		i;

    #ifndef NOWARNINGS
    printf("langdetectDestroy\n");
    #endif

    if (langdetectSkip) return;

    for (i=0; i<_D_size; i++)
	free( _D[i].iso639_code.data );

    free( _D );
}

/*
int main()
{
    _dict_elem	*ptr;
    int		i, j;
    unsigned int	key;

    langdetectInit();

    key = crc32boitho("kan");
    ptr = (_dict_elem*)bsearch( &(key), &(_D[0]), _D_size, sizeof(_dict_elem), _dict_elem_compare );

    printf("%u ", key);

    if (ptr == NULL)
	printf("(not found)\n");
    else
	{
	    printf("( ");
	    for (j=0; j<ptr->iso639_code.size; j++)
		printf("%.3s ", ptr->iso639_code.data[j].code);
	    printf(")\n");
	}

    langdetectDestroy();
}
*/
