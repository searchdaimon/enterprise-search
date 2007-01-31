
#include "highlight.parser.c"
#include <assert.h>

// --- fra flex:
typedef void* yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE highlight_scan_bytes (const char *bytes,int len ,yyscan_t yyscanner );
// ---

int buf_printf(struct parser_data *pd, const char *fmt, ...)
{
    va_list	ap;

    va_start(ap, fmt);

    int	size = MAX_BUFFER_SIZE - pd->cur_rpos - 1;
    int	len_printed = vsnprintf(&(pd->buf[pd->cur_rpos]), size, fmt, ap);

    if (len_printed > size)
	{
	    // We should copy the end of the buffer to beginning of new buffer:

	    int		diff = pd->match_cur->start_rpos;

	    pd->cur_rpos-= diff;
	    pd->cur_stop_rpos-= diff;
	    memmove( pd->buf, &(pd->buf[diff]), pd->cur_rpos );

	    pd->cur_rpos--;	// Fix: Det dukket opp en mystisk '\0' her som ødela.

	    int		i;
	    for (i=0; i<POS_DATA_SIZE; i++)
		{
		    pd->pos_data[i].start_rpos-= diff;
		    pd->pos_data[i].stop_rpos-= diff;
		}

	    pd->match_cur->start_rpos-= diff;
	    pd->match_best->start_rpos-= diff;

	    // Write the text here, at the beginning of the new buffer:

	    va_end(ap);
	    va_start(ap, fmt);
	    size = MAX_BUFFER_SIZE - pd->cur_rpos - 1;
	    len_printed = vsnprintf(&(pd->buf[pd->cur_rpos]), size, fmt, ap);
	}

    pd->cur_rpos+= len_printed;

    va_end(ap);

    return len_printed;
}


void generate_highlighting( query_array qa, char text[], int text_size, char **output_text )
{
    int		i, j;

    // Variables for lexical analyzer:
    struct _hp_yy_extra	*he = (struct _hp_yy_extra*)malloc(sizeof(struct _hp_yy_extra));

    he->stringtop = -1;

    // Variables for parser:

    struct parser_data	*pd = (struct parser_data*)malloc(sizeof(struct parser_data));

    pd->siste_tegn = ' ';

    pd->is_div_start = FALSE;
    pd->is_head_start = FALSE;
    pd->is_span_start = FALSE;

    pd->snippet[0] = '\0';

    pd->frase_bygger = (int*)malloc(sizeof(int[qa.n]));
    for (i=0; i<qa.n; i++)
	pd->frase_bygger[i] = 0;

    for (i=0; i<POS_DATA_SIZE; i++)
	{
    	    pd->pos_data[i].match_start = (int*)malloc(sizeof(int[qa.n]));
	    pd->pos_data[i].match_end = (int*)malloc(sizeof(int[qa.n]));
	}
    pd->pos_data_ptr = -1;
    pd->pos_data_snippet_start = 0;
    for (i=0; i<qa.n; i++)
	{
	    pd->pos_data[0].match_start[i] = 0;
	    pd->pos_data[0].match_end[i] = 0;
	}
    pd->pos_data[0].start_rpos = 0;
    pd->pos_data[0].stop_rpos = 0;
    pd->scheduled_print = FALSE;
    pd->cur_stop_rpos = 0;

    pd->match_cur = (struct match_data_struct*)malloc(sizeof(struct match_data_struct));
    pd->match_cur->matches = (int*)malloc(sizeof(int[qa.n]));
    pd->match_best = (struct match_data_struct*)malloc(sizeof(struct match_data_struct));
    pd->match_best->matches = (int*)malloc(sizeof(int[qa.n]));

    pd->match_best->balance = 0;
    pd->match_best->sum = 0;
    pd->match_best->start_rpos = 0;
    pd->match_best->num_words = 0;

    pd->match_cur->balance = 0;
    pd->match_cur->sum = 0;
    pd->match_cur->start_rpos = 0;
    pd->match_cur->num_words = 0;

    for (i=0; i<qa.n; i++)
	{
	    pd->match_best->matches[i] = 0;
	    pd->match_cur->matches[i] = 0;
	}

    pd->match_best->begins_with_div = FALSE;
    pd->match_best->begins_with_head = FALSE;
    pd->match_best->begins_with_span = FALSE;
    pd->match_best->begins_sentence = FALSE;

//    pd->qa = qa;
    copy_htmlescaped_query( &(pd->qa), &qa );
    pd->cur_rpos = 0;
    pd->buf_pos = 0;


    // Initialize:
    void	*pParser = highlightParseAlloc(malloc);
    int		zv;

    // Run parser:
    yyscan_t	scanner;

    highlightlex_init( &scanner );
    highlightset_extra( he, scanner );
    YY_BUFFER_STATE	bs = highlight_scan_bytes( text, text_size, scanner );

    he->token.space = 0;
    while ((zv = highlightlex(scanner)) != 0)
	{
	    highlightParse(pParser, zv, he->token, pd);
	    he->token.space = 0;
	}

    highlightParse(pParser, 0, he->token, pd);
    highlightParseFree(pParser, free);

    highlight_delete_buffer( bs, scanner );
    highlightlex_destroy( scanner );


    // Copy snippet to output-buffer.
    int		output_size = strlen(pd->snippet);

    (*output_text) = (char*)malloc( output_size +1 );

    strcpy( *output_text, pd->snippet );
    (*output_text)[output_size] = '\0';


    free( pd->match_best->matches );
    free( pd->match_cur->matches );
    free( pd->match_best );
    free( pd->match_cur );
    for (i=0; i<POS_DATA_SIZE; i++)
	{
	    free( pd->pos_data[i].match_start );
	    free( pd->pos_data[i].match_end );
	}
    free( pd->frase_bygger );

    destroy_query( &(pd->qa) );

    free( pd );

    free( he );
}
