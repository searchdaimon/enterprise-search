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


#define STEMMING

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

struct match_block
{
    int		bstart, bend;
    int		hit;
    int		wordnr;
    int		link;
    int		div_pos, span_pos;
};

struct calc_data
{
    int		Match_start;
    int		Sstart, WSstart;
};

struct bsg_intern_data
{
    int		bsize, bpos;
    char	*buf;
    int		wordnr;
    char	in_link;
    container	*Q, *Q2;
    container	*WSentence;
    container	*Sentence;
    container	*Match;
    automaton	*A;
    int		*q_dep;
    char	*q_stop;
    int		last_match, q_start;
    struct calc_data	calc_data1, calc_data2;
    char	has_hits;
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
    int		current_div, current_span;
    int		sentence_start, sentence_stop;
#ifdef DEBUG_ON
    char	sbuf[2048];
    int		spos;
    char	last_top;
    int		last_score;
#endif
};

const int	show = 0;
const int	v_section_div=1, v_section_head=2, v_section_span=4, v_section_sentence=8, v_capital_word=16;

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
		data->current_div = data->bpos;
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
		data->current_span = data->bpos;
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

	    if (utf8_first_char_uppercase((unsigned char*)$2))
		data->q_flags|= v_capital_word;

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
			    struct match_block	*mb;

			    slot = data->history_crnt;
			    slot-= data->phrase_sizes[data->accepted[data->tilstand]];
			    while (slot<0) slot+= data->history_size;

			    // Sjekk for overlappende treff:
			    vsize = vector_size(data->Match);
			    if (vsize>0) mb = vector_get(data->Match, vsize-1).ptr;

			    if (vsize>0 && data->history[slot] <= mb->bend)
				{
				    mb->bend = data->bpos;
				    mb->hit = data->accepted[data->tilstand];
				}
			    else
				{
				    mb = malloc(sizeof(struct match_block));
				    mb->bstart = data->history[slot];
				    mb->bend = data->bpos;
				    mb->hit = data->accepted[data->tilstand];
				    mb->wordnr = data->wordnr;
				    mb->link = data->in_link;
				    mb->div_pos = data->current_div;
				    mb->span_pos = data->current_span;
				    vector_pushback(data->Match, mb);
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
    data->sentence_stop = data->bpos;

    if (data->words_ordinary >= 6 && (data->words_ordinary + data->words_other >= 8)
	&& data->words_other >= 1 && ((float)data->words_other / (float)data->words_ordinary <= 0.34))
	    {
//		printf("SENTENCE: %.100s\n", &(data->buf[data->bpos - 100]));
		vector_pushback(data->Sentence, data->sentence_start, data->sentence_stop);
	        data->sentence_start = data->bpos;
	        data->words_ordinary = 0;
		data->words_other = 0;
    		return 1;
	    }

//    printf(" %2i, %2i : %.100s\n", data->words_ordinary, data->words_other, &(data->buf[data->bpos - 100]));
    data->sentence_start = data->bpos;
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


// calculate_snippet:
static inline void calculate_snippet(struct bsg_intern_data *data, char forced, int type, container *Q, struct calc_data *calc_data, int maxsize, char verbose)
{
    // Simple form for cache:
    int		_queue_size_Q = queue_size(Q);
    value	_queue_peak_Q;

    if (_queue_size_Q > 0) _queue_peak_Q = queue_peak(Q);

    while (_queue_size_Q > 0 && (((data->bpos - pair(_queue_peak_Q).first.i) > maxsize) || forced))
	{
	    int		_vector_size_Match;
	    int 	pos, flags;
	    int		i, j;
	    char	more;
	    int		d_hits;
	    int		score=0;
	    int		bstart;
	    int		hits_in_links, hits_in_div, hits_in_span, hits_in_sentence;
	    int		sentences;
#ifdef DEBUG_ON
	    struct match_block	*mb;
	    int		_spos = 0;
	    char	_sbuf[2048];
#endif

	    pos = pair(_queue_peak_Q).first.i;
	    flags = pair(_queue_peak_Q).second.i;


	    _vector_size_Match = vector_size(data->Match);

	    for (i=calc_data->Match_start; i<_vector_size_Match; i++)
		if (((struct match_block*)vector_get(data->Match,i).ptr)->bstart >= pos) break;

	    calc_data->Match_start = i;
	    more = (i < _vector_size_Match);
	    bstart = pos;

	    if (more)
		{
#ifdef DEBUG_ON
		    mb = vector_get(data->Match,i).ptr;
#endif
		    data->has_hits = 1;
		    bstart = ((struct match_block*)vector_get(data->Match,i).ptr)->bstart;
		}
	    else if (data->has_hits)
		{
		    // Empty queue:
		    while (queue_size(Q) > 0 && ((data->bpos - pair(queue_peak(Q)).first.i) > maxsize))
			queue_pop(Q);
		    return;
		}


#ifdef DEBUG_ON
	    if (verbose)
		{
		    _spos+= sprintf(_sbuf+_spos, "%2i ", data->good_sentence);

		    if (forced)
			_spos+= sprintf(_sbuf+_spos, "frc ");
		    else
			_spos+= sprintf(_sbuf+_spos, "%i ", (data->bpos - pos) );

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

		    if (flags & v_capital_word)
			_spos+= sprintf(_sbuf+_spos, "C");
		    else
			_spos+= sprintf(_sbuf+_spos, "_");
		}
#endif


	    int		treff[data->num_queries];
	    int		diff[data->num_queries];
	    int		matrix[data->num_queries][data->num_queries];
	    int		sum_hits = 0;
	    int		k;
	    int		bin_hits = 0;

	    // Initialization of arrays:
	    for (i=0; i<data->num_queries; i++)
		{
		    treff[i] = 0;
		    diff[i] = -1;

		    for (j=0; j<data->num_queries; j++)
			matrix[i][j] = 10000;
		}

	    d_hits = 0;
	    hits_in_links = 0;
	    hits_in_div = 0;
	    hits_in_span = 0;
	    hits_in_sentence = 0;

	    // Find end of first sentence:
	    int		s_end=0, Ssize=vector_size(data->Sentence);
	    for (i=calc_data->Sstart; i<Ssize && s_end==0; i++)
		{
		    value	val = vector_get(data->Sentence,i);

		    if (pair(val).first.i <= bstart && pair(val).second.i >= bstart)
			s_end = pair(val).second.i;
		}
	    if (i>0) calc_data->Sstart = i-1;

	    // For each hit:
	    for (i=calc_data->Match_start; i<_vector_size_Match; i++)
		{
		    struct match_block	*B = vector_get(data->Match,i).ptr;

		    // Determine number of hits:
		    if ((treff[B->hit]++)==0) d_hits++;
		    sum_hits+= data->phrase_sizes[B->hit];

		    if ((B->hit)<32) bin_hits|= (1<<(B->hit));

		    // Calculate 'diff' and 'matrix':
		    if (i > calc_data->Match_start)
			{
			    for (j=0; j<data->num_queries; j++)
			        {
				    if (diff[j] >= 0 && j!=B->hit)
				        {
					    if (B->wordnr - diff[j] < matrix[B->hit][j])
						{
						    matrix[B->hit][j] = matrix[j][B->hit] = B->wordnr - diff[j];
						}
				        }
				}
			}

		    diff[B->hit] = B->wordnr;

		    // Determine various properties of the hits in the snippet:
		    if (B->link) hits_in_links++;
		    if (B->div_pos <= bstart) hits_in_div++;
		    if (B->span_pos <= bstart) hits_in_span++;
		    if (B->bstart < s_end) hits_in_sentence++;
		}


	    // Calculate 'closeness':
	    int		closeness = 0;
	    int		bucket[4];

	    for (i=0; i<4; i++)
		bucket[i] = 0;

	    for (i=0; i<data->num_queries; i++)
		for (j=i+1; j<data->num_queries; j++)
		    {
			if (matrix[i][j] == 1)
			    bucket[0]++;
			else if (matrix[i][j] == 2)
			    bucket[1]++;
			else if (matrix[i][j] <= 4)
			    bucket[2]++;
			else
			    bucket[3]++;
		    }

	    int		rest = 0;

	    for (i=0; i<4; i++)
		{
		    bucket[i]+= rest;
		    if (bucket[i]>3)
			{
			    rest = bucket[i]-3;
			    bucket[i] = 3;
			}

		    closeness+= bucket[i]<<((3-i)*2);
		}


	    // Calculate number of sentences:
	    sentences = 0;
	    int		WSsize = vector_size(data->WSentence);

	    for (i=calc_data->WSstart; i<WSsize; i++)
		{
		    if (pair(vector_get(data->WSentence, i)).first.i >= pos)
			break;
		}
	    calc_data->WSstart = i;

	    if (WSsize > 0 && i < (WSsize-1))
	        sentences = pair(vector_get(data->WSentence, WSsize-1)).second.i
		    - pair(vector_get(data->WSentence, i)).second.i;


#ifdef DEBUG_ON
	    if (verbose)
		{
		    _spos+= sprintf(_sbuf+_spos, "\033[1;35m");
		    _spos+= sprintf(_sbuf+_spos, " (%i) ", closeness);
		    _spos+= sprintf(_sbuf+_spos, "\033[0m");
		}
//	    _spos+= sprintf(_sbuf+_spos, "[%i] ", hits_in_links);
	    _spos+= sprintf(_sbuf+_spos, "(%i) ", sentences);
#endif

    /**
        Algoritme for kalkulering av beste snippet:

        x bit (23+)		d_hits (antall unike treff)
	1 bit (22)		div har good sentences
        8 bit (14-21)		closeness (avstand mellom trefford gitt flere sokeord)
	2 bit (12-13)		antall hits i bstart-div (max 3)
	1 bit (11)		hits i bstart-span
	1 bit (10)		hits i bstart-sentence
        2 bit (8-9)		div and !head | div and head | !div and head | --
        2 bit (6-7)		span | sentence
	2 bit (4-5)		antall good sentences (max 4) -1
        4 bit (0-3)		sum_hits (max 15)
     */

	    // Calculate score for this snippet:

	    if (sum_hits) sum_hits++;	// sum_hits er alltid minst 1 dersom det forekommer en hit,
					// selv om alle hits er i lenker.
	    sum_hits-= hits_in_links;	// Fjern hits i lenker.

    // TODO: closeness?

	    // Calculate score:
	    score|= (d_hits<<23);

	    if (sentences)
		score|= (1<<22);

	    score|= (closeness<<14);

	    if (hits_in_div>3) hits_in_div = 3;
	    score|= (hits_in_div<<12);
	    if (hits_in_span) score|= (1<<11);
	    if (hits_in_sentence) score|= (1<<10);

	    if ((flags & v_section_div) && !(flags & v_section_head))
		score|= (1<<9) + (1<<8);
	    if ((flags & v_section_div) && (flags & v_section_head))
		score|= (1<<9);
	    if (!(flags & v_section_div) && (flags & v_section_head))
		score|= (1<<8);

	    /**
		span+C	= 3
		span	= 2
		sntnc+C	= 2
		C	= 1
	    */
	    if ((flags & v_section_span))
		{
		    score|= (1<<7);
		    if ((flags & v_capital_word))
			score|= (1<<6);
		}
	    else if ((flags & v_section_sentence) && (flags & v_capital_word))
		score|= (1<<7);
	    else if ((flags & v_capital_word))
		score|= (1<<6);

	    if (sentences > 4) sentences = 4;
	    if (sentences > 0) sentences--;
	    score|= (sentences<<4);

	    if (sum_hits > 15)
		score|= 0x0f;
	    else
		score|= sum_hits;

	    // Save score:
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
		    _spos+= sprintf(_sbuf+_spos, "\033[1;30m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<26) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<25) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<24) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<23) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;37m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<22) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;30m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<21) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<20) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<19) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<18) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<17) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<16) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<15) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<14) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;37m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<13) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<12) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;30m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<11) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;37m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<10) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;30m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<9) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<8) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;37m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<7) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<6) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;30m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<5) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<4) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[1;37m");
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<3) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<2) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<1) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "%c", (score&(1<<0) ? '1' : '0'));
		    _spos+= sprintf(_sbuf+_spos, "\033[0m ");

		    _spos+= sprintf(_sbuf+_spos, "\033[1;31m");
		    _spos+= sprintf(_sbuf+_spos, "%7i ", score);
		    _spos+= sprintf(_sbuf+_spos, "\033[0m");

		    for (;pos < data->bpos; pos++)
			{
			    if (more && pos == mb->bstart)
				{
				    _spos+= sprintf(_sbuf+_spos, "\033[1;36m\'");
				}

			    if (more && pos == mb->bend)
				{
				    _spos+= sprintf(_sbuf+_spos, "\'\033[0m");

				    i++;
				    more = (i < vector_size(data->Match));

				    if (more)
					{
					    mb = vector_get(data->Match,i).ptr;
					}
				}

			    _spos+= sprintf(_sbuf+_spos, "%c", data->buf[pos]);
			}
		    if (more && pos == mb->bend)
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

	    _queue_size_Q = queue_size(Q);
	    if (_queue_size_Q > 0) _queue_peak_Q = queue_peak(Q);

	    if (forced && _queue_size_Q > 0 &&  ((data->bpos - pair(_queue_peak_Q).first.i) < (maxsize*3/4)))
		break;
	}
}



static inline void test_for_snippet(struct bsg_intern_data *data, char forced)
{
#ifdef DEBUG
    calculate_snippet(data, forced, 1, data->Q, &(data->calc_data1), data->snippet_size, 1);
#else
    calculate_snippet(data, forced, 1, data->Q, &(data->calc_data1), data->snippet_size, 0);
#endif

#ifdef DEBUG2
    calculate_snippet(data, forced, 2, data->Q2, &(data->calc_data2), data->snippet_size/2, 1);
#else
    calculate_snippet(data, forced, 2, data->Q2, &(data->calc_data2), data->snippet_size/2, 0);
#endif
}


// TODO: Rydde i denne funksjonen:
static inline char* print_best_snippet( struct bsg_intern_data *data, char* b_start, char* b_end )
{
    int		i, pos;
    char	more;
    int		bsize = data->snippet_size*5;
    char	buf[bsize];
    int		bpos=0;
    struct match_block	*mb;
    int		active_highl=0;
    char	last_was_eos=0;
    int		Msize = vector_size(data->Match);

    for (i=0; i<Msize; i++)
	if (((struct match_block*)(vector_get(data->Match,i).ptr))->bstart >= data->best.start) break;

    more = (i < Msize);

    if (more)
	{
	    mb = vector_get(data->Match,i).ptr;
	}

//    bpos+= sprintf(buf+bpos, "[[%i]]", data->snippet_size);

    for (pos=data->best.start; pos<data->best.stop; pos++)
	{
	    if (more && pos == mb->bstart)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_start);
		    active_highl++;
		}

	    if (more && pos == mb->bend)
		{
		    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
		    active_highl--;
		    i++;
		    more = (i < Msize);

		    if (more)
			{
			    mb = vector_get(data->Match,i).ptr;
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
	    char	more;
	    struct match_block	*mb;
	    int		active_highl=0;
	    char	last_was_eos=0;
	    int		Msize = vector_size(data->Match);

//	    bpos+= sprintf(buf+bpos, "((%i))", nr);

	    for (i=0; i<Msize; i++)
		if (((struct match_block*)(vector_get(data->Match,i).ptr))->bstart >= data->q_best[nr].start) break;

	    more = (i < Msize);

	    if (more)
		{
		    mb = vector_get(data->Match,i).ptr;
		}

	    for (pos=data->q_best[nr].start; pos<data->q_best[nr].stop; pos++)
		{
		    if (more && pos == mb->bstart)
			{
			    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_start);
			    active_highl++;
			}

		    if (more && pos == mb->bend)
			{
			    bpos+= snprintf(buf+bpos, bsize-bpos-1, b_end);
			    active_highl--;
			    i++;
			    more = (i < Msize);

			    if (more)
				{
				    mb = vector_get(data->Match,i).ptr;
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
    fprintf(stderr, "snippet.parser: generate_snippet()\n");

    struct bsgp_yy_extra	*he = malloc(sizeof(struct bsgp_yy_extra));
    struct bsg_intern_data	*data = malloc(sizeof(struct bsg_intern_data));
    int				i, j, k, m, found, qw_size=0, num_qw, longest_phrase=0, sigma_size;
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
    data->Sentence = vector_container( pair_container( int_container(), int_container() ) );
    data->Match = vector_container( ptr_container() );
    data->calc_data1.Match_start = 0;
    data->calc_data1.Sstart = 0;
    data->calc_data1.WSstart = 0;
    data->calc_data2.Match_start = 0;
    data->calc_data2.Sstart = 0;
    data->calc_data2.WSstart = 0;
    data->has_hits = 0;
    data->q_flags = v_section_sentence;


    data->num_queries = qa.n;

    if (data->num_queries > 0)
	{
	    int		phrase_nr=0;

	    for (i=0; i<qa.n; i++)
		{
		    qw_size+= qa.query[i].n;
#ifdef STEMMING
		    if (qa.query[i].alt != NULL)
			{
			    data->num_queries+= qa.query[i].alt_n;
			    for (j=0; j<qa.query[i].alt_n; j++)
				{
				    qw_size+= qa.query[i].alt[j].n;
				}
			}
#endif
		}

	    data->phrase_sizes = malloc(sizeof(int)*data->num_queries);
	    printf("data->num_queries = %i\n", data->num_queries);

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

		    printf("num_qw(1) = %i\n", phrase_nr);
		    data->phrase_sizes[phrase_nr++] = qa.query[i].n;

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

#ifdef STEMMING
		    if (qa.query[i].alt != NULL)
			{
			    if (qa.query[i].alt_n > longest_phrase)
				longest_phrase = qa.query[i].alt_n;

			    for (m=0; m<qa.query[i].alt_n; m++)
				{
				    printf("num_qw(2) = %i\n", phrase_nr);
				    data->phrase_sizes[phrase_nr++] = qa.query[i].alt[m].n;

				    for (j=0; j<qa.query[i].alt[m].n; j++)
					{
					    if (j==0)
						data->q_dep[sigma_size] = -1;
					    else
						data->q_dep[sigma_size] = sigma_size-1;

					    if (j==(qa.query[i].alt[m].n-1))
						data->q_stop[sigma_size] = 1;
					    else
						data->q_stop[sigma_size] = 0;

					    found = -1;
					    for (k=0; k<sigma_size; k++)
						if (!strcmp(qa.query[i].alt[m].s[j], (char*)qw[k]))
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
						    qw[sigma_size++] = (unsigned char*)qa.query[i].alt[m].s[j];
						}
					}
				}
			}
#endif
		}

	    int		state, num_states=1;
	    int		P[num_qw+1][longest_phrase+1];

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
	    for (i=0,num_qw=0; i<data->num_queries; i++)
		{
		    state = 0;

		    for (j=0; j<data->phrase_sizes[i]; j++)
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

    data->sentence_start = 0;
    data->sentence_stop = 0;

    data->current_div = 0;
    data->current_span = 0;

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

    int		Msize = vector_size(data->Match);
    for (i=0; i<Msize; i++)
	free(vector_get(data->Match,i).ptr);

    destroy(data->Match);
    destroy(data->Sentence);
    destroy(data->WSentence);
    destroy(data->Q2);
    destroy(data->Q);

    free(data->buf);

    int		success = !data->parse_error;
    free(data);
    free(he);

    if (!success)
	{
	    fprintf(stderr, "snippet.parser: Document error!\n");
	    fprintf(stderr, "snippet.parser: --- Content ---\n");
	    fprintf(stderr, "%s\n", text);
	    fprintf(stderr, "snippet.parser: --- End ---\n");
	}

    return success;
}


bsgperror( struct bsg_intern_data *data, yyscan_t scanner, char *s )
{
    fprintf(stderr, "snippet.parser: Parse error! %s\n", s);
    data->parse_error = 1;
}

