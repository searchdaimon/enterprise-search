%{

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "../common/utf8-strings.h"
#include "../common/search_automaton.h"
#include "../query/query_parser.h"
#include "../ds/dcontainer.h"
#include "../ds/dpair.h"
#include "../ds/dvector.h"
#include "../ds/dqueue.h"
#include "snippet.parser.common.h"

// --- fra flex:
typedef void* yyscan_t;
typedef struct bsgp_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE bsgp_scan_bytes( const char *bytes, int len, yyscan_t yyscanner );
struct bsgp_yy_extra *bsgpget_extra( yyscan_t yyscanner );
// ---


struct bsg_intern_data
{
    int		bsize, bpos;
    char	*buf;
    container	*Q, *VMatch, *VSection, *History;
    int		history_start;
    int		VSection_start;
    automaton	*A;
    int		*q_dep;
    char	*q_stop;
    int		last_match, q_start;
    int		VMatch_start;
    int		klamme_firkant, klamme_rund, klamme_sikksakk;
    int		q_flags;
    int		best_score, best_start, best_stop;
    int		snippet_size;
};

const int	show = 0;
const int	v_section_div=1, v_section_head=2, v_section_span=4;

static inline int buf_printf(struct bsg_intern_data *data, const char *fmt, ...);
static inline void test_for_snippet(struct bsg_intern_data *data, char forced);

%}

%pure-parser
%parse-param { struct bsg_intern_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token DIV_START DIV_END HEAD_START HEAD_END SPAN_START SPAN_END WORD EOS PARANTES OTHER

%%
doc	:
	| doc div
	;
div	: div_start paragraph DIV_END
	    {
		if (show) buf_printf(data, "\033[1;33m}\033[0m");
	    }
	;
div_start : DIV_START
	    {
		data->q_flags|= v_section_div;
		if (show) buf_printf(data, "\033[1;33m{\033[0m");
//		vector_pushback(data->History, data->bpos, v_section_div);
	    }
	;
paragraph :
	| paragraph head_start spans head_end
	| paragraph spans
	;
head_start : HEAD_START
	    {
		data->q_flags|= v_section_head;
		if (show) buf_printf(data, "\033[1;34m[\033[0m");
	    }
	;
head_end : HEAD_END
	    {
		if (show) buf_printf(data, "\033[1;34m]\033[0m");
//		vector_pushback(data->History, data->bpos, v_section_head);
	    }
	;
spans	:
	| spans span_start sentence span_end
	;
span_start : SPAN_START
	    {
		data->q_flags|= v_section_span;
		if (show) buf_printf(data, "\033[1;35m(\033[0m");
	    }
	;
span_end : SPAN_END
	    {
		if (show) buf_printf(data, "\033[1;35m)\033[0m");
//		vector_pushback(data->History, data->bpos, v_section_span);
	    }
	;
sentence :
	| sentence WORD
	{
    	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }

//	    printf("(%s)\n", (char*)$2);

	    queue_push(data->Q, data->bpos, data->q_flags);
	    data->q_flags = 0;

	    int	ret = search_automaton( data->A, (char*)$2 );
//	    if (ret>=0)
//		{ printf("match: %s\n", (char*)$2); }

	    if (ret>=0)
		{
		    if (data->q_dep[ret] == -1)
			data->q_start = data->bpos;

		    // First check phrase dependencies:
		    if (data->q_dep[ret] == -1 || data->q_dep[ret] == data->last_match)
			{
//			    buf_printf(data, "\033[1;36m(%s)\033[0m", (char*)$2);
			    buf_printf(data, "%s", (char*)$2);

			    // Then check if this is the last word in the phrase:
			    if (data->q_stop[ret] == 1)
				{
				    vector_pushback(data->VMatch, data->q_start, data->bpos);
				}
			}
		    else
			{
			    buf_printf(data, "%s", (char*)$2);
			}
//		    buf_printf(data, "\033[1;36m%s\033[0m", (char*)$2);
		}
	    else
		{
		    buf_printf(data, "%s", (char*)$2);
		}

	    data->last_match = ret;

	    test_for_snippet(data,0);
	}
	| sentence EOS
	{
	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }
	    buf_printf(data, "%s", (char*)$2);
	}
	| sentence PARANTES
	{
	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }
//	    buf_printf(data, "%s", (char*)$2);

	    queue_push(data->Q, data->bpos, data->q_flags);
	    data->q_flags = 0;
	    buf_printf(data, "%s", (char*)$2);
/*
	    switch (((char*)$2)[0])
		{
		    case '(': data->klamme_rund++; break;
		    case ')': data->klamme_rund--; break;
		    case '[': data->klamme_firkant++; break;
		    case ']': data->klamme_firkant--; break;
		    case '{': data->klamme_sikksakk++; break;
		    case '}': data->klamme_sikksakk--; break;
		}
*/
	    test_for_snippet(data,0);
	}
	| sentence OTHER
	{
	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }
	    buf_printf(data, "%s", (char*)$2);
	}
	;
%%


static inline int buf_printf(struct bsg_intern_data *data, const char *fmt, ...)
{
    va_list	ap;
    int		size, len_printed;

//    printf("_");
    va_start(ap, fmt);

    size = data->bsize - data->bpos - 1;

    len_printed = vsnprintf(&(data->buf[data->bpos]), size, fmt, ap);

    // Dersom bufferet går fullt, dobler vi størrelsen:
    if (len_printed + data->bpos + 2 > data->bsize)
	{
	    // Allocate a new buffer, twice the size:
	    char	*new_buf;
	    int		new_size;

//	    printf("new_size = %i (%i)\n", data->bsize, len_printed);

	    if (len_printed < data->bsize)
		new_size = data->bsize*2;
	    else
		// Lite sannsynlig, men *kan* skje:
		new_size = data->bsize*2 + len_printed;

	    new_buf = malloc(new_size);
	    memcpy(new_buf, data->buf, data->bsize);

	    free(data->buf);
	    data->buf = new_buf;
	    data->bsize = new_size;

	    va_end(ap);
	    va_start(ap, fmt);
	    size = data->bsize - data->bpos - 1;
	    len_printed = vsnprintf(&(data->buf[data->bpos]), size, fmt, ap);
	}

    data->bpos+= len_printed;

    va_end(ap);

    return len_printed;
}


static inline void test_for_snippet(struct bsg_intern_data *data, char forced)
{
//    if (queue_size(data->Q) > 0)
//	{
	    while (queue_size(data->Q) > 0 && ((data->bpos - pair(queue_peak(data->Q)).first.i) > data->snippet_size || forced))
		{
		    value	temp = queue_peak(data->Q);
		    int 	pos, flags;
		    value	phrase;
		    int		i, j;
		    char	m, n;
		    int		p_hits;
		    int		score=0;

		    pos = pair(temp).first.i;
		    flags = pair(temp).second.i;
/*
		    if (flags & v_section_div)
			printf("D");
		    else
			printf("_");

		    if (flags & v_section_head)
			printf("H");
		    else
			printf("_");

		    if (flags & v_section_span)
			printf("S");
		    else
			printf("_");
*/

		    for (i=data->VMatch_start; i<vector_size(data->VMatch)
			&& pair(vector_get(data->VMatch,i)).first.i < pos; i++);
		    data->VMatch_start = i;
/*
		    m = (i < vector_size(data->VMatch));

		    if (m)
			{
			    phrase = vector_get(data->VMatch,i);
			}
*/
/*
		    for (j=data->history_start; j<vector_size(data->History)
			&& pair(vector_get(data->History,j)).first.i < pos; j++);
		    data->history_start = j;

		    if (j < vector_size(data->History))
			{
//			    for (; j<vector_size(data->History)
//				&& pair(vector_get(data->History,j)).first.i ==
			}

		    n = (j < vector_size(data->History));

		    if (n)
			{
			    section = vector_get(data->History,j);
			}
*/

		    p_hits = vector_size(data->VMatch) - data->VMatch_start;
//		    printf("[%i] ", p_hits);

		    // Calculate score:
		    if (p_hits>0)
			score|= 256;
		    if ((flags & v_section_div) && !(flags & v_section_head))
			score|= 128;
		    if ((flags & v_section_div) && (flags & v_section_head))
			score|= 64;
		    if (!(flags & v_section_div) && (flags & v_section_head))
			score|= 32;
		    if ((flags & v_section_span))
			score|= 16;

		    if (p_hits < 8)
			score|= p_hits;
		    else
			score|= 7;

		    if (score > data->best_score)
			{
			    data->best_score = score;
			    data->best_start = pos;
			    data->best_stop = data->bpos;
			}
/*
		    printf("%5i ", score);

		    for (;pos < data->bpos; pos++)
			{
			    if (m && pos == pair(phrase).first.i)
				printf("\033[1;36m\'");

			    if (m && pos == pair(phrase).second.i)
				{
				    printf("\'\033[0m");
				    i++;
				    m = (i < vector_size(data->VMatch));

				    if (m)
					{
					    phrase = vector_get(data->VMatch,i);
					}
				}

			    printf("%c", data->buf[pos]);
			}
		    if (m && pos == pair(phrase).second.i)
			printf("\'\033[0m");
		    printf("\n");
*/
		    queue_pop(data->Q);
		}
//	}
}


static inline char* print_best_snippet( struct bsg_intern_data *data, char* b_start, char* b_end )
{
    int		i, pos;
    char	m;
    int		bsize = data->snippet_size*5;
    char	buf[bsize];
    int		bpos=0;
    value	phrase;
    int		active_highl=0;

    for (i=0; i<vector_size(data->VMatch) && pair(vector_get(data->VMatch,i)).first.i < data->best_start; i++);

    m = (i < vector_size(data->VMatch));

    if (m)
	{
	    phrase = vector_get(data->VMatch,i);
	}

    for (pos=data->best_start; pos<data->best_stop; pos++)
	{
	    if (m && pos == pair(phrase).first.i)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_start);
//		bpos+= snprintf(buf+bpos, bsize-bpos-1, "\033[1;32m'");
		    active_highl++;
		}

	    if (m && pos == pair(phrase).second.i)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
//		    bpos+= snprintf(buf+bpos, bsize-bpos-1, "\'\033[0m");
		    active_highl--;
		    i++;
		    m = (i < vector_size(data->VMatch));

		    if (m)
			{
			    phrase = vector_get(data->VMatch,i);
			}
		}

	    bpos+= snprintf(buf+bpos, bsize-bpos-1, "%c", data->buf[pos]);
	}

//    if (m && pos <= pair(phrase).second.i)
    if (active_highl>0)
	bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
//	bpos+= snprintf(buf+bpos, bsize-bpos-1, "\'\033[0m");

//    printf("\n");
    buf[bpos++] = ' ';
    buf[bpos++] = '.';
    buf[bpos++] = '.';
    buf[bpos++] = '.';
    buf[bpos] = '\0';

    return strdup(buf);
}


void generate_snippet( query_array qa, char text[], int text_size, char **output_text, char* b_start, char* b_end, int _snippet_size )
{
    struct bsgp_yy_extra	*he = malloc(sizeof(struct bsgp_yy_extra));
    struct bsg_intern_data	*data = malloc(sizeof(struct bsg_intern_data));
    int				i, j, qw_size=0, qw_i;
    unsigned char		**qw;

    he->stringtop = 0;
    he->space = 0;

    data->snippet_size = _snippet_size;

    data->bpos = 0;
    data->bsize = 65536;
    data->buf = malloc(data->bsize);
    data->Q = queue_container( pair_container( int_container(), int_container() ) );
    data->VMatch = vector_container( pair_container( int_container(), int_container() ) );
    data->VMatch_start = 0;
//    data->History = vector_container( pair_container( int_container(), int_container() ) );
//    data->history_start = 0;
    data->q_flags = 0;

    for (i=0; i<qa.n; i++)
	qw_size+= qa.query[i].n;

    qw = malloc(sizeof(char*)*qw_size);
    data->q_dep = malloc(sizeof(int)*qw_size);
    data->q_stop = malloc(sizeof(char)*qw_size);
    data->last_match = -1;
    data->q_start = -1;

    for (i=0,qw_i=0; i<qa.n; i++)
	{
	    for (j=0; j<qa.query[i].n; j++)
		{
		    if (j==0)
			data->q_dep[qw_i] = -1;
		    else
			data->q_dep[qw_i] = qw_i-1;

		    if (j==(qa.query[i].n-1))
			data->q_stop[qw_i] = 1;
		    else
			data->q_stop[qw_i] = 0;

		    qw[qw_i++] = (unsigned char*)qa.query[i].s[j];
		}
	}    

    data->A = build_automaton(qw_size, qw);
    free(qw);

    data->klamme_firkant = 0;
    data->klamme_rund = 0;
    data->klamme_sikksakk = 0;

    data->best_score = 0;
    data->best_start = 0;
    data->best_stop = 0;

    yyscan_t	scanner;
    int		yv;

    bsgplex_init( &scanner );
    bsgpset_extra( he, scanner );

    YY_BUFFER_STATE	bs = bsgp_scan_bytes( text, text_size, scanner );

    while ((yv = bsgpparse(data, scanner)) != 0)
	{
	}

    test_for_snippet(data,1);
    if (data->best_stop==0)
	data->best_stop = data->bpos;
    *output_text = print_best_snippet(data, b_start, b_end);

    bsgp_delete_buffer( bs, scanner );
    bsgplex_destroy( scanner );

    free_automaton(data->A);
    destroy(data->VMatch);
    destroy(data->Q);

    free(data->q_stop);
    free(data->q_dep);
//    printf("%s\n", data->buf);
    free(data->buf);

    free(data);
    free(he);
}


bsgperror( struct bsg_intern_data *data, yyscan_t scanner, char *s )
{
    fprintf(stderr, "Parse error: %s\n", s);
}

