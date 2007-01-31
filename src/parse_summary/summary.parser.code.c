
#include "summary.parser.c"

// --- fra flex:
typedef void* yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE summary_scan_bytes (const char *bytes,int len ,yyscan_t yyscanner );
// ---

struct trans_tab
{
    char	*escape, translation;
};

struct trans_tab tt[130] = {
    {"#178",'²'},{"#179",'³'},{"#185",'¹'},{"#192",'À'},{"#193",'Á'},{"#194",'Â'},{"#195",'Ã'},{"#196",'Ä'},
    {"#197",'Å'},{"#198",'Æ'},{"#199",'Ç'},{"#200",'È'},{"#201",'É'},{"#202",'Ê'},{"#203",'Ë'},{"#204",'Ì'},
    {"#205",'Í'},{"#206",'Î'},{"#207",'Ï'},{"#208",'Ð'},{"#209",'Ñ'},{"#210",'Ò'},{"#211",'Ó'},{"#212",'Ô'},
    {"#213",'Õ'},{"#214",'Ö'},{"#216",'Ø'},{"#217",'Ù'},{"#218",'Ú'},{"#219",'Û'},{"#220",'Ü'},{"#221",'Ý'},
    {"#222",'Þ'},{"#223",'ß'},{"#224",'à'},{"#225",'á'},{"#226",'â'},{"#227",'ã'},{"#228",'ä'},{"#229",'å'},
    {"#230",'æ'},{"#231",'ç'},{"#232",'è'},{"#233",'é'},{"#234",'ê'},{"#235",'ë'},{"#236",'ì'},{"#237",'í'},
    {"#238",'î'},{"#239",'ï'},{"#240",'ð'},{"#241",'ñ'},{"#242",'ò'},{"#243",'ó'},{"#244",'ô'},{"#245",'õ'},
    {"#246",'ö'},{"#248",'ø'},{"#249",'ù'},{"#250",'ú'},{"#251",'û'},{"#252",'ü'},{"#253",'ý'},{"#254",'þ'},
    {"#255",'ÿ'},{"AElig",'Æ'},{"Aacute",'Á'},{"Acirc",'Â'},{"Agrave",'À'},{"Aring",'Å'},{"Atilde",'Ã'},{"Auml",'Ä'},
    {"Ccedil",'Ç'},{"ETH",'Ð'},{"Eacute",'É'},{"Ecirc",'Ê'},{"Egrave",'È'},{"Euml",'Ë'},{"Iacute",'Í'},{"Icirc",'Î'},
    {"Igrave",'Ì'},{"Iuml",'Ï'},{"Ntilde",'Ñ'},{"Oacute",'Ó'},{"Ocirc",'Ô'},{"Ograve",'Ò'},{"Oslash",'Ø'},{"Otilde",'Õ'},
    {"Ouml",'Ö'},{"THORN",'Þ'},{"Uacute",'Ú'},{"Ucirc",'Û'},{"Ugrave",'Ù'},{"Uuml",'Ü'},{"Yacute",'Ý'},{"aacute",'á'},
    {"acirc",'â'},{"aelig",'æ'},{"agrave",'à'},{"aring",'å'},{"atilde",'ã'},{"auml",'ä'},{"ccedil",'ç'},{"eacute",'é'},
    {"ecirc",'ê'},{"egrave",'è'},{"eth",'ð'},{"euml",'ë'},{"iacute",'í'},{"icirc",'î'},{"igrave",'ì'},{"iuml",'ï'},
    {"ntilde",'ñ'},{"oacute",'ó'},{"ocirc",'ô'},{"ograve",'ò'},{"oslash",'ø'},{"otilde",'õ'},{"ouml",'ö'},{"sup1",'¹'},
    {"sup2",'²'},{"sup3",'³'},{"szlig",'ß'},{"thorn",'þ'},{"uacute",'ú'},{"ucirc",'û'},{"ugrave",'ù'},{"uuml",'ü'},
    {"yacute",'ý'},{"yuml",'ÿ'}};


int compare(const void *a, const void *b)
{
    return strncmp( (char*)a, ((struct trans_tab*)b)->escape, strlen(((struct trans_tab*)b)->escape) );
}

// Translate escapes in string:
char* translate(char *s)
{
    char	*d = (char*)malloc(strlen(s)+1);
    int		i, j, k;
    char	replace;

    for (i=0, j=0; s[j]!='\0';)
	switch (s[j])
	    {
		case '&':
		    replace = 0;

		    if (s[j+1]!='\0')
			{
			    struct trans_tab	*code = (struct trans_tab*)bsearch(&(s[j+1]),tt,130,sizeof(struct trans_tab),compare);

			    if (code!=NULL)
				{
				    replace = 1;
				    d[i++] = code->translation;
				    j+= strlen(code->escape)+1;
				    if (s[j]==';') j++;
				}
			}

		    if (!replace)
			{
			    d[i++] = '&';
			    j++;
			}

		    break;
		default:
		    d[i++] = s[j++];
	    }

    d[i] = '\0';
    return d;
}


struct html_esc
{
    char	c, *esc;
};

struct html_esc he[65] = {
    {'²',"sup2"},{'³',"sup3"},{'¹',"sup1"},{'À',"Agrave"},{'Á',"Aacute"},{'Â',"Acirc"},{'Ã',"Atilde"},{'Ä',"Auml"},
    {'Å',"Aring"},{'Æ',"AElig"},{'Ç',"Ccedil"},{'È',"Egrave"},{'É',"Eacute"},{'Ê',"Ecirc"},{'Ë',"Euml"},{'Ì',"Igrave"},
    {'Í',"Iacute"},{'Î',"Icirc"},{'Ï',"Iuml"},{'Ð',"ETH"},{'Ñ',"Ntilde"},{'Ò',"Ograve"},{'Ó',"Oacute"},{'Ô',"Ocirc"},
    {'Õ',"Otilde"},{'Ö',"Ouml"},{'Ø',"Oslash"},{'Ù',"Ugrave"},{'Ú',"Uacute"},{'Û',"Ucirc"},{'Ü',"Uuml"},{'Ý',"Yacute"},
    {'Þ',"THORN"},{'ß',"szlig"},{'à',"agrave"},{'á',"aacute"},{'â',"acirc"},{'ã',"atilde"},{'ä',"auml"},{'å',"aring"},
    {'æ',"aelig"},{'ç',"ccedil"},{'è',"egrave"},{'é',"eacute"},{'ê',"ecirc"},{'ë',"euml"},{'ì',"igrave"},{'í',"iacute"},
    {'î',"icirc"},{'ï',"iuml"},{'ð',"eth"},{'ñ',"ntilde"},{'ò',"ograve"},{'ó',"oacute"},{'ô',"ocirc"},{'õ',"otilde"},
    {'ö',"ouml"},{'ø',"oslash"},{'ù',"ugrave"},{'ú',"uacute"},{'û',"ucirc"},{'ü',"uuml"},{'ý',"yacute"},{'þ',"thorn"},
    {'ÿ',"yuml"}};


int esc_compare(const void *a, const void *b)
{
    if (*((char*)a) < ((struct html_esc*)b)->c) return -1;
    if (*((char*)a) > ((struct html_esc*)b)->c) return +1;
    return 0;
}

void print2buffer(buffer *b, const char *fmt, ...)
{
    if (b->overflow) return;

    va_list	ap;

    va_start(ap, fmt);
    int	len_printed = vsnprintf((char*)(&(b->data[b->pos])), b->maxsize - b->pos - 1, fmt, ap);

    b->pos+= len_printed;

    if (b->pos >= b->maxsize - 1) b->overflow = 1;
}


void print_with_escapes(char *c, buffer *b)
{
    int		i;

//    if (b->pos > 0) print2buffer(b, " ");

    for (i=0; c[i]!='\0'; i++)
	{
	    if ((unsigned char)c[i]<128)
		print2buffer(b, "%c", c[i]);
	    else
		{
		    struct html_esc	*p = (struct html_esc*)
		    bsearch( (const void*)(((char*)&(c[i]))), he, 65, sizeof(struct html_esc), esc_compare);

		    if (p==NULL)
			print2buffer(b, "%c", c[i]);
		    else
			print2buffer(b, "&%s;", p->esc);
		}
	}

    free( c );
}

void print_raw(char *c, buffer *b)
{
//    if (b->pos > 0) print2buffer(b, " ");

    print2buffer(b, "%s", c);
}


buffer buffer_init( int _maxsize )
{
    buffer	b;

    b.overflow = 0;
    b.pos = 0;
    b.maxsize = _maxsize -1;	    // In case we need that trailing zero ;)
    b.data = (unsigned char*)malloc(b.maxsize +1);

    return b;
}

char* buffer_exit( buffer b )
{
    char	*output;

    output = (char*)malloc(b.pos+1);
    memcpy( output, &(b.data[0]), b.pos );
    output[b.pos] = '\0';
    free( b.data );

    return output;
}



void generate_summary( char text[], int text_size, char **output_title, char **output_body, char **output_metakeywords, char **output_metadescription )
{
    // Variables for lexical analyzer:
    struct _sp_yy_extra	*se = (struct _sp_yy_extra*)malloc(sizeof(struct _sp_yy_extra));

    se->top = -1;
    se->stringtop = -1;

    // Set variables for lalr(1)-parser:

    // Fields 'title', 'meta keywords' and 'meta description', will only keep first 10240 bytes,
    // field body will only keep up to double original textsize (should be enough for all ordinary documents).

    struct parseExtra pE;

    pE.section = INIT;

    pE.newspan = 1;
    pE.newdiv = 1;
    pE.newhead = 0;
    pE.endhead = 0;
    pE.inspan = 0;
    pE.indiv = 0;
    pE.inhead = 0;

    pE.title = buffer_init( 10240 );
    pE.body = buffer_init( text_size*2 );
    pE.metakeyw = buffer_init( 10240 );
    pE.metadesc = buffer_init( 10240 );

    void	*pParser = summaryParseAlloc(malloc);
    int		yv;

    // Run parser:
    yyscan_t	scanner;

    summarylex_init( &scanner );
    summaryset_extra( se, scanner );

    YY_BUFFER_STATE	bs = summary_scan_bytes( text, text_size, scanner );

    se->token.space = 0;
    while ((yv = summarylex(scanner)) != 0)
	{
	    summaryParse(pParser, yv, se->token, &pE);
	    if (yv==WORD || yv==ESC)
		se->token.space = 0;
	}

    summaryParse(pParser, 0, se->token, &pE);
    summaryParseFree(pParser, free);

    summary_delete_buffer( bs, scanner );
    summarylex_destroy( scanner );

    if (pE.inspan)
        print_raw( "</span>", &pE.body );
    if (pE.inhead)
        print_raw( "\n</h2>", &pE.body );
    if (pE.indiv)
        print_raw( "\n</div>", &pE.body );

//    pE.rawbody.data[pE.rawbody.pos] = '\0';

//    print_with_escapes( pE.rawbody.data, &(pE.body) );		// pr_w_esc kjører free automagisk!!

    (*output_title) = buffer_exit( pE.title );
    (*output_body) = buffer_exit( pE.body );
    (*output_metakeywords) = buffer_exit( pE.metakeyw );
    (*output_metadescription) = buffer_exit( pE.metadesc );

    free( se );
}
