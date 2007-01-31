%{
#include "y.tab.h"
#include "summary_common.h"
#include "summary.h"

#undef YY_INPUT
#define YY_INPUT(b, r, ms)	(r = custom_yyinput(b, ms))

extern yylval;

int linenr=1;
%}

character	[a-z]
legalchar	[a-z0-9_]
word		[0-9a-z'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõöøùúûüış]
allemuligetegn	[!"#$%'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîïğñòóôõö÷øùúûüış]
url		[0-9a-z&-.:/\\?=]
blank		[ \t\n]
tillatesc	(aacute|acirc|aelig|agrave|aring|atilde|auml|ccedil|eacute|ecirc|egrave|eth|euml|iacute|icirc|igrave|iuml|ntilde|oacute|ocirc|ograve|oslash|otilde|ouml|sup1|sup2|sup3|szlig|thorn|uacute|ucirc|ugrave|uuml|yacute|yuml)
tillatnumesc	(#178|#179|#185|#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255)
nestenalleesc	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
%option	nomain noyywrap
%x	TAG ENDTAG COMMENT SCRIPT STYLE SELECT TEXTAREA
%%
({allemuligetegn}|\&{nestenalleesc}|\&{nestenalleesc};|\&\#[0-9]+;)+	{ yylval = (int)newstring(yytext);	return WORD; }
\&[^;]*;		{} // Ignore other escapes.
\<{blank}*script[^\>]*\> {				BEGIN pushstate(SCRIPT); }	// Ignore characters between script-tags.
\<{blank}*style[^\>]*\> {				BEGIN pushstate(STYLE); }	// Ignore characters between style-tags.
\<{blank}*select[^\>]*\> {				BEGIN pushstate(SELECT); }	// Ignore characters between select-tags.
\<{blank}*textarea[^\>]*\> {				BEGIN pushstate(TEXTAREA); }	// Ignore characters between textarea-tags.
\<\/			{ yylval = (int)newstring(yytext);	BEGIN pushstate(ENDTAG);	return ENDTAG_START; }
\<			{ yylval = (int)newstring(yytext);	BEGIN pushstate(TAG);		return TAG_START; }
\<\!\-\-		{				BEGIN pushstate(COMMENT); }
<TAG>\/\>		{ yylval = (int)newstring(yytext);	BEGIN popstate();		return TAG_ENDTAG_STOPP; }
<TAG>\>			{ yylval = (int)newstring(yytext);	BEGIN popstate(); 		return TAG_STOPP; }
<TAG>\=			{ yylval = (int)newstring(yytext);					return EQUALS; }
<TAG>{legalchar}+	{ yylval = (int)newstring(yytext);					return ATTR; }
<ENDTAG>\>		{ yylval = (int)newstring(yytext);	BEGIN popstate();		return ENDTAG_STOPP; }
<ENDTAG>{legalchar}+	{ yylval = (int)newstring(yytext);					return ATTR; }
<TAG>\'(\\\'|[^\'])*\'	{ yylval = (int)newstring(yytext);					return TEXTFIELD; }
<TAG>\"(\\\"|[^\"])*\"	{ yylval = (int)newstring(yytext);					return TEXTFIELD; }
<COMMENT>\-\-\>		{				BEGIN popstate(); }
%<SCRIPT>\'(\\\'|[^\'])*\'	{}
%<SCRIPT>\"(\\\"|[^\"])*\"	{}
<SCRIPT>\<\/{blank}*script[^\>]*\>	{ 		BEGIN popstate(); }
<STYLE>\<\/{blank}*style[^\>]*\>	{ 		BEGIN popstate(); }
<SELECT>\<\/{blank}*select[^\>]*\>	{ 		BEGIN popstate(); }
<TEXTAREA>\<\/{blank}*textarea[^\>]*\>	{ 		BEGIN popstate(); }
<*>\n			{ linenr++; }
<*>.
%%

int     top=-1;
int	stack[128];
int     stringtop=-1;
#define maxNewString 2048
char  stringcircle[128][maxNewString +1];

int pushstate( int state )
{
    if (top>=127) return stack[top];	// overflow
    stack[++top] = state;
    return state;
}

int popstate()
{
    if (top<=0)	// underflow
	{
	    top--;
	    return INITIAL;
	}
    return stack[--top];
}

int custom_yyinput( char *buf, int max_size )
{
    int		size = custom_size - custom_pos;

    if (max_size < size) size = max_size;

    memcpy( buf, &(custom_input[custom_pos]), size );
    custom_pos+= size;

    return size;
}

int newstring( const char* c )
{
    int         i;

//Runar
// Bruker statisk minne, ikke dynamisk, da vi en skjelden gang får segfeil i free
// desuten er det raskere, ved at vi ikke gjør systemkallet malloc heletiden

    stringtop++;
    if (stringtop>=128) stringtop = 0;

    #ifdef DEBUG
        //har fast lengde på stringene. Hvis den er for lang vil ikke alt bli kopiert over. Usikker på hva
        //det kan fære til
        if (strlen(c) > maxNewString) {
                printf("New string is to long, will be truncated. Length was %i\n",strlen(c));
        }
    #endif

    strncpy(stringcircle[stringtop],c,maxNewString);

    return (int)stringcircle[stringtop];

}
