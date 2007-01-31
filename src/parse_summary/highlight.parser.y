// (C) Copyright Boitho 2006, Magnus Galåen (magnusga@idi.ntnu.no)

// BUGS: tekststreng > 1600 tegn?
// BUGS: fraselengde > MAX_SNIPPET_LEN ?

%include
{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "c_queue.h"

#include "highlight.h"
#include "highlight.parser.common.h"
#include "highlight.parser.h"

#define BOOL	char

#define FALSE	0
#define TRUE	1

//#define BOLD_START	"&nbsp;&lt;b&gt;"
//#define BOLD_STOP	"&lt;/b&gt;"

#define BOLD_START	"<b>"
#define BOLD_STOP	"</b>"

#define MAX_BUFFER_SIZE	16384
#define MAX_SNIPPET_LEN	160
#define POS_DATA_SIZE	100


// Braheten til en posisjon (til bruk i dynamisk programmering):
struct pos_data_struct
{
    int		*match_start, *match_end;		// Array med antall påbegynte og avsluttende treff.
    int		b_start, b_end;
    int		start_rpos, stop_rpos;
    BOOL	is_div_start, is_head_start, is_span_start, is_sentence_start;
	// - Antall trefford hittil i dokumentet [33,4,0,89]
	// - Antall påbegynte treffraser _før_denne_pos_ i dokumentet [2,1,3]
	// - Antall avsluttede treffraser _med_denne_pos_ i dokumentet [2,0,3]
	// - Er dette en div-start?
	// - Er dette en head-start?
	// - Er dette en span-start?
	// - Er treffet i en head/denne head-en?
};

struct match_data_struct
{
    int		*matches;
    int		balance, sum;
    int		start_rpos;
    int		num_words;
//    BOOL	has_first_hit, has_hit_in_first_part;
    BOOL	begins_with_div, begins_with_head, begins_with_span, begins_sentence; //, ends_sentence;
};

struct parser_data
{
    BOOL	is_div_start, is_head_start, is_span_start;

    struct pos_data_struct	pos_data[POS_DATA_SIZE];
    int		pos_data_snippet_start, pos_data_ptr;
    struct match_data_struct	*match_best, *match_cur;
    BOOL	scheduled_print;
    int		cur_stop_rpos;

    int		*frase_bygger;

    char	siste_tegn;
    int		cur_rpos;
    query_array	qa;
    int		buf_pos;
    char	buf[MAX_BUFFER_SIZE];
    char	snippet[MAX_SNIPPET_LEN * 10];
};


// Flytt til neste posisjon i 'pos_data' (sirkulær array).
void pos_data_next_ptr( struct parser_data *pd )
{
    int		i;
    int		old_ptr = pd->pos_data_ptr;

    pd->pos_data_ptr++;

    if (pd->pos_data_ptr >= POS_DATA_SIZE)
	pd->pos_data_ptr = 0;

    for (i=0; i<pd->qa.n; i++)
	{
	    pd->pos_data[pd->pos_data_ptr].match_start[i] = pd->pos_data[old_ptr].match_start[i];
	    pd->pos_data[pd->pos_data_ptr].match_end[i] = pd->pos_data[old_ptr].match_end[i];
	}
}

// Returner relativ posisjon i 'pos_data' (sirkulær array).
int pos_data_get_rel_ptr( struct parser_data *pd, int rel_pos )
{
    int		i = pd->pos_data_ptr;

    i+= rel_pos;

    while (i < 0) i+= POS_DATA_SIZE;
    while (i >= POS_DATA_SIZE) i-= POS_DATA_SIZE;

    return i;
}

// Save current snippet.
void save_snippet( struct parser_data *pd )
{
    int		i;

//	if (pd->scheduled_print)
	    {
		pd->scheduled_print = FALSE;

		int	dest_pos = 0, size, matches=0, bstart=0, written_matches=0;

		// For å hindre at invalide b-tagger blir skrevet ut (f.eks: "bla</b> bla <b>bla bla</b> bla <b>bla").
        	for (i=pd->pos_data_snippet_start; i<0; i++)
		    {
			if (pd->pos_data[pos_data_get_rel_ptr(pd, i)].b_start)
			    bstart = 1;

			if (pd->pos_data[pos_data_get_rel_ptr(pd, i)].b_end && bstart)
			    {
				bstart = 0;
				matches++;
			    }
		    }
		bstart = 0;

        	for (i=pd->pos_data_snippet_start; i<0; i++)
		    {
			int	_start_rpos = pd->pos_data[pos_data_get_rel_ptr(pd, i)].start_rpos,
				_stop_rpos = pd->pos_data[pos_data_get_rel_ptr(pd, i)].stop_rpos;

			if (pd->pos_data[pos_data_get_rel_ptr(pd, i)].b_start && written_matches<matches)
			    {
				strcpy( &(pd->snippet[dest_pos]), BOLD_START );
				dest_pos+= strlen(BOLD_START);

				bstart = 1;
//				printf(BOLD_START);
			    }

			size = _stop_rpos - _start_rpos;
			memcpy( &(pd->snippet[dest_pos]), &(pd->buf[_start_rpos]), size );
			dest_pos+= size;
//			for (j=_start_rpos; j<_stop_rpos; j++)
//			    printf("%c", pd->buf[j]);

			if (pd->pos_data[pos_data_get_rel_ptr(pd, i)].b_end && bstart==1)
			    {
				strcpy( &(pd->snippet[dest_pos]), BOLD_STOP );
				dest_pos+= strlen(BOLD_STOP);

				written_matches++;
//				printf(BOLD_STOP);
			    }

			if (i<-1)
			    {
				size = pd->pos_data[pos_data_get_rel_ptr(pd, i+1)].start_rpos - _stop_rpos;
				memcpy( &(pd->snippet[dest_pos]), &(pd->buf[_stop_rpos]), size );
				dest_pos+= size;
//				for (j=_stop_rpos; j<pd->pos_data[pos_data_get_rel_ptr(pd, i+1)].start_rpos; j++)
//				    printf("%c", pd->buf[j]);
			    }
			else
			    {
				int		j;
				j = _stop_rpos;

				if (pd->buf[j] == '.')
				    {
					for (j=_stop_rpos; j<pd->cur_stop_rpos && pd->buf[j]=='.'; j++)
					    pd->snippet[dest_pos++] = '.';
				    }
				else if (pd->buf[j] == ',')
				    {
					strcpy( &(pd->snippet[dest_pos]), ", ..." );
					dest_pos+= strlen(", ...");
				    }
				else
				    {
					strcpy( &(pd->snippet[dest_pos]), " ..." );
					dest_pos+= strlen(" ...");
				    }
			    }
		    }

//		printf("\n");
		pd->snippet[dest_pos] = '\0';

//#define VERBOSE 1
#ifdef VERBOSE
		printf(":%s\n", pd->snippet );
#endif	// VERBOSE
	    }
}

// Sjekk om denne snippeten er bedre enn den tidligere beste.
void check_if_this_is_the_best_snippet( struct parser_data *pd )
{
    int		i, j;

    // Lag pd->match_cur:
    pd->match_cur->balance = 0;
    pd->match_cur->sum = 0;
    pd->match_cur->num_words = - pd->pos_data_snippet_start + 1;
    pd->match_cur->begins_with_div = pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].is_div_start;
    pd->match_cur->begins_with_head = pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].is_head_start;
    pd->match_cur->begins_with_span = pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].is_span_start;
    pd->match_cur->begins_sentence = pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].is_sentence_start;
    int	more_in_front = 0;

    // Preferanse:	head | første treff | div | treff i første halvdel | span | setning | (TODO: avslutter setning)

    for (i=0; i<pd->qa.n; i++)
        {
	    {
		int	a = pd->pos_data_ptr;
		int	b = pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start);

//		printf("%i %i\n", a, b);

    	        j = pd->pos_data[a].match_end[i];
		j-= pd->pos_data[b].match_start[i];

	    }
//	    j = pd->pos_data[pd->pos_data_ptr].match_end[i] - pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].match_start[i];

	    if (j < 0) j = 0;

	    if (j>0) pd->match_cur->balance++;
	    pd->match_cur->sum+= j;
	    pd->match_cur->matches[i] = j;
	    if (more_in_front == 0 && j > pd->match_best->matches[i]) more_in_front = 1;
	    if (more_in_front == 0 && j < pd->match_best->matches[i]) more_in_front = -1;
        }

    BOOL	better_match = FALSE;

//    printf("_{%i}_", (pd->match_cur->start_rpos == pd->match_best->start_rpos && pd->match_cur->num_words > pd->match_best->num_words) );

    if ( (pd->match_cur->balance > pd->match_best->balance)
        || (pd->match_cur->balance == pd->match_best->balance && pd->match_cur->sum > pd->match_best->sum)
        || (pd->match_cur->balance == pd->match_best->balance && pd->match_cur->sum == pd->match_best->sum && more_in_front==1)
        || (pd->match_cur->start_rpos == pd->match_best->start_rpos && pd->match_cur->num_words > pd->match_best->num_words)
        )
        better_match = TRUE;

    if (pd->match_cur->balance == pd->match_best->balance && pd->match_cur->sum == pd->match_best->sum)
//	&& pd->match_cur->balance > 1)	// Minst ett treff!
        {
    	    if (pd->match_cur->begins_with_head > pd->match_best->begins_with_head)
	        better_match = TRUE;

    	    if (pd->match_cur->begins_with_head == pd->match_best->begins_with_head)
	        {
        	    if (pd->match_cur->begins_with_div > pd->match_best->begins_with_div)
		        better_match = TRUE;

    		    if (pd->match_cur->begins_with_div == pd->match_best->begins_with_div)
		        {
	    		    if (pd->match_cur->begins_with_span > pd->match_best->begins_with_span)
			        better_match = TRUE;

    			    if (pd->match_cur->begins_with_span == pd->match_best->begins_with_span)
			        {
		    		    if (pd->match_cur->begins_sentence > pd->match_best->begins_sentence)
				        better_match = TRUE;
			        }
			}
		}
	}

    if (better_match)
        {
	    // Swap pointers to new best snippet:
	    struct match_data_struct	*old = pd->match_best;
	    pd->match_best = pd->match_cur;
	    pd->match_cur = old;

	    pd->scheduled_print = TRUE;
	}


//#define VERBOSE_2 1
#ifdef VERBOSE_2
//	if (pd->scheduled_print) {

	printf("[");
	for (i=0; i<pd->qa.n; i++)
	    {
		printf("%i", pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].match_start[i]);
		if (i < pd->qa.n -1)
		    printf(",");
	    }
	printf("]");

	printf("[");
	for (i=0; i<pd->qa.n; i++)
	    {
		printf("%i", pd->pos_data[pd->pos_data_ptr].match_end[i]);
		if (i < pd->qa.n -1)
		    printf(",");
	    }
	printf("]");

/***/

	printf(" = [");
	for (i=0; i<pd->qa.n; i++)
	    {
		if (better_match == TRUE)
		    printf("%i", pd->match_best->matches[i]);
		else
		    printf("%i", pd->match_cur->matches[i]);

		if (i < pd->qa.n -1)
		    printf(",");
	    }
	printf("]");
/***/
	printf(" ");

    if (pd->scheduled_print)
	{
	    printf("( %3i:_", pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start) );
	    printf("[%i,%i,%i,%i](%i) )", pd->match_best->begins_with_div, pd->match_best->begins_with_head, pd->match_best->begins_with_span, pd->match_best->begins_sentence, pd->match_best->num_words );
	    printf(" ( %3i: ", pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start) );
	    printf("[%i,%i,%i,%i](%i) ):\n", pd->match_cur->begins_with_div, pd->match_cur->begins_with_head, pd->match_cur->begins_with_span, pd->match_cur->begins_sentence, pd->match_cur->num_words );
        }
    else
	{
	    printf("( %3i:_", pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start) );
	    printf("[%i,%i,%i,%i](%i) )", pd->match_cur->begins_with_div, pd->match_cur->begins_with_head, pd->match_cur->begins_with_span, pd->match_cur->begins_sentence, pd->match_cur->num_words );
	    printf(" ( %3i: ", pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start) );
	    printf("[%i,%i,%i,%i](%i) ):\n", pd->match_best->begins_with_div, pd->match_best->begins_with_head, pd->match_best->begins_with_span, pd->match_best->begins_sentence, pd->match_best->num_words );
	    pd->scheduled_print = TRUE;
        }
//	}
#endif	// VERBOSE_2
}


int buf_printf(struct parser_data *pd, const char *fmt, ...);

} //%include

%extra_argument {struct parser_data *pd}

%token_type {Token}
%default_type {Token}

%name highlightParse

%parse_failure
{
    fprintf(stderr, "\nhighlight.y: Parse Failure.\n");
    exit(-1);
}

%stack_overflow
{
    fprintf(stderr, "highlight.y: Stack Overflow.\n");
}


main ::= doc.
doc ::= .
/*
doc ::= DIV_START(W).
doc ::= DIV_END(W).
doc ::= HEAD_START(W).
doc ::= HEAD_END(W).
doc ::= SPAN_START(W).
doc ::= SPAN_END(W).
doc ::= WORD(W).
doc ::= EOS(W).
doc ::= OTHER(W).
doc ::= SPACE(W).
*/
doc ::= doc paragraph.

div_start ::= DIV_START.
    {
	pd->is_div_start = TRUE;
    }

paragraph ::= div_start text DIV_END.
    {
	if (pd->siste_tegn != '.')
	    {
		buf_printf(pd, ".");
		pd->siste_tegn = '.';
	    }

	pd->cur_stop_rpos = pd->cur_rpos;
    }

text ::= .
text ::= text header.
text ::= text span.

head_start ::= HEAD_START.
    {
	pd->is_head_start = TRUE;
    }

header ::= head_start series_of_span HEAD_END.
    {
	if (pd->siste_tegn != '.')
	    {
		buf_printf(pd, ".");
		pd->siste_tegn = '.';
	    }

	pd->cur_stop_rpos = pd->cur_rpos;
    }

series_of_span ::= .
series_of_span ::= series_of_span span.

span_start ::= SPAN_START.
    {
	pd->is_span_start = TRUE;
    }

span ::= span_start sentence SPAN_END.
    {
	if (pd->siste_tegn != '.')
	    {
		buf_printf(pd, ".");
		pd->siste_tegn = '.';
	    }

	pd->cur_stop_rpos = pd->cur_rpos;
    }

sentence ::= .
/*
sentence ::= sentence WORD(W).
    {
	// Sjekk om W er en match.

	// Dynamisk Programmering: Hvert ord skal være en struct som beskriver "braheten":
	// - Antall trefford hittil i dokumentet [33,4,0,89]
	// - Antall påbegynte treffraser _før_denne_pos_ i dokumentet [2,1,3]
	// - Antall avsluttede treffraser _med_denne_pos_ i dokumentet [2,0,3]
	// - Er dette en div-start?
	// - Er dette en head-start?
	// - Er dette en span-start?
	// - Er treffet i en head/denne head-en?
    }
*/
sentence ::= sentence WORD(W).
    {
//	printf ("%s\n", W);
	assert( strlen(W.str) < MAX_SNIPPET_LEN/2 );
	// Dersom vi får sinnsykt lange ord, kan det skape problemer.

	int	i, j;

	if (pd->pos_data_ptr >= 0)
	    {
		if (pd->scheduled_print)
		    {
			int	old_pos_data_ptr = pd->pos_data_ptr;

			pd->pos_data_ptr = pos_data_get_rel_ptr(pd, +1);
			pd->pos_data_snippet_start--;

			save_snippet( pd );

			pd->pos_data_ptr = old_pos_data_ptr;
			pd->pos_data_snippet_start++;
		    }

		int	len = strlen(W.str);
		if (W.space) len++;

		// Sjekk om snippet har blitt for stor:
		while ( pd->pos_data[pd->pos_data_ptr].stop_rpos + len
		    > pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].start_rpos + MAX_SNIPPET_LEN )
		    {
			pd->pos_data_snippet_start++;
			pd->match_cur->start_rpos = pd->pos_data[pos_data_get_rel_ptr(pd, pd->pos_data_snippet_start)].start_rpos;

//			int	old_pos_data_ptr = pd->pos_data_ptr;
//			pd->pos_data_ptr = pos_data_get_rel_ptr(pd, -1);

			if (pd->scheduled_print)
			    {
//			printf("a ");
//			save_snippet(pd);

				pd->pos_data_snippet_start--;

//				printf("+ ");
				save_snippet(pd);

				pd->pos_data_snippet_start++;
			    }

			check_if_this_is_the_best_snippet(pd);
//			pd->pos_data_ptr = old_pos_data_ptr;
		    }
	    }

	if (pd->pos_data_ptr < 0)
	    pd->pos_data_ptr = 0;
	else
	    {
		pos_data_next_ptr( pd );	// Snirkulær.
		pd->pos_data_snippet_start--;
	    }


	pd->pos_data[pd->pos_data_ptr].b_start = FALSE;
	pd->pos_data[pd->pos_data_ptr].b_end = FALSE;

	pd->pos_data[pd->pos_data_ptr].is_div_start = pd->is_div_start;
	pd->pos_data[pd->pos_data_ptr].is_head_start = pd->is_head_start;
	pd->pos_data[pd->pos_data_ptr].is_span_start = pd->is_span_start;

	if (pd->siste_tegn!='a')
	    pd->pos_data[pd->pos_data_ptr].is_sentence_start = TRUE;
	else
	    pd->pos_data[pd->pos_data_ptr].is_sentence_start = FALSE;

	pd->is_div_start = FALSE;
	pd->is_head_start = FALSE;
	pd->is_span_start = FALSE;
// /*DEBUG:*/ printf(" --- (%i,%i,%i) ---\n", pd->pos_data[pd->pos_data_ptr].is_div_start, pd->pos_data[pd->pos_data_ptr].is_head_start, pd->pos_data[pd->pos_data_ptr].is_span_start );

	// Legg til dette ordet:
	if (W.space && pd->cur_rpos>0) buf_printf(pd, " ");

	pd->pos_data[pd->pos_data_ptr].start_rpos = pd->cur_rpos;

	buf_printf(pd, "%s", W.str);

	pd->pos_data[pd->pos_data_ptr].stop_rpos = pd->cur_rpos;
	pd->cur_stop_rpos = pd->cur_rpos;


	// Sjekk om W er en match:
	for (i=0; i<pd->qa.n; i++)
	    // Sjekk for treff på frase:
	    if (pd->qa.query[i].n > 1)
		{
		    if (!strcasecmp(pd->qa.query[i].s[pd->frase_bygger[i]], W.str))
			{
			    pd->frase_bygger[i]++;
			    if (pd->frase_bygger[i]==pd->qa.query[i].n)
				{
				    pd->frase_bygger[i] = 0;

				    // Merk: pos_data for første ordet i frasen vil _ikke_ bli merket.
				    for (j=1; j< pd->qa.query[i].n -1; j++)
					pd->pos_data[ pos_data_get_rel_ptr( pd, -j ) ].match_start[i]++;

				    pd->pos_data[ pos_data_get_rel_ptr( pd, - pd->qa.query[i].n+1 ) ].b_start = TRUE;

				    pd->pos_data[pd->pos_data_ptr].match_start[i]++;
				    pd->pos_data[pd->pos_data_ptr].match_end[i]++;
				    pd->pos_data[pd->pos_data_ptr].b_end = TRUE;
				}
			}
		    else
			{
			    pd->frase_bygger[i] = 0;
			}
		}
	    // Treff på enkeltord:
	    else if (!strcasecmp(pd->qa.query[i].s[0], W.str))
		{
		    pd->pos_data[pd->pos_data_ptr].match_start[i]++;
		    pd->pos_data[pd->pos_data_ptr].match_end[i]++;

		    pd->pos_data[pd->pos_data_ptr].b_start = TRUE;
		    pd->pos_data[pd->pos_data_ptr].b_end = TRUE;
		}

	// Sjekk om denne snippeten er bedre enn forrige valgte:
        check_if_this_is_the_best_snippet(pd);

 	pd->siste_tegn = 'a';
    }
sentence ::= sentence OTHER(W).
    {
	if (W.space) buf_printf(pd, " ");
	buf_printf(pd, "%s", W.str);

	pd->siste_tegn = '?';
    }
sentence ::= sentence EOS(W).
    {
	if (W.space) buf_printf(pd, " ");
	buf_printf(pd, "%s", W.str);

	if (pd->siste_tegn=='a' || pd->siste_tegn=='.')
	    pd->cur_stop_rpos = pd->cur_rpos;

	pd->siste_tegn = '.';
    }


/*
doc ::= DIV_START(W).	{ printf("D (%s)\n", W.str); fflush(stdout); }
doc ::= DIV_END(W).	{ printf("d (%s)\n", W.str); fflush(stdout); }
doc ::= HEAD_START(W).	{ printf("H (%s)\n", W.str); fflush(stdout); }
doc ::= HEAD_END(W).	{ printf("h (%s)\n", W.str); fflush(stdout); }
doc ::= SPAN_START(W).	{ printf("S (%s)\n", W.str); fflush(stdout); }
doc ::= SPAN_END(W).	{ printf("s (%s)\n", W.str); fflush(stdout); }
doc ::= WORD(W).	{ printf("w (%s)\n", W.str); fflush(stdout); }
doc ::= EOS(W).		{ printf(". (%s)\n", W.str); fflush(stdout); }
doc ::= OTHER(W).	{ printf("? (%s)\n", W.str); fflush(stdout); }
doc ::= SPACE(W).	{ printf("_ (%s)\n", W.str); fflush(stdout); }
*/
