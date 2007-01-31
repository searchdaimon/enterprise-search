%{
#include "summary.parser.common.h"
#include "summary.parser.h"

#include <string.h>
#include <assert.h>

int _s_pushstate( int state, yyscan_t scanner );
int _s_popstate( yyscan_t scanner );
void _s_newstring( const char* c, yyscan_t scanner );

%}

character	[a-z]
legalchar	[a-z0-9_]
word		[0-9a-z'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþ]
allemuligetegn	[!"#$%'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ]
url		[0-9a-z&-.:/\\?=]
blank		[ \t\n]
tillatesc	(aacute|acirc|aelig|agrave|aring|atilde|auml|ccedil|eacute|ecirc|egrave|eth|euml|iacute|icirc|igrave|iuml|ntilde|oacute|ocirc|ograve|oslash|otilde|ouml|sup1|sup2|sup3|szlig|thorn|uacute|ucirc|ugrave|uuml|yacute|yuml)
tillatnumesc	(#178|#179|#185|#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255)
nestenalleesc	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
letter		[A-Za-z0-9_'´ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
letteresc	(AElig|Aacute|Acirc|Agrave|Aring|Atilde|Auml|Ccedil|ETH|Eacute|Ecirc|Egrave|Euml|Iacute|Icirc|Igrave|Iuml|Ntilde|Oacute|Ocirc|Ograve|Oslash|Otilde|Ouml|THORN|Uacute|Ucirc|Ugrave|Uuml|Yacute|aacute|acirc|aelig|agrave|aring|atilde|auml|ccedil|eacute|ecirc|egrave|eth|euml|iacute|icirc|igrave|iuml|ntilde|oacute|ocirc|ograve|oslash|otilde|ouml|szlig|thorn|uacute|ucirc|ugrave|uuml|yacute|yuml)
letternumesc	(#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255)
ikkeletter	[!"#$%()*+,-./:;=?@[\]^{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿×÷]
ikkeletteresc	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|times|divide|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
egentligesc	[²³¹ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
alletegnutenesc	[\\!"#$%'()*+,\-\./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±´µ¶·¸º»¼½¾¿×÷]
    /* Det er ingen html-escapes som ikke er med i disse (252 tilsammen): */
allenumesc	(#160|#161|#162|#163|#164|#165|#166|#167|#168|#169|#170|#171|#172|#173|#174|#175|#176|#177|#178|#179|#180|#181|#182|#183|#184|#185|#186|#187|#188|#189|#190|#191|#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#215|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#247|#248|#249|#250|#251|#252|#253|#254|#255|#402|#913|#914|#915|#916|#917|#918|#919|#920|#921|#922|#923|#924|#925|#926|#927|#928|#929|#931|#932|#933|#934|#935|#936|#937|#945|#946|#947|#948|#949|#950|#951|#952|#953|#954|#955|#956|#957|#958|#959|#960|#961|#962|#963|#964|#965|#966|#967|#968|#969|#977|#978|#982|#8226|#8230|#8242|#8243|#8254|#8260|#8472|#8465|#8476|#8482|#8501|#8592|#8593|#8594|#8595|#8596|#8629|#8656|#8657|#8658|#8659|#8660|#8704|#8706|#8707|#8709|#8711|#8712|#8713|#8715|#8719|#8721|#8722|#8727|#8730|#8733|#8734|#8736|#8743|#8744|#8745|#8746|#8747|#8756|#8764|#8773|#8776|#8800|#8801|#8804|#8805|#8834|#8835|#8836|#8838|#8839|#8853|#8855|#8869|#8901|#8968|#8969|#8970|#8971|#9001|#9002|#9674|#9824|#9827|#9829|#9830|#34|#38|#60|#62|#338|#339|#352|#353|#376|#710|#732|#8194|#8195|#8201|#8204|#8205|#8206|#8207|#8211|#8212|#8216|#8217|#8218|#8220|#8221|#8222|#8224|#8225|#8240|#8249|#8250|#8364)
alleesc		(nbsp|iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
alleesc_minus_nbsp (iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
%option	nomain noyywrap reentrant
%x	TAG ENDTAG COMMENT SCRIPT STYLE SELECT TEXTAREA
%%
 /* ({allemuligetegn}|\&{nestenalleesc}|\&{nestenalleesc};|\&\#[0-9]+;)+    { _s_newstring(yytext, yyscanner); return WORD; } */
 /* Midlertidig fiks for escapes uten avsluttende semikolon: */
 /* ({allemuligetegn}|\&{nestenalleesc};|\&\#[0-9]+;)+    { _s_newstring(yytext, yyscanner); return WORD; } */
 /* \&[^; ]*;		{} // Ignore other escapes. */
 /* \&			{ _s_newstring("&amp;", yyscanner); return WORD; } // Send ampersands not valid escapes as &amp; */
\303\246		{ _s_newstring("&aelig;", yyscanner); return ESC; }	// æ
\303\270		{ _s_newstring("&oslash;", yyscanner); return ESC; }	// ø
\303\245		{ _s_newstring("&aring;", yyscanner); return ESC; }	// å
\303\206		{ _s_newstring("&AElig;", yyscanner); return ESC; }	// Æ
\303\230		{ _s_newstring("&Oslash;", yyscanner); return ESC; }	// Ø
\303\205		{ _s_newstring("&Aring;", yyscanner); return ESC; }	// Å
({alletegnutenesc})+	{ _s_newstring(yytext, yyscanner); return WORD; }
({egentligesc})+	{ _s_newstring(yytext, yyscanner); return ESC; }
\&{alleesc_minus_nbsp};	{ _s_newstring(yytext, yyscanner); return ESC; }
\&{allenumesc};		{ _s_newstring(yytext, yyscanner); return ESC; }
\&			{ _s_newstring("&amp;", yyscanner); return ESC; }
\&nbsp;			{ (yyget_extra(yyscanner))->token.space = 1; }
[\ \t\n\r]		{ (yyget_extra(yyscanner))->token.space = 1; }
\<{blank}*script[^\>]*\> {	// Ignore characters between script-tags.
			    BEGIN _s_pushstate( SCRIPT, yyscanner );
			}
\<{blank}*style[^\>]*\> {	// Ignore characters between style-tags.
			    BEGIN _s_pushstate( STYLE, yyscanner );
			}
\<{blank}*select[^\>]*\> {	// Ignore characters between select-tags.
			    BEGIN _s_pushstate( SELECT, yyscanner );
			}
\<{blank}*textarea[^\>]*\> {	// Ignore characters between textarea-tags.
			    BEGIN _s_pushstate( TEXTAREA, yyscanner );
			}
\<\/			{
			    _s_newstring(yytext, yyscanner);
			    BEGIN _s_pushstate( ENDTAG, yyscanner );
			    return ENDTAG_START;
			}
\<			{
			    _s_newstring(yytext, yyscanner);
			    BEGIN _s_pushstate( TAG, yyscanner );
			    return TAG_START;
			}
\<\!\-\-		{
			    BEGIN _s_pushstate( COMMENT, yyscanner );
			}
<TAG>\/\>		{
			    _s_newstring(yytext, yyscanner);
			    BEGIN _s_popstate(yyscanner);
			    return TAG_ENDTAG_STOPP;
			}
<TAG>\>			{
			    _s_newstring(yytext, yyscanner);
			    BEGIN _s_popstate(yyscanner);
			    return TAG_STOPP;
			}
<TAG>\=			{
			    _s_newstring(yytext, yyscanner);
			    return EQUALS;
			}
<TAG>{legalchar}+	{
			    _s_newstring(yytext, yyscanner);
			    return ATTR;
			}
<ENDTAG>\>		{
			    _s_newstring(yytext, yyscanner);
			    BEGIN _s_popstate(yyscanner);
			    return ENDTAG_STOPP;
			}
<ENDTAG>{legalchar}+	{
			    _s_newstring(yytext, yyscanner);
			    return ATTR;
			}
<TAG>\'(\\\'|[^\'])*\'	{
			    _s_newstring(yytext, yyscanner);
			    return TEXTFIELD;
			}
<TAG>\"(\\\"|[^\"])*\"	{
			    _s_newstring(yytext, yyscanner);
			    return TEXTFIELD;
			}
<COMMENT>\-\-\>		{			BEGIN _s_popstate(yyscanner); }
<SCRIPT>\<\/{blank}*script[^\>]*\>	{	BEGIN _s_popstate(yyscanner); }
<STYLE>\<\/{blank}*style[^\>]*\>	{	BEGIN _s_popstate(yyscanner); }
<SELECT>\<\/{blank}*select[^\>]*\>	{	BEGIN _s_popstate(yyscanner); }
<TEXTAREA>\<\/{blank}*textarea[^\>]*\>	{	BEGIN _s_popstate(yyscanner); }
<*>.|\n			{}
%%

int _s_pushstate( int state, yyscan_t scanner )
{
    struct _sp_yy_extra		*se = yyget_extra( scanner );
    se->token.space = 1;

    if (se->top >= 127) return se->stack[se->top];	// overflow
    se->stack[++(se->top)] = state;
    return state;
}

int _s_popstate( yyscan_t scanner )
{
    struct _sp_yy_extra		*se = yyget_extra( scanner );

    if (se->top<=0)	// underflow
	{
	    se->top--;
	    return INITIAL;
	}
    return se->stack[--(se->top)];
}


void _s_newstring( const char* c, yyscan_t scanner )
{
    struct _sp_yy_extra		*se = yyget_extra( scanner );

//Runar
// Bruker statisk minne, ikke dynamisk, da vi en skjelden gang får segfeil i free
// desuten er det raskere, ved at vi ikke gjør systemkallet malloc heletiden

    se->stringtop++;
    if (se->stringtop >= 128) se->stringtop = 0;

    #ifdef DEBUG
        //har fast lengde på stringene. Hvis den er for lang vil ikke alt bli kopiert over. Usikker på hva
        //det kan fære til
	// Ax: Strenger lengre enn 2k er uansett søppel som ikke bør vises i sammendraget.
	// Ax: Tror ikke dette skal ha noen dårlige ringvirkninger.
        if (strlen(c) > maxNewString) {
                printf("New string is to long, will be truncated. Length was %i\n",strlen(c));
        }
    #endif

//    assert( strlen(c) <= maxNewString );
    if ( strlen(c) <= maxNewString )
	{
	    strncpy( se->stringcircle[se->stringtop], c, maxNewString );
	    se->stringcircle[se->stringtop][strlen(c)] = '\0';
	}
    else
	{
	    fprintf( stderr, "Warning: summary.parser.l: Parsed word is bigger than %i characters!", maxNewString );
	    strcpy( se->stringcircle[se->stringtop], "" );
	}

    se->token.str = se->stringcircle[se->stringtop];
}
