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
    container	*Q, *Q2, *VMatch, *VSection, *VHit;
    int		VSection_start;
    automaton	*A;
    int		*q_dep;
    char	*q_stop;
    int		last_match, q_start;
    int		VMatch_start, VMatch_start2;
    int		q_flags;
    int		best_score, best_start, best_stop, best_hits;
    int		*q_best_score, *q_best_start, *q_best_stop, *q_best_hits;
    int		snippet_size;
    int		tilstand;
    int		**d;
    int		*accepted;
    int		*history;
    int		history_crnt, history_size;
    int		*phrase_sizes;
    int		num_queries;
};

const int	show = 0;
const int	v_section_div=1, v_section_head=2, v_section_span=4, v_section_sentence=8;

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
	    }
	;
sentence :
	| sentence WORD
	{
    	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }

	    queue_push(data->Q, data->bpos, data->q_flags);
	    queue_push(data->Q2, data->bpos, data->q_flags);
	    data->q_flags = 0;

	    int	ret = search_automaton( data->A, (char*)$2 );

	    if (ret>=0)
		{
		    data->history[data->history_crnt] = data->bpos;
		    data->history_crnt++;
		    if (data->history_crnt >= data->history_size)
			data->history_crnt = 0;

		    buf_printf(data, "%s", (char*)$2);

		    data->tilstand = data->d[data->tilstand][ret];

		    if (data->accepted[data->tilstand] >= 0)
			{
			    int		slot;
			    int		vsize;

			    slot = data->history_crnt;
			    slot-= data->phrase_sizes[data->accepted[data->tilstand]];
			    while (slot<0) slot+= data->history_size;

			    // Sjekk for overlappende treff:
			    vsize = vector_size(data->VMatch);
			    if (vsize>0 && (data->history[slot] <= pair(vector_get(data->VMatch, vsize-1)).second.i))
				{
				    value	v;

				    pair(vector_get(data->VMatch, vsize-1)).second.i = data->bpos;
				    v.i = data->accepted[data->tilstand];
				    vector_set(data->VHit, vsize-1, v);
				}
			    else
				{
				    vector_pushback(data->VMatch, data->history[slot], data->bpos);
				    vector_pushback(data->VHit, data->accepted[data->tilstand]);
				}
			}
		}
	    else
		{
		    buf_printf(data, "%s", (char*)$2);
		    data->tilstand = 0;
		}

	    data->last_match = ret;

	    test_for_snippet(data,0);
	}
	| sentence EOS
	{
    	    data->q_flags|= v_section_sentence;

	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }
	    buf_printf(data, "%s", (char*)$2);

	    test_for_snippet(data,0);
	}
	| sentence PARANTES
	{
	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }

	    queue_push(data->Q, data->bpos, data->q_flags);
	    queue_push(data->Q2, data->bpos, data->q_flags);
	    data->q_flags = 0;
	    buf_printf(data, "%s", (char*)$2);

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


// TODO: Rydde i denne funksjonen:
static inline void test_for_snippet(struct bsg_intern_data *data, char forced)
{
	    while (queue_size(data->Q) > 0 && (((data->bpos - pair(queue_peak(data->Q)).first.i) > data->snippet_size) || forced))
		{
		    value	temp = queue_peak(data->Q);
		    int 	pos, flags;
		    value	phrase;
		    int		i;
		    char	m;
		    int		d_hits;
		    int		score=0;

		    pos = pair(temp).first.i;
		    flags = pair(temp).second.i;

#ifdef DEBUG
		    if (forced)
			printf("frc ");
		    else
			printf("%i ", (data->bpos - pair(queue_peak(data->Q)).first.i) );

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

		    if (flags & v_section_sentence)
			printf("s");
		    else
			printf("_");
#endif

		    for (i=data->VMatch_start; i<vector_size(data->VMatch)
			&& pair(vector_get(data->VMatch,i)).first.i < pos; i++);
		    data->VMatch_start = i;

		    m = (i < vector_size(data->VMatch));

		    if (m)
			{
			    phrase = vector_get(data->VMatch,i);
			}

		    int		treff[data->num_queries];
		    int		sum_hits = 0;
		    int		k;
		    int		bin_hits = 0;

		    for (k=0; k<data->num_queries; k++)
			treff[k] = 0;

		    for (k=data->VMatch_start; k<vector_size(data->VHit); k++)
			{
			    int		a = vector_get(data->VHit,k).i;
			    treff[a]++;
			    sum_hits+= data->phrase_sizes[a];

			    if (a<32) bin_hits|= (1<<a);
			}

		    d_hits = 0;
		    for (k=0; k<data->num_queries; k++)
			if (treff[k])
			    d_hits++;

//		    printf("[%i] ", d_hits);

		    // Calculate score:
		    score|= (d_hits<<9);
		    if (sum_hits > 15)
			score|= 0xf0;
		    else
			score|= (sum_hits<<5);

		    if ((flags & v_section_div) && !(flags & v_section_head))
			score|= 16;
		    if ((flags & v_section_div) && (flags & v_section_head))
			score|= 8;
		    if (!(flags & v_section_div) && (flags & v_section_head))
			score|= 4;
		    if ((flags & v_section_span))
			score|= 2;
		    if ((flags & v_section_sentence))
			score|= 1;

		    if (score > data->best_score)
			{
			    data->best_score = score;
			    data->best_start = pos;
			    data->best_stop = data->bpos;
			    data->best_hits = bin_hits;
			}
#ifdef DEBUG
		    printf("%5i ", score);

		    for (;pos < data->bpos; pos++)
			{
			    if (m && pos == pair(phrase).first.i)
				{
				    printf("\033[1;36m\'");
				}

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
#endif
		    queue_pop(data->Q);

		    if (forced && queue_size(data->Q) > 0 &&  ((data->bpos - pair(queue_peak(data->Q)).first.i) < (data->snippet_size*3/4)))
			break;
		}

  // *******************************************************

	    while (queue_size(data->Q2) > 0 && (((data->bpos - pair(queue_peak(data->Q2)).first.i) > (data->snippet_size/2)) || forced))
		{
		    value	temp = queue_peak(data->Q2);
		    int 	pos, flags;
		    value	phrase;
		    int		i;
		    char	m;
		    int		d_hits;
		    int		score=0;

		    pos = pair(temp).first.i;
		    flags = pair(temp).second.i;

#ifdef DEBUG2
		    if (forced)
			printf("frc ");
		    else
			printf("%i ", (data->bpos - pair(queue_peak(data->Q)).first.i) );

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

		    if (flags & v_section_sentence)
			printf("s");
		    else
			printf("_");
#endif

		    for (i=data->VMatch_start2; i<vector_size(data->VMatch)
			&& pair(vector_get(data->VMatch,i)).first.i < pos; i++);
		    data->VMatch_start2 = i;

		    m = (i < vector_size(data->VMatch));

		    if (m)
			{
			    phrase = vector_get(data->VMatch,i);
			}

		    int		treff[data->num_queries];
		    int		sum_hits = 0;
		    int		k;
		    int		bin_hits = 0;

		    for (k=0; k<data->num_queries; k++)
			treff[k] = 0;

		    for (k=data->VMatch_start2; k<vector_size(data->VHit); k++)
			{
			    int		a = vector_get(data->VHit,k).i;
			    treff[a]++;
			    sum_hits+= data->phrase_sizes[a];

			    if (a<32) bin_hits|= (1<<a);
			}

		    d_hits = 0;
		    for (k=0; k<data->num_queries; k++)
			if (treff[k])
			    d_hits++;

//		    printf("[%i] ", d_hits);

		    // Calculate score:
		    score|= (d_hits<<9);
		    if (sum_hits > 15)
			score|= 0xf0;
		    else
			score|= (sum_hits<<5);

		    if ((flags & v_section_div) && !(flags & v_section_head))
			score|= 16;
		    if ((flags & v_section_div) && (flags & v_section_head))
			score|= 8;
		    if (!(flags & v_section_div) && (flags & v_section_head))
			score|= 4;
		    if ((flags & v_section_span))
			score|= 2;
		    if ((flags & v_section_sentence))
			score|= 1;

		    for (k=0; k<data->num_queries; k++)
			{
			    if (treff[k] && score > data->q_best_score[k])
				{
//				    printf("(%i)", k);
				    data->q_best_score[k] = score;
				    data->q_best_start[k] = pos;
				    data->q_best_stop[k] = data->bpos;
				    data->q_best_hits[k] = bin_hits;
				}
			}
#ifdef DEBUG2
		    printf("%5i ", score);
		    printf("{%i} ", bin_hits);

		    for (;pos < data->bpos; pos++)
			{
			    if (m && pos == pair(phrase).first.i)
				{
				    printf("\033[1;36m\'");
				}

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
#endif
		    queue_pop(data->Q2);

		    if (forced && queue_size(data->Q2) > 0 &&  ((data->bpos - pair(queue_peak(data->Q2)).first.i) < (data->snippet_size*3/8)))
			break;
		}
}


// TODO: Rydde i denne funksjonen:
static inline char* print_best_snippet( struct bsg_intern_data *data, char* b_start, char* b_end )
{
    int		i, pos;
    char	m;
    int		bsize = data->snippet_size*5;
    char	buf[bsize];
    int		bpos=0;
    value	phrase;
    int		active_highl=0;
    char	last_was_eos=0;

    for (i=0; i<vector_size(data->VMatch) && pair(vector_get(data->VMatch,i)).first.i < data->best_start; i++);

    m = (i < vector_size(data->VMatch));

    if (m)
	{
	    phrase = vector_get(data->VMatch,i);
	}

//    bpos+= sprintf(buf+bpos, "[[%i]]", data->snippet_size);

    for (pos=data->best_start; pos<data->best_stop; pos++)
	{
	    if (m && pos == pair(phrase).first.i)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_start);
		    active_highl++;
		}

	    if (m && pos == pair(phrase).second.i)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
		    active_highl--;
		    i++;
		    m = (i < vector_size(data->VMatch));

		    if (m)
			{
			    phrase = vector_get(data->VMatch,i);
			}
		}

	    bpos+= snprintf(buf+bpos, bsize-bpos-1, "%c", data->buf[pos]);

	    switch (data->buf[pos])
		{
		    case '.': case ';': case ':': case '!': case '?': last_was_eos = 1; break;
		    case ' ': break;
		    default: last_was_eos = 0;
		}
	}

    if (active_highl>0)
	bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);

    if (data->bpos - data->best_stop < 5)
	{
	    snprintf(buf+bpos, bsize-bpos-1, "%s", &(data->buf[pos]));
	}
    else if (!last_was_eos && data->best_stop < data->bpos)
	{
	    buf[bpos++] = ' ';
	    buf[bpos++] = '.';
	    buf[bpos++] = '.';
	    buf[bpos++] = '.';
	    buf[bpos] = '\0';
	}

    return strdup(buf);
}


// TODO: Rydde i denne funksjonen:
static inline char* print_best_dual_snippet( struct bsg_intern_data *data, char* b_start, char* b_end )
{
    int		i, j, x;
//    int		bestebeste=0, nestebeste=0;
    int		nr, nr1=0, nr2=0;
    int		bsize = data->snippet_size*5;
    char	buf[bsize];
    int		bpos=0;
    int		best_hits = 0;

    // Finn de to mini-snippetene som utfyller hverandre best:

    for (i=0; i<data->num_queries; i++)
	for (j=i+1; j<data->num_queries; j++)
	    {
		int	bin_hits = data->q_best_hits[i] | data->q_best_hits[j];

		if (bin_hits > best_hits)
		    {
			best_hits = bin_hits;

			if (data->q_best_start[i] < data->q_best_start[j])
			    {
				nr1 = i;
				nr2 = j;
			    }
			else
			    {
				nr1 = j;
				nr2 = i;
			    }
		    }
	    }

    // Dersom de to mini-snippetene tilsammen ikke er bedre enn den store, så skrive ut vanlig snippet istedet:
    if ((nr1==nr2) || (data->q_best_score[nr1] == 0 || data->q_best_score[nr2] == 0)
	|| (data->q_best_start[nr1] == data->q_best_start[nr2])
	|| (best_hits < data->best_hits)
	|| (best_hits == data->best_hits && (((data->q_best_score[nr1] | data->q_best_score[nr2]) & 31) <= data->best_score)))
	{
	    return print_best_snippet( data, b_start, b_end );
	}

    // Skriv ut to mini-snippets med '...' imellom:
    for (nr=nr1, x=0; x<2; nr=nr2, x++)
	{
	    int		pos;
	    char	m;
	    value	phrase;
	    int		active_highl=0;
	    char	last_was_eos=0;

//	    bpos+= sprintf(buf+bpos, "((%i))", nr);

	    for (i=0; i<vector_size(data->VMatch) && pair(vector_get(data->VMatch,i)).first.i < data->q_best_start[nr]; i++);

	    m = (i < vector_size(data->VMatch));

	    if (m)
		{
		    phrase = vector_get(data->VMatch,i);
		}

	    for (pos=data->q_best_start[nr]; pos<data->q_best_stop[nr]; pos++)
		{
		    if (m && pos == pair(phrase).first.i)
			{
			    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_start);
			    active_highl++;
			}

		    if (m && pos == pair(phrase).second.i)
			{
			    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
			    active_highl--;
			    i++;
			    m = (i < vector_size(data->VMatch));

			    if (m)
				{
				    phrase = vector_get(data->VMatch,i);
				}
			}

		    bpos+= snprintf(buf+bpos, bsize-bpos-1, "%c", data->buf[pos]);

		    if (x==1)
		    switch (data->buf[pos])
			{
			    case '.': case ';': case ':': case '!': case '?': last_was_eos = 1; break;
			    case ' ': break;
			    default: last_was_eos = 0;
			}
		}

	    if (active_highl>0)
		bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);

	    if (x==0)
		{
		    buf[bpos++] = ' ';
		    buf[bpos++] = '.';
		    buf[bpos++] = '.';
		    buf[bpos++] = '.';
		    buf[bpos++] = ' ';
		}
	    else if (data->bpos - data->q_best_stop[nr] < 5)
		{
		    snprintf(buf+bpos, bsize-bpos-1, "%s", &(data->buf[pos]));
		}
	    else if (!last_was_eos && data->q_best_stop[nr] < data->bpos)
		{
		    buf[bpos++] = ' ';
		    buf[bpos++] = '.';
		    buf[bpos++] = '.';
		    buf[bpos++] = '.';
		    buf[bpos] = '\0';
		}
	}

    return strdup(buf);
}


void generate_snippet( query_array qa, char text[], int text_size, char **output_text, char* b_start, char* b_end, int _snippet_size )
{
    struct bsgp_yy_extra	*he = malloc(sizeof(struct bsgp_yy_extra));
    struct bsg_intern_data	*data = malloc(sizeof(struct bsg_intern_data));
    int				i, j, k, found, qw_size=0, num_qw, longest_phrase=0, sigma_size;
    unsigned char		**qw;
    int				*sigma;

    he->stringtop = 0;
    he->space = 0;

    data->snippet_size = _snippet_size;

    data->bpos = 0;
    data->bsize = 65536;
    data->buf = malloc(data->bsize);
    data->Q = queue_container( pair_container( int_container(), int_container() ) );
    data->Q2 = queue_container( pair_container( int_container(), int_container() ) );
    data->VMatch = vector_container( pair_container( int_container(), int_container() ) );
    data->VHit = vector_container( int_container() );
    data->VMatch_start = 0;
    data->VMatch_start2 = 0;
    data->q_flags = 0;

    for (i=0; i<qa.n; i++)
	qw_size+= qa.query[i].n;

    qw = malloc(sizeof(char*)*qw_size);
    sigma = malloc(sizeof(int)*qw_size);
    data->q_dep = malloc(sizeof(int)*qw_size);
    data->q_stop = malloc(sizeof(char)*qw_size);
    data->last_match = -1;
    data->q_start = -1;

    data->tilstand = 0;
    data->num_queries = qa.n;

    for (i=0,sigma_size=0,num_qw=0; i<qa.n; i++)
	{
	    if (qa.query[i].n > longest_phrase)
		longest_phrase = qa.query[i].n;

	    for (j=0; j<qa.query[i].n; j++)
		{
		    if (j==0)
			data->q_dep[sigma_size] = -1;
		    else
			data->q_dep[sigma_size] = sigma_size-1;

		    if (j==(qa.query[i].n-1))
			data->q_stop[sigma_size] = 1;
		    else
			data->q_stop[sigma_size] = 0;

		    found = -1;
		    for (k=0; k<sigma_size; k++)
			if (!strcmp(qa.query[i].s[j], (char*)qw[k]))
			    {
				found = k;
				break;
			    }

		    if (found>=0)
			{
			    sigma[num_qw++] = found;
			}
		    else
			{
			    sigma[num_qw++] = sigma_size;
			    qw[sigma_size++] = (unsigned char*)qa.query[i].s[j];
			}
		}
	}

    int		state, num_states=1;
    int		P[num_qw+1][longest_phrase+1];

    data->phrase_sizes = malloc(sizeof(int)*qa.n);
    data->history = malloc(sizeof(int)*longest_phrase);
    data->history_crnt = 0;
    data->history_size = longest_phrase;
    data->accepted = malloc(sizeof(int)*(num_qw+1));
    data->d = malloc(sizeof(int*)*(num_qw+1));
    for (i=0; i<num_qw+1; i++)
	data->d[i] = malloc(sizeof(int)*sigma_size);

    for (i=0; i<num_qw+1; i++)
	{
	    for (j=0; j<sigma_size; j++)
		data->d[i][j] = -1;

	    data->accepted[i] = -1;
	}

    P[0][0] = -1;
    for (i=0,num_qw=0; i<qa.n; i++)
	{
	    state = 0;

	    for (j=0; j<qa.query[i].n; j++)
		{
		    int		last_state = state;

		    if (data->d[state][sigma[num_qw]] == -1)
			{
			    data->d[state][sigma[num_qw]] = num_states;
			    state = num_states;
			    num_states++;
			}
		    else
			{
			    state = data->d[state][sigma[num_qw]];
			}

		    if (j>0)
			{
			    for (k=0; k<j; k++)
			    {
				P[state][k] = P[last_state][k];
			    }
			}
		    P[state][j] = sigma[num_qw];
		    P[state][j+1] = -1;
		    num_qw++;
		}

	    data->accepted[state] = i;
	    data->phrase_sizes[i] = qa.query[i].n;
	}
/*
    printf("%i %i\n", num_states, num_qw);
    assert(num_states <= num_qw+1);
*/
    // Generere "tilbakehopp" for tilstandsmaskin:
    for (i=0; i<num_states; i++)
	{
	    for (j=0; j<sigma_size; j++)
		{
		    if (data->d[i][j]==-1)
			{
			    int		nstate=-1;
			    int		l;

			    if (P[i][0] == -1)
				data->d[i][j] = 0;
			    else
				{
				    for (l=1; ; l++)
					{
					    nstate = 0;

					    for (k=l; P[i][k]!=-1; k++)
						{
						    nstate = data->d[nstate][P[i][k]];
						    if (nstate==-1)
							break;
						}

					    if (nstate!=-1)
						{
						    nstate = data->d[nstate][j];
						    if (nstate!=-1)
							break;
						}

					    if (P[i][l] == -1)
						break;
					}

				    if (nstate==-1)
					data->d[i][j] = 0;
				    else
					data->d[i][j] = nstate;
				}
			}
		}
	}

#ifdef DEBUG
    printf("   ");
    for (i=0; i<sigma_size; i++)
	printf("%c ", 'a'+i);
    printf("\n--");
    for (i=0; i<sigma_size; i++)
	printf("--", i);
    printf("\n");

    for (i=0; i<num_states; i++)
	{
	    printf("%i: ", i);
	    for (j=0; j<sigma_size; j++)
		{
		    printf("%i ", data->d[i][j]);
		}
	    printf("    (%i)\n", data->accepted[i]);
	}
#endif

    data->A = build_automaton(sigma_size, qw);
    free(qw);

    data->best_score = 0;
    data->best_start = 0;
    data->best_stop = 0;

    data->q_best_score = malloc(sizeof(int)*data->num_queries);
    data->q_best_start = malloc(sizeof(int)*data->num_queries);
    data->q_best_stop = malloc(sizeof(int)*data->num_queries);
    data->q_best_hits = malloc(sizeof(int)*data->num_queries);

    for (i=0; i<data->num_queries; i++)
	{
	    data->q_best_score[i] = 0;
	    data->q_best_start[i] = 0;
	    data->q_best_stop[i] = 0;
	}

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
    *output_text = print_best_dual_snippet(data, b_start, b_end);

    bsgp_delete_buffer( bs, scanner );
    bsgplex_destroy( scanner );

    free_automaton(data->A);
    destroy(data->VHit);
    destroy(data->VMatch);
    destroy(data->Q2);
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

