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

#ifdef DEBUG
#define DEBUG_ON
#endif

#ifdef DEBUG2
#ifndef DEBUG_ON
#define DEBUG_ON
#endif
#endif


struct score_block
{
    int		score, start, stop, hits;
};


struct bsg_intern_data
{
    int		bsize, bpos;
    char	*buf;
    int		wordnr;
    char	in_link;
    container	*Q, *Q2, *VMatch, *VSection, *VHit, *VWordNr, *VLink, *WSentence;
    int		VSection_start;
    automaton	*A;
    int		*q_dep;
    char	*q_stop;
    int		last_match, q_start;
    int		VMatch_start, VMatch_start2;
    int		q_flags;
    struct score_block	best, *q_best;
    int		snippet_size;
    int		tilstand;
    int		**d;
    int		*accepted;
    int		*history;
    int		history_crnt, history_size;
    int		*phrase_sizes;
    int		num_queries;
    int		words_ordinary, words_other;
    int		good_sentence;
    char	parse_error;
    int		div_num;
#ifdef DEBUG_ON
    char	sbuf[2048];
    int		spos;
    char	last_top;
    int		last_score;
#endif
};

const int	show = 0;
const int	v_section_div=1, v_section_head=2, v_section_span=4, v_section_sentence=8;

static inline char ordinary( char *s );
static inline char sentence( struct bsg_intern_data *data );
static inline int buf_printf(struct bsg_intern_data *data, const char *fmt, ...);
static inline void test_for_snippet(struct bsg_intern_data *data, char forced);

%}

%pure-parser
%parse-param { struct bsg_intern_data *data }
%parse-param { yyscan_t yyscanner }
%lex-param { yyscan_t yyscanner }
%token DIV_START DIV_END HEAD_START HEAD_END SPAN_START SPAN_END LINK_START LINK_END WORD EOS PARANTES OTHER

%%
doc	:
	| doc div
	;
div	: div_start paragraph DIV_END
	    {
		if (show) buf_printf(data, "\033[1;33m}\033[0m");

//		printf("%i good sentences.\n", data->good_sentence);
//		printf("-----\n");
/*
		char	good = 0;
		if (data->words_other > 0 && data->words_ordinary > 10)
		    {
			if (data->words_ordinary / data->words_other >= 3) good = 1;
		    }
		printf("DIV scored: %i / %i => %s.\n", data->words_ordinary, data->words_other, (good ? "GOOD" : "bad"));
*/
/*
		int		i;

		if (data->good_sentence >= 2)
		    {
			data->best.score|= (1<<19);
			for (i=0; i<data->num_queries; i++)
			    data->q_best[i].score|= (1<<19);
		    }

		if (data->old.score > data->best.score)
		    data->best = data->old;
		for (i=0; i<data->num_queries; i++)
		    if (data->old_q[i].score > data->q_best[i].score)
			data->q_best[i] = data->old_q[i];

		data->old = data->best;
		for (i=0; i<data->num_queries; i++)
		    data->old_q[i] = data->q_best[i];

		data->best.score&= (0x7fffffff - (1<<19));
		for (i=0; i<data->num_queries; i++)
		    data->q_best[i].score&= (0x7fffffff - (1<<19));
*/
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

		if (sentence(data)) data->good_sentence++;
	    }
	;
sentence :
	| sentence WORD
	{
    	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }

	    if (data->in_link==0)
		{
		    if (ordinary((char*)$2))
			data->words_ordinary++;
		    else
			data->words_other++;
		}

	    vector_pushback(data->WSentence, data->bpos, data->good_sentence);

	    queue_push(data->Q, data->bpos, data->q_flags);
	    queue_push(data->Q2, data->bpos, data->q_flags);
	    data->q_flags = 0;

	    int	ret;

	    if (data->num_queries>0) ret = search_automaton( data->A, (char*)$2 );
	    else ret = -1;

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
				    vector_pushback(data->VWordNr, data->wordnr);
				    vector_pushback(data->VLink, data->in_link);
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
	    data->wordnr++;
	}
	| sentence EOS
	{
    	    data->q_flags|= v_section_sentence;

	    if (bsgpget_extra(yyscanner)->space) { buf_printf(data, " "); bsgpget_extra(yyscanner)->space = 0; }
	    buf_printf(data, "%s", (char*)$2);

	    test_for_snippet(data,0);

	    if (sentence(data)) data->good_sentence++;
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

	    if (sentence(data)) data->good_sentence++;
	}
	| sentence LINK_START
	{
	    data->in_link = 1;
	}
	| sentence LINK_END
	{
	    data->in_link = 0;
	}
	;
%%


static inline char ordinary( char *s )
{
    int		i;

    for (i=0; s[i]!='\0'; i++)
        if (s[i]<'a')
	    return 0;

    return 1;
}


static inline char sentence( struct bsg_intern_data *data )
{
    if (data->words_ordinary >= 6 && (data->words_ordinary + data->words_other >= 8)
	&& data->words_other >= 1 && ((float)data->words_other / (float)data->words_ordinary <= 0.34))
	    {
	        data->words_ordinary = 0;
		data->words_other = 0;
    		return 1;
	    }

    data->words_ordinary = 0;
    data->words_other = 0;
    return 0;
}


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


static inline void calculate_snippet(struct bsg_intern_data *data, char forced, int type, container *Q, int *VMatch_start, int maxsize, char verbose)
{
    while (queue_size(Q) > 0 && (((data->bpos - pair(queue_peak(Q)).first.i) > maxsize) || forced))
	{
	    value	temp = queue_peak(Q);
	    int		qfirst, qsize;
	    int 	pos, flags;
	    value	phrase;
	    int		i, j;
	    char	m;
	    int		d_hits;
	    int		score=0;
	    int		hits_in_links;
	    int		sentences;
#ifdef DEBUG_ON
	    int		_spos = 0;
	    char	_sbuf[2048];
#endif

	    pos = pair(temp).first.i;
	    flags = pair(temp).second.i;

#ifdef DEBUG_ON
	    if (verbose)
		{
		    _spos+= sprintf(_sbuf+_spos, "%2i ", data->good_sentence);

		    if (forced)
			_spos+= sprintf(_sbuf+_spos, "frc ");
		    else
			_spos+= sprintf(_sbuf+_spos, "%i ", (data->bpos - pair(queue_peak(Q)).first.i) );

		    if (flags & v_section_div)
			_spos+= sprintf(_sbuf+_spos, "D");
		    else
			_spos+= sprintf(_sbuf+_spos, "_");

		    if (flags & v_section_head)
			_spos+= sprintf(_sbuf+_spos, "H");
		    else
			_spos+= sprintf(_sbuf+_spos, "_");

		    if (flags & v_section_span)
			_spos+= sprintf(_sbuf+_spos, "S");
		    else
			_spos+= sprintf(_sbuf+_spos, "_");

		    if (flags & v_section_sentence)
			_spos+= sprintf(_sbuf+_spos, "s");
		    else
			_spos+= sprintf(_sbuf+_spos, "_");
		}
#endif

	    qfirst = (*VMatch_start);
	    qsize = vector_size(data->VMatch);

//	    for (i=(*VMatch_start); i<vector_size(data->VMatch)
	    for (i=qfirst; i<qsize
		&& pair(vector_get(data->VMatch,i)).first.i < pos; i++);
	    (*VMatch_start) = i;

	    m = (i < qsize);

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

	    for (k=qfirst; k<qsize; k++)
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

//	    printf("[%i] ", d_hits);

	    int		closeness = 0;

	    {
	    int		l, n;
	    int		diff[data->num_queries];
	    int		matrix[data->num_queries][data->num_queries];

	    for (l=0; l<data->num_queries; l++)
		diff[l] = -1;

	    for (l=0; l<data->num_queries; l++)
		for (n=0; n<data->num_queries; n++)
		    matrix[l][n] = 10000;

	    int		lastnr = -1;
	    for (j = qfirst; j<qsize; j++)
		{
		    int		thisnr = vector_get(data->VWordNr,j).i;
		    int		thisq = vector_get(data->VHit,j).i;
		    int		z;

		    if (lastnr>=0)
		    {
		    for (z=0; z<data->num_queries; z++)
		    {
			if (diff[z] >= 0 && z!=thisq)
			    {
			    if (thisnr-diff[z] < matrix[thisq][z])
				{
				    matrix[thisq][z] = matrix[z][thisq] = thisnr-diff[z];
				}
			    }
		    }
		    }

		    lastnr = thisnr;
		    diff[thisq] = thisnr;
		}

	    hits_in_links = 0;
	    for (j = qfirst; j<qsize; j++)
		{
		    int		in_link = vector_get(data->VLink,j).i;

		    if (in_link) hits_in_links++;
		}

	    int		bucket[4];

	    for (l=0; l<4; l++)
		bucket[l] = 0;

	    for (l=0; l<data->num_queries; l++)
		for (n=l+1; n<data->num_queries; n++)
		    {
			if (matrix[l][n] == 1)
			    bucket[0]++;
			else if (matrix[l][n] == 2)
			    bucket[1]++;
			else if (matrix[l][n] <= 4)
			    bucket[2]++;
			else
			    bucket[3]++;
		    }

	    int		rest = 0;

	    for (l=0; l<4; l++)
		{
		    bucket[l]+= rest;
		    if (bucket[l]>3)
			{
			    rest = bucket[l]-3;
			    bucket[l] = 3;
			}

		    closeness+= bucket[l]<<((3-l)*2);
		}

	    sentences = 0;
	    int		start_bpos = pair(queue_peak(Q)).first.i;

	    for (l=0; l<vector_size(data->WSentence); l++)
		{
		    if (pair(vector_get(data->WSentence, l)).first.i >= start_bpos)
			break;
		}

	    sentences = pair(vector_get(data->WSentence, vector_size(data->WSentence)-1)).second.i
		- pair(vector_get(data->WSentence, l)).second.i;

#ifdef DEBUG_ON
	    if (verbose)
		{
		    _spos+= sprintf(_sbuf+_spos, "\033[1;35m");
		    _spos+= sprintf(_sbuf+_spos, " (%i) ", closeness);
		    _spos+= sprintf(_sbuf+_spos, "\033[0m");
		}
	    _spos+= sprintf(_sbuf+_spos, "[%i] ", hits_in_links);
	    _spos+= sprintf(_sbuf+_spos, ":%i: ", sentences);
#endif
	    }

    /**
        Algoritme for kalkulering av beste snippet:

        x bit (22+)		d_hits (antall unike treff)
	3 bit (19-21)		div har good sentences
        2 bit (17-18)		3 - hits_in_links (min 0)
        8 bit (9-16)		closeness (avstand mellom trefford gitt flere sokeord)
        1 bit (8)		div and !head
        1 bit (7)		div and head
        1 bit (6)		!div and head
        1 bit (5)		span
        1 bit (4)		sentence
        4 bit (0-3)		sum_hits (max 15)
     */

//	    sum_hits-= hits_in_links;
	    int		num;
	    num = 3 - hits_in_links;
	    if (num<0) num = 0;

	    // Calculate score:
	    score|= (d_hits<<22);

	    if (sentences > 7) sentences = 7;
	    score|= (sentences<<19);

	    score|= (num<<17);
	    score|= (closeness<<9);

	    if ((flags & v_section_div) && !(flags & v_section_head))
		score|= (1<<8);
	    if ((flags & v_section_div) && (flags & v_section_head))
		score|= (1<<7);
	    if (!(flags & v_section_div) && (flags & v_section_head))
		score|= (1<<6);
	    if ((flags & v_section_span))
		score|= (1<<5);
	    if ((flags & v_section_sentence))
		score|= (1<<4);

	    if (sum_hits > 15)
		score|= 0x0f;
	    else
		score|= sum_hits;

	    if (type==1)
		{
		    if (score > data->best.score)
			{
			    data->best.score = score;
			    data->best.start = pos;
			    data->best.stop = data->bpos;
			    data->best.hits = bin_hits;
			}
		}
	    else
		{
		    for (k=0; k<data->num_queries; k++)
			{
			    if (treff[k] && score > data->q_best[k].score)
				{
//				    printf("(%i)", k);
				    data->q_best[k].score = score;
				    data->q_best[k].start = pos;
				    data->q_best[k].stop = data->bpos;
				    data->q_best[k].hits = bin_hits;
				}
			}
		}

#ifdef DEBUG_ON
	    if (verbose)
		{
		    _spos+= sprintf(_sbuf+_spos, "\033[1;31m");
		    _spos+= sprintf(_sbuf+_spos, "%5i ", score);
		    _spos+= sprintf(_sbuf+_spos, "\033[0m");

		    for (;pos < data->bpos; pos++)
			{
			    if (m && pos == pair(phrase).first.i)
				{
				    _spos+= sprintf(_sbuf+_spos, "\033[1;36m\'");
				}

			    if (m && pos == pair(phrase).second.i)
				{
				    _spos+= sprintf(_sbuf+_spos, "\'\033[0m");

				    i++;
				    m = (i < vector_size(data->VMatch));

				    if (m)
					{
					    phrase = vector_get(data->VMatch,i);
					}
				}

			    _spos+= sprintf(_sbuf+_spos, "%c", data->buf[pos]);
			}
		    if (m && pos == pair(phrase).second.i)
			_spos+= sprintf(_sbuf+_spos, "\'\033[0m");
		    _spos+= sprintf(_sbuf+_spos, "\n");
		    _sbuf[_spos] = '\0';

		    if (score<=data->last_score && data->last_top) printf("%s", data->sbuf);
//		    printf("%i -> %i (%s)\n", data->last_score, score, (data->last_top ? "top" : "-"));

		    data->last_top = (score > data->last_score);
		    data->last_score = score;
		    memcpy(data->sbuf, _sbuf, 2048);
		}
#endif

	    queue_pop(Q);

	    if (forced && queue_size(Q) > 0 &&  ((data->bpos - pair(queue_peak(Q)).first.i) < (maxsize*3/4)))
		break;
	}
}



static inline void test_for_snippet(struct bsg_intern_data *data, char forced)
{
#ifdef DEBUG
    calculate_snippet(data, forced, 1, data->Q, &(data->VMatch_start), data->snippet_size, 1);
#else
    calculate_snippet(data, forced, 1, data->Q, &(data->VMatch_start), data->snippet_size, 0);
#endif

#ifdef DEBUG2
    calculate_snippet(data, forced, 2, data->Q2, &(data->VMatch_start2), data->snippet_size/2, 1);
#else
    calculate_snippet(data, forced, 2, data->Q2, &(data->VMatch_start2), data->snippet_size/2, 0);
#endif
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

    for (i=0; i<vector_size(data->VMatch) && pair(vector_get(data->VMatch,i)).first.i < data->best.start; i++);

    m = (i < vector_size(data->VMatch));

    if (m)
	{
	    phrase = vector_get(data->VMatch,i);
	}

//    bpos+= sprintf(buf+bpos, "[[%i]]", data->snippet_size);

    for (pos=data->best.start; pos<data->best.stop; pos++)
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

    if (data->bpos - data->best.stop < 5)
	{
	    snprintf(buf+bpos, bsize-bpos-1, "%s", &(data->buf[pos]));
	}
    else if (!last_was_eos && data->best.stop < data->bpos)
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
		int	bin_hits = data->q_best[i].hits | data->q_best[j].hits;

		if (bin_hits > best_hits)
		    {
			best_hits = bin_hits;

			if (data->q_best[i].start < data->q_best[j].start)
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
    if ((nr1==nr2) || (data->q_best[nr1].score == 0 || data->q_best[nr2].score == 0)
	|| (data->q_best[nr1].start == data->q_best[nr2].start)
	|| (best_hits < data->best.hits)
	|| (best_hits == data->best.hits && ((data->q_best[nr1].score | data->q_best[nr2].score) <= data->best.score)))
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

	    for (i=0; i<vector_size(data->VMatch) && pair(vector_get(data->VMatch,i)).first.i < data->q_best[nr].start; i++);

	    m = (i < vector_size(data->VMatch));

	    if (m)
		{
		    phrase = vector_get(data->VMatch,i);
		}

	    for (pos=data->q_best[nr].start; pos<data->q_best[nr].stop; pos++)
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
	    else if (data->bpos - data->q_best[nr].stop < 5)
		{
		    snprintf(buf+bpos, bsize-bpos-1, "%s", &(data->buf[pos]));
		}
	    else if (!last_was_eos && data->q_best[nr].stop < data->bpos)
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


int generate_snippet( query_array qa, char text[], int text_size, char **output_text, char* b_start, char* b_end, int _snippet_size )
{
    struct bsgp_yy_extra	*he = malloc(sizeof(struct bsgp_yy_extra));
    struct bsg_intern_data	*data = malloc(sizeof(struct bsg_intern_data));
    int				i, j, k, found, qw_size=0, num_qw, longest_phrase=0, sigma_size;
    unsigned char		**qw;
    int				*sigma;

    he->stringtop = 0;
    he->space = 0;

    data->snippet_size = _snippet_size;

#ifdef DEBUG
    data->spos = 0;
    data->last_score = 0;
    data->last_top = 0;
    data->div_num = 0;
#endif

    data->bpos = 0;
    data->bsize = 65536;
    data->buf = malloc(data->bsize);
    data->wordnr = 0;
    data->in_link = 0;
    data->Q = queue_container( pair_container( int_container(), int_container() ) );
    data->Q2 = queue_container( pair_container( int_container(), int_container() ) );
    data->WSentence = vector_container( pair_container( int_container(), int_container() ) );
    data->VMatch = vector_container( pair_container( int_container(), int_container() ) );
    data->VHit = vector_container( int_container() );
    data->VWordNr = vector_container( int_container() );
    data->VLink = vector_container( int_container() );
    data->VMatch_start = 0;
    data->VMatch_start2 = 0;
    data->q_flags = v_section_sentence;


    data->num_queries = qa.n;

    if (data->num_queries > 0)
	{
	    for (i=0; i<qa.n; i++)
		qw_size+= qa.query[i].n;

	    qw = malloc(sizeof(char*)*qw_size);
	    sigma = malloc(sizeof(int)*qw_size);
	    data->q_dep = malloc(sizeof(int)*qw_size);
	    data->q_stop = malloc(sizeof(char)*qw_size);
	    data->last_match = -1;
	    data->q_start = -1;

	    data->tilstand = 0;

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
	    free(sigma);


	    data->q_best = malloc(sizeof(struct score_block)*data->num_queries);
//	    data->old_q = malloc(sizeof(struct score_block)*data->num_queries);

	    for (i=0; i<data->num_queries; i++)
		{
		    data->q_best[i].score = 0;
		    data->q_best[i].start = 0;
		    data->q_best[i].stop = 0;
//		    data->old_q[i].score = 0;
		}
	}

    data->best.score = 0;
    data->best.start = 0;
    data->best.stop = 0;
//    data->old.score = 0;

    data->words_ordinary = 0;
    data->words_other = 0;
    data->good_sentence = 0;

    yyscan_t	scanner;
    int		yv;

    bsgplex_init( &scanner );
    bsgpset_extra( he, scanner );

    YY_BUFFER_STATE	bs = bsgp_scan_bytes( text, text_size, scanner );

    data->parse_error = 0;

    while ((yv = bsgpparse(data, scanner)) != 0 && data->parse_error==0)
	{
	}

    test_for_snippet(data,1);
    if (data->best.stop==0)
	data->best.stop = data->bpos;
    *output_text = print_best_dual_snippet(data, b_start, b_end);

    bsgp_delete_buffer( bs, scanner );
    bsgplex_destroy( scanner );

    if (data->num_queries > 0)
	{
	    free_automaton(data->A);

	    free(data->q_best);
//	    free(data->old_q);

	    free(data->q_stop);
	    free(data->q_dep);
	    free(data->accepted);
	    free(data->history);
	    free(data->phrase_sizes);

	    for (i=0; i<num_qw+1; i++)
		free(data->d[i]);
	    free(data->d);
	}

    destroy(data->VLink);
    destroy(data->VWordNr);
    destroy(data->VHit);
    destroy(data->VMatch);
    destroy(data->WSentence);
    destroy(data->Q2);
    destroy(data->Q);

    free(data->buf);

    int		success = !data->parse_error;
    free(data);
    free(he);

    return success;
}


bsgperror( struct bsg_intern_data *data, yyscan_t scanner, char *s )
{
    fprintf(stderr, "Parse error: %s\n", s);
    data->parse_error = 1;
}

