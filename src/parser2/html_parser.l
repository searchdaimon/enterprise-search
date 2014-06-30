%{
#include "html_parser_common.h"
#include "bhpm.tab.h"
#include "../common/search_automaton.h"
#include "../ds/dcontainer.h"
#include "css_parser.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef PARSE_ERR
    #define D	PRINT(bhpmget_extra(yyscanner), "%s", yytext)
#else
    #define D	1
#endif

/*
 * Denne lexical analyzeren støtter Latin-1, Latin-1 supplement, utf-8, samt character entities (html escapes)
 * i alle dokumenter. Den ser ikke på hva som er definert character encoding, men antar at en eller flere av
 * disse brukes. Alt (med unntak av quot,amp,lt og gt) oversettes til utf-8. Vi har følgende mangler (todo):
 *
 * - Ingen støtte for andre charsets.
 *
 * Se forøvrig http://www.w3.org/TR/html4/charset.html
 *
 * (C) SearchDaimon AS, Searchdaimon AS 2004-2008, Magnus Galåen (magnusga@idi.ntnu.no)
 */

#define BHPM_VISIBLE (!(bhpmget_extra(yyscanner)->invisible_text || bhpmget_extra(yyscanner)->illegal_charset))

static inline void bhpm_newstring( char* str, yyscan_t scanner, int tt );
static inline void bhpm_newutf8( int val, yyscan_t scanner, int tt );
static inline void bhpm_new_esc_to_utf8( const char* str, yyscan_t scanner, int tt );
static inline void bhpm_newunicode( int val, yyscan_t scanner );

%}

character	[a-z]
legalchar	[a-z0-9'+#%]
attrchar	[a-z0-9\-_]
legaltegn	[!"#$%()*+,\-_./:;=?@[\\\]^{|}~]
tfchars		[a-z0-9_`!#$%()*+,\-./:;=?@[\]^{|}~]
url		[0-9a-z&-.:/\\?=]
blank		[ \t\n\r\f]
latin-sup	[ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
    /* Her har vi fjernet noen html-spesifikke tegn (lt,gt,amp,quot, samt nbsp): */
letteresc	(Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml)
tegnesc_minus	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|times|divide|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
utf8-a		([\101-\132]|[\141-\172]|\302\252|\302\265|\302\272|\303[\200-\226]|\303[\230-\266]|\303[\270-\277]|[\304-\312][\200-\277]|\313[\200-\201]|\313[\206-\221]|\313[\240-\244]|\313\254|\313\256|\315\205|\315[\260-\264]|\315[\266-\267]|\315[\272-\275]|\316\206|\316[\210-\212]|\316\214|\316[\216-\241]|\316[\243-\277]|\317[\200-\265]|\317[\267-\277]|[\320-\321][\200-\277]|\322[\200-\201]|\322[\212-\277]|\324[\200-\243]|\324[\261-\277]|\325[\200-\226]|\325\231|\325[\241-\277]|\326[\200-\207]|\326[\260-\275]|\326\277|\327[\201-\202]|\327[\204-\205]|\327\207|\327[\220-\252]|\327[\260-\262]|\330[\220-\232]|\330[\241-\277]|\331[\200-\227]|\331[\231-\236]|\331[\256-\277]|\333[\200-\223]|\333[\225-\234]|\333[\241-\250]|\333[\255-\257]|\333[\272-\274]|\333\277|\334[\220-\277]|\335[\215-\277]|\336[\200-\261]|\337[\212-\252]|\337[\264-\265]|\337\272|\340\244[\201-\271]|\340\244[\275-\277]|\340\245[\200-\214]|\340\245\220|\340\245[\230-\243]|\340\245[\261-\262]|\340\245[\273-\277]|\340\246[\201-\203]|\340\246[\205-\214]|\340\246[\217-\220]|\340\246[\223-\250]|\340\246[\252-\260]|\340\246\262|\340\246[\266-\271]|\340\246[\275-\277]|\340\247[\200-\204]|\340\247[\207-\210]|\340\247[\213-\214]|\340\247\216|\340\247\227|\340\247[\234-\235]|\340\247[\237-\243]|\340\247[\260-\261]|\340\250[\201-\203]|\340\250[\205-\212]|\340\250[\217-\220]|\340\250[\223-\250]|\340\250[\252-\260]|\340\250[\262-\263]|\340\250[\265-\266]|\340\250[\270-\271]|\340\250[\276-\277]|\340\251[\200-\202]|\340\251[\207-\210]|\340\251[\213-\214]|\340\251\221|\340\251[\231-\234]|\340\251\236|\340\251[\260-\265]|\340\252[\201-\203]|\340\252[\205-\215]|\340\252[\217-\221]|\340\252[\223-\250]|\340\252[\252-\260]|\340\252[\262-\263]|\340\252[\265-\271]|\340\252[\275-\277]|\340\253[\200-\205]|\340\253[\207-\211]|\340\253[\213-\214]|\340\253\220|\340\253[\240-\243]|\340\254[\201-\203]|\340\254[\205-\214]|\340\254[\217-\220]|\340\254[\223-\250]|\340\254[\252-\260]|\340\254[\262-\263]|\340\254[\265-\271]|\340\254[\275-\277]|\340\255[\200-\204]|\340\255[\207-\210])
utf8-b		(\340\255[\213-\214]|\340\255[\226-\227]|\340\255[\234-\235]|\340\255[\237-\243]|\340\255\261|\340\256[\202-\203]|\340\256[\205-\212]|\340\256[\216-\220]|\340\256[\222-\225]|\340\256[\231-\232]|\340\256\234|\340\256[\236-\237]|\340\256[\243-\244]|\340\256[\250-\252]|\340\256[\256-\271]|\340\256[\276-\277]|\340\257[\200-\202]|\340\257[\206-\210]|\340\257[\212-\214]|\340\257\220|\340\257\227|\340\260[\201-\203]|\340\260[\205-\214]|\340\260[\216-\220]|\340\260[\222-\250]|\340\260[\252-\263]|\340\260[\265-\271]|\340\260[\275-\277]|\340\261[\200-\204]|\340\261[\206-\210]|\340\261[\212-\214]|\340\261[\225-\226]|\340\261[\230-\231]|\340\261[\240-\243]|\340\262[\202-\203]|\340\262[\205-\214]|\340\262[\216-\220]|\340\262[\222-\250]|\340\262[\252-\263]|\340\262[\265-\271]|\340\262[\275-\277]|\340\263[\200-\204]|\340\263[\206-\210]|\340\263[\212-\214]|\340\263[\225-\226]|\340\263\236|\340\263[\240-\243]|\340\264[\202-\203]|\340\264[\205-\214]|\340\264[\216-\220]|\340\264[\222-\250]|\340\264[\252-\271]|\340\264[\275-\277]|\340\265[\200-\204]|\340\265[\206-\210]|\340\265[\212-\214]|\340\265\227|\340\265[\240-\243]|\340\265[\272-\277]|\340\266[\202-\203]|\340\266[\205-\226]|\340\266[\232-\261]|\340\266[\263-\273]|\340\266\275|\340\267[\200-\206]|\340\267[\217-\224]|\340\267\226|\340\267[\230-\237]|\340\267[\262-\263]|\340\270[\201-\272]|\340\271[\200-\206]|\340\271\215|\340\272[\201-\202]|\340\272\204|\340\272[\207-\210]|\340\272\212|\340\272\215|\340\272[\224-\227]|\340\272[\231-\237]|\340\272[\241-\243]|\340\272\245|\340\272\247|\340\272[\252-\253]|\340\272[\255-\271]|\340\272[\273-\275]|\340\273[\200-\204]|\340\273\206|\340\273\215|\340\273[\234-\235]|\340\274\200|\340\275[\200-\207]|\340\275[\211-\254]|\340\275[\261-\277]|\340\276[\200-\201]|\340\276[\210-\213]|\340\276[\220-\227]|\340\276[\231-\274]|\341\200[\200-\266]|\341\200\270|\341\200[\273-\277]|\341\201[\220-\242]|\341\201[\245-\250]|\341\201[\256-\277]|\341\202[\200-\206]|\341\202\216|\341\202[\240-\277]|\341\203[\200-\205]|\341\203[\220-\272])
utf8-c		(\341\203\274|\341\204[\200-\277]|\341\205[\200-\231]|\341\205[\237-\277]|\341\206[\200-\242]|\341\206[\250-\277]|\341\207[\200-\271]|\341\210[\200-\277]|\341\211[\200-\210]|\341\211[\212-\215]|\341\211[\220-\226]|\341\211\230|\341\211[\232-\235]|\341\211[\240-\277]|\341\212[\200-\210]|\341\212[\212-\215]|\341\212[\220-\260]|\341\212[\262-\265]|\341\212[\270-\276]|\341\213\200|\341\213[\202-\205]|\341\213[\210-\226]|\341\213[\230-\277]|\341\214[\200-\220]|\341\214[\222-\225]|\341\214[\230-\277]|\341\215[\200-\232]|\341\215\237|\341\216[\200-\217]|\341\216[\240-\277]|\341\217[\200-\264]|\341\220[\201-\277]|\341[\221-\230][\200-\277]|\341\231[\200-\254]|\341\231[\257-\266]|\341\232[\201-\232]|\341\232[\240-\277]|\341\233[\200-\252]|\341\233[\256-\260]|\341\234[\200-\214]|\341\234[\216-\223]|\341\234[\240-\263]|\341\235[\200-\223]|\341\235[\240-\254]|\341\235[\256-\260]|\341\235[\262-\263]|\341\236[\200-\263]|\341\236[\266-\277]|\341\237[\200-\210]|\341\237\227|\341\237\234|\341\240[\240-\277]|\341\241[\200-\267]|\341\242[\200-\252]|\341\244[\200-\234]|\341\244[\240-\253]|\341\244[\260-\270]|\341\245[\220-\255]|\341\245[\260-\264]|\341\246[\200-\251]|\341\246[\260-\277]|\341\247[\200-\211]|\341\250[\200-\233]|\341\254[\200-\263]|\341\254[\265-\277]|\341\255[\200-\203]|\341\255[\205-\213]|\341\256[\200-\251]|\341\256[\256-\257]|\341\260[\200-\265]|\341\261[\215-\217]|\341\261[\232-\275]|\341[\264-\266][\200-\277]|\341\270[\200-\277]|\341[\271-\273][\200-\277]|\341\274[\200-\225]|\341\274[\230-\235]|\341\274[\240-\277]|\341\275[\200-\205]|\341\275[\210-\215]|\341\275[\220-\227]|\341\275\231|\341\275\233|\341\275\235|\341\275[\237-\275]|\341\276[\200-\264]|\341\276[\266-\274]|\341\276\276|\341\277[\202-\204]|\341\277[\206-\214]|\341\277[\220-\223]|\341\277[\226-\233]|\341\277[\240-\254]|\341\277[\262-\264]|\341\277[\266-\274]|\342\201\261|\342\201\277|\342\202[\220-\224]|\342\204\202|\342\204\207|\342\204[\212-\223]|\342\204\225|\342\204[\231-\235]|\342\204\244|\342\204\246|\342\204\250|\342\204[\252-\255])
utf8-d		(\342\204[\257-\271]|\342\204[\274-\277]|\342\205[\205-\211]|\342\205\216|\342\205[\240-\277]|\342\206[\200-\210]|\342\222[\266-\277]|\342\223[\200-\251]|\342\260[\200-\256]|\342\260[\260-\277]|\342\261[\200-\236]|\342\261[\240-\257]|\342\261[\261-\275]|\342\262[\200-\277]|\342\263[\200-\244]|\342\264[\200-\245]|\342\264[\260-\277]|\342\265[\200-\245]|\342\265\257|\342\266[\200-\226]|\342\266[\240-\246]|\342\266[\250-\256]|\342\266[\260-\266]|\342\266[\270-\276]|\342\267[\200-\206]|\342\267[\210-\216]|\342\267[\220-\226]|\342\267[\230-\236]|\342\267[\240-\277]|\342\270\257|\343\200[\205-\207]|\343\200[\241-\251]|\343\200[\261-\265]|\343\200[\270-\274]|\343\201[\201-\277]|\343\202[\200-\226]|\343\202[\235-\237]|\343\202[\241-\277]|\343\203[\200-\272]|\343\203[\274-\277]|\343\204[\205-\255]|\343\204[\261-\277]|\343\206[\200-\216]|\343\206[\240-\267]|\343\207[\260-\277]|\343[\220-\277][\200-\277]|\344[\200-\265][\200-\277]|\344\266[\200-\265]|\344[\270-\277][\200-\277]|[\345-\350][\200-\277][\200-\277]|\351[\200-\276][\200-\277]|\351\277[\200-\203]|\352[\200-\221][\200-\277]|\352\222[\200-\214]|\352\224[\200-\277]|\352[\225-\227][\200-\277]|\352\230[\200-\214]|\352\230[\220-\237]|\352\230[\252-\253]|\352\231[\200-\237]|\352\231[\242-\256]|\352\231[\277-\277]|\352\232[\200-\227]|\352\234[\227-\237]|\352\234[\242-\277]|\352\236[\200-\210]|\352\236[\213-\214]|\352\237[\273-\277]|\352\240[\200-\201]|\352\240[\203-\205]|\352\240[\207-\212]|\352\240[\214-\247]|\352\241[\200-\263]|\352\242[\200-\277]|\352\243[\200-\203]|\352\244[\212-\252]|\352\244[\260-\277]|\352\245[\200-\222]|\352\250[\200-\266]|\352\251[\200-\215]|\352[\260-\277][\200-\277]|[\353-\354][\200-\277][\200-\277]|\355[\200-\235][\200-\277]|\355\236[\200-\243]|\357\244[\200-\277]|\357[\245-\247][\200-\277]|\357\250[\200-\255]|\357\250[\260-\277]|\357\251[\200-\252]|\357\251[\260-\277]|\357\253[\200-\231]|\357\254[\200-\206]|\357\254[\223-\227]|\357\254[\235-\250]|\357\254[\252-\266]|\357\254[\270-\274]|\357\254\276|\357\255[\200-\201]|\357\255[\203-\204])
utf8-e		(\357\255[\206-\277]|\357\256[\200-\261]|\357\257[\223-\277]|\357[\260-\263][\200-\277]|\357\264[\200-\275]|\357\265[\220-\277]|\357\266[\200-\217]|\357\266[\222-\277]|\357\267[\200-\207]|\357\267[\260-\273]|\357\271[\260-\264]|\357\271[\266-\277]|\357\273[\200-\274]|\357\274[\241-\272]|\357\275[\201-\232]|\357\275[\246-\277]|\357\276[\200-\276]|\357\277[\202-\207]|\357\277[\212-\217]|\357\277[\222-\227]|\357\277[\232-\234]|\360\220\200[\200-\213]|\360\220\200[\215-\246]|\360\220\200[\250-\272]|\360\220\200[\274-\275]|\360\220\200\277|\360\220\201[\200-\215]|\360\220\201[\220-\235]|\360\220\202[\200-\277]|\360\220\203[\200-\272]|\360\220\205[\200-\264]|\360\220\212[\200-\234]|\360\220\212[\240-\277]|\360\220\213[\200-\220]|\360\220\214[\200-\236]|\360\220\214[\260-\277]|\360\220\215[\200-\212]|\360\220\216[\200-\235]|\360\220\216[\240-\277]|\360\220\217[\200-\203]|\360\220\217[\210-\217]|\360\220\217[\221-\225]|\360\220\220[\200-\277]|\360\220\222[\200-\235]|\360\220\240[\200-\205]|\360\220\240\210|\360\220\240[\212-\265]|\360\220\240[\267-\270]|\360\220\240\274|\360\220\240\277|\360\220\244[\200-\225]|\360\220\244[\240-\271]|\360\220\250[\200-\203]|\360\220\250[\205-\206]|\360\220\250[\214-\223]|\360\220\250[\225-\227]|\360\220\250[\231-\263]|\360\222[\200-\214][\200-\277]|\360\222\215[\200-\256]|\360\222\220[\200-\277]|\360\222\221[\200-\242]|\360\235\220[\200-\277]|\360\235\221[\200-\224]|\360\235\221[\226-\277]|\360\235\222[\200-\234]|\360\235\222[\236-\237]|\360\235\222\242|\360\235\222[\245-\246]|\360\235\222[\251-\254]|\360\235\222[\256-\271]|\360\235\222\273|\360\235\222[\275-\277]|\360\235\223[\200-\203]|\360\235\223[\205-\277]|\360\235\224[\200-\205]|\360\235\224[\207-\212]|\360\235\224[\215-\224]|\360\235\224[\226-\234]|\360\235\224[\236-\271]|\360\235\224[\273-\276]|\360\235\225[\200-\204]|\360\235\225\206|\360\235\225[\212-\220]|\360\235\225[\222-\277]|\360\235[\226-\231][\200-\277]|\360\235\232[\200-\245]|\360\235\232[\250-\277]|\360\235\233[\200-\200]|\360\235\233[\202-\232])
utf8-f		(\360\235\233[\234-\272]|\360\235\233[\274-\277]|\360\235\234[\200-\224]|\360\235\234[\226-\264]|\360\235\234[\266-\277]|\360\235\235[\200-\216]|\360\235\235[\220-\256]|\360\235\235[\260-\277]|\360\235\236[\200-\210]|\360\235\236[\212-\250]|\360\235\236[\252-\277]|\360\235\237[\200-\202]|\360\235\237[\204-\213]|\360[\240-\251][\200-\277][\200-\277]|\360\252[\200-\232][\200-\277]|\360\252\233[\200-\226]|\360\257\240[\200-\277]|\360\257[\241-\247][\200-\277]|\360\257\250[\200-\235])
utf8-all	([\300-\337][\200-\277]|[\340-\357][\200-\277][\200-\277]|[\360-\367][\200-\277][\200-\277][\200-\277])
%option	nomain noyywrap reentrant bison-bridge
%x	TAG ENDTAG COMMENT SCRIPT STYLE SELECT TEXTAREA NOSCRIPT NOFRAMES OBJECT TAG_TF
%%
    /* Pga av begrensning på maks linjelengde i lex, må utf8-bokstavene spesifiseres over flere character classes: */
({utf8-a}|{utf8-b}|{utf8-c}|{utf8-d}|{utf8-e}|{utf8-f})+	{ D; if (BHPM_VISIBLE) bhpm_newstring(yytext, yyscanner, WORD); }
{utf8-all}		{ D; if (BHPM_VISIBLE) bhpm_newstring(yytext, yyscanner, ESC); }
\`			{ D; if (BHPM_VISIBLE) bhpm_newstring("'", yyscanner, WORD); }
{legalchar}+		{ D; if (BHPM_VISIBLE) bhpm_newstring(yytext, yyscanner, WORD); }
{latin-sup}		{ D; if (BHPM_VISIBLE) bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, WORD ); }
{legaltegn}+		{ D; if (BHPM_VISIBLE) bhpm_newstring(yytext, yyscanner, ESC); }
[\200-\377]		{ D; if (BHPM_VISIBLE) bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, ESC ); }
\&{letteresc};?		{ D; if (BHPM_VISIBLE) bhpm_new_esc_to_utf8(yytext, yyscanner, WORD); }
\&{tegnesc_minus};?	{ D; if (BHPM_VISIBLE) bhpm_new_esc_to_utf8(yytext, yyscanner, ESC); }
    /* quot,amp,lt og gt bør alltid sendes videre som character entities (html-escapes): */
(\&quot;?|\&#34;?)	{ D; if (BHPM_VISIBLE) bhpm_newstring("&quot;", yyscanner, ESC); }
(\&amp;?|\&#38;?)	{ D; if (BHPM_VISIBLE) bhpm_newstring("&amp;", yyscanner, ESC); }
(\&lt;?|\&#60;?)	{ D; if (BHPM_VISIBLE) bhpm_newstring("&lt;", yyscanner, ESC); }
(\&gt;?|\&#62;?)	{ D; if (BHPM_VISIBLE) bhpm_newstring("&gt;", yyscanner, ESC); }
(\&#35;?)		{ D; if (BHPM_VISIBLE) bhpm_newstring("#", yyscanner, WORD); }
(\&#43;?)		{ D; if (BHPM_VISIBLE) bhpm_newstring("+", yyscanner, WORD); }
(\&#45;?)		{ D; if (BHPM_VISIBLE) bhpm_newstring("%", yyscanner, WORD); }
\&nbsp;?		{ /* ignore */ }
\&#[0-9]+;		{ D; if (BHPM_VISIBLE) bhpm_newunicode( atoi(&(yytext[2])), yyscanner ); }
\&#(x|X)[0-9a-fA-F]+;	{ D; if (BHPM_VISIBLE) bhpm_newunicode( strtol(&(yytext[3]), NULL, 16), yyscanner ); }
\&			{ D; if (BHPM_VISIBLE) bhpm_newstring("&amp;", yyscanner, ESC); }
\<{blank}*script(\>|{blank}[^\>]*\>) { D;	// Ignore characters between script-tags.
			    BEGIN SCRIPT;
			}
\<{blank}*style(\>|{blank}[^\>]*\>) { D;	// To be sent to css-parser.
			    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

			    he->ss_size = 0;
			    he->ss_block = 65536;
			    he->stylesheet = malloc(he->ss_block);

			    BEGIN STYLE;
			}
\<{blank}*select(\>|{blank}[^\>]*\>) { D;	// Ignore characters between select-tags.
			    BEGIN SELECT;
			}
\<{blank}*textarea(\>|{blank}[^\>]*\>) { D;	// Ignore characters between textarea-tags.
			    BEGIN TEXTAREA;
			}
\<{blank}*noscript(\>|{blank}[^\>]*\>) { D;
			    BEGIN NOSCRIPT;
			}
\<{blank}*noframes(\>|{blank}[^\>]*\>) { D;
			    BEGIN NOFRAMES;
			}
\<{blank}*object(\>|{blank}[^\>]*\>) { D;
			    bhpmget_extra(yyscanner)->nested_obj = 1;
			    BEGIN OBJECT;
			}
\<\/			{ D;
			    BEGIN ENDTAG;
			    return ENDTAG_START;
			}
\<			{ D;
			    BEGIN TAG;
			    return TAG_START;
			}
\<\!\-\-		{ D;
			    BEGIN COMMENT;
			}
<TAG>\/\>		{ D;
			    BEGIN INITIAL;
			    return TAG_ENDTAG_STOPP;
			}
<TAG>\>			{ D;
			    BEGIN INITIAL;
			    return TAG_STOPP;
			}
<TAG>\=			{ D;
			    return EQUALS;
			}
<TAG>{attrchar}+	{ D;
			    bhpm_newstring(yytext, yyscanner, ATTR);
			    return ATTR;
			}
<TAG>:{attrchar}+	{ D; } /* Ignore */
<ENDTAG>\>		{ D;
			    BEGIN INITIAL;
			    return ENDTAG_STOPP;
			}
<ENDTAG>{attrchar}+	{ D;
			    bhpm_newstring(yytext, yyscanner, ATTR);
			    return ATTR;
			}
<ENDTAG>:{attrchar}+	{ D; } /* Ignore */
<TAG>\'			{ D;
			    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
			    he->flush = 1;
			    he->field_delimit = '\'';
			    BEGIN TAG_TF;
			}
<TAG>\"			{ D;
			    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
			    he->flush = 1;
			    he->field_delimit = '"';
			    BEGIN TAG_TF;
			}
<TAG_TF>\\\"		{ D; bhpm_newstring("\"", yyscanner, TEXTFIELD); }
<TAG_TF>\\\'		{ D; bhpm_newstring("\'", yyscanner, TEXTFIELD); }
<TAG_TF>\"		{ D;
			    if ((bhpmget_extra(yyscanner))->field_delimit == '"')
				{
				    BEGIN TAG;
				    return TEXTFIELD;
				}
			    else bhpm_newstring("\"", yyscanner, TEXTFIELD);
			}
<TAG_TF>\'		{ D;
			    if ((bhpmget_extra(yyscanner))->field_delimit == '\'')
				{
				    BEGIN TAG;
				    return TEXTFIELD;
				}
			    else bhpm_newstring("\'", yyscanner, TEXTFIELD);
			}
    /* Tatt fra toppen: */
<TAG_TF>\\			{ D; /* ignore */ }
<TAG_TF>({utf8-all}|{tfchars})+	{ D; bhpm_newstring(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>{latin-sup}		{ D; bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, TEXTFIELD ); }
<TAG_TF>\&({letteresc}|{tegnesc_minus});?	{ D; bhpm_new_esc_to_utf8(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>(\&(nbsp|#160);?|{blank})+	{ D; bhpm_newstring(" ", yyscanner, TEXTFIELD); }
<TAG_TF>(\&(quot|#34);?)	{ D; bhpm_newstring("&quot;", yyscanner, TEXTFIELD); }
<TAG_TF>(\&(amp|#38);?)		{ D; bhpm_newstring("&amp;", yyscanner, TEXTFIELD); }
<TAG_TF>(\&(lt|#60);?)		{ D; bhpm_newstring("&lt;", yyscanner, TEXTFIELD); }
<TAG_TF>(\&(gt|#62);?)		{ D; bhpm_newstring("&gt;", yyscanner, TEXTFIELD); }
<TAG_TF>\&#[0-9]+;		{ D; bhpm_newutf8( atoi(&(yytext[2])), yyscanner, TEXTFIELD ); }
<TAG_TF>\&#(x|X)[0-9a-fA-F]+;	{ D; bhpm_newutf8( strtol(&(yytext[3]), NULL, 16), yyscanner, TEXTFIELD ); }
<TAG_TF>\&			{ D; bhpm_newstring("&amp;", yyscanner, TEXTFIELD); }

<COMMENT>\-\-\>			{ D; BEGIN INITIAL; }
<SCRIPT>\<\/{blank}*script(\>|{blank}[^\>]*\>)		{ D; BEGIN INITIAL; }
<STYLE>\<\/{blank}*style(\>|{blank}[^\>]*\>)		{ D;
				    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

				    he->stylesheet[he->ss_size] = '\0';

				    if (he->css_selector_block!=NULL)
					destroy_selectors(he->css_selector_block);
				    he->css_selector_block = css_parser_run(he->stylesheet, he->ss_size);
				    free(he->stylesheet);

				    BEGIN INITIAL;
				}
<STYLE>(.|\n)			{ D;
				    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);

				    he->stylesheet[he->ss_size++] = yytext[0];

				    if (he->ss_size >= he->ss_block)
					{
					    char	*new_block = malloc(he->ss_block*2);

					    memcpy(new_block, he->stylesheet, he->ss_block);
					    he->ss_block*= 2;
					    free(he->stylesheet);
					    he->stylesheet = new_block;
					}
				}
<SELECT>\<\/{blank}*select(\>|{blank}[^\>]*\>)		{ D; BEGIN INITIAL; }
<TEXTAREA>\<\/{blank}*textarea(\>|{blank}[^\>]*\>)	{ D; BEGIN INITIAL; }
<NOSCRIPT>\<\/{blank}*noscript(\>|{blank}[^\>]*\>)	{ D; BEGIN INITIAL; }
<NOFRAMES>\<\/{blank}*noframes(\>|{blank}[^\>]*\>)	{ D; BEGIN INITIAL; }
<OBJECT>\<{blank}*object(\>|{blank}[^\>]*\>)		{ D; bhpmget_extra(yyscanner)->nested_obj++; }
<OBJECT>\<\/{blank}*object(\>|{blank}[^\>]*\>)		{ D; if ((--(bhpmget_extra(yyscanner)->nested_obj))==0) BEGIN INITIAL; }
    /* Framtving at ord-bufferet tømmes: */
<*><<EOF>>		{ bhpm_newstring("", yyscanner, -1); yyterminate(); }
.|\n			{ D; (bhpmget_extra(yyscanner))->space = 1; }
<*>.|\n			{ D; }
%%

/*
<TAG>\"(\\\"|[^\">])*\"	{
			    yytext[strlen(yytext)-1] = '\0';
			    bhpm_newstring(&(yytext[1]), yyscanner, TEXTFIELD);
			    return TEXTFIELD;
			}
*/

struct htmlesc_val
{
	char	*str;
	int	val;
};

struct htmlesc_val bhpm_heval[252] =
{{"AElig", 198},{"Aacute", 193},{"Acirc", 194},{"Agrave", 192},{"Alpha", 913},{"Aring", 197},
{"Atilde", 195},{"Auml", 196},{"Beta", 914},{"Ccedil", 199},{"Chi", 935},{"Dagger", 8225},
{"Delta", 916},{"ETH", 208},{"Eacute", 201},{"Ecirc", 202},{"Egrave", 200},{"Epsilon", 917},
{"Eta", 919},{"Euml", 203},{"Gamma", 915},{"Iacute", 205},{"Icirc", 206},{"Igrave", 204},
{"Iota", 921},{"Iuml", 207},{"Kappa", 922},{"Lambda", 923},{"Mu", 924},{"Ntilde", 209},
{"Nu", 925},{"OElig", 338},{"Oacute", 211},{"Ocirc", 212},{"Ograve", 210},{"Omega", 937},
{"Omicron", 927},{"Oslash", 216},{"Otilde", 213},{"Ouml", 214},{"Phi", 934},{"Pi", 928},
{"Prime", 8243},{"Psi", 936},{"Rho", 929},{"Scaron", 352},{"Sigma", 931},{"THORN", 222},
{"Tau", 932},{"Theta", 920},{"Uacute", 218},{"Ucirc", 219},{"Ugrave", 217},{"Upsilon", 933},
{"Uuml", 220},{"Xi", 926},{"Yacute", 221},{"Yuml", 376},{"Zeta", 918},{"aacute", 225},
{"acirc", 226},{"acute", 180},{"aelig", 230},{"agrave", 224},{"alefsym", 8501},{"alpha", 945},
{"amp", 38},{"and", 8743},{"ang", 8736},{"aring", 229},{"asymp", 8776},{"atilde", 227},
{"auml", 228},{"bdquo", 8222},{"beta", 946},{"brvbar", 166},{"bull", 8226},{"cap", 8745},
{"ccedil", 231},{"cedil", 184},{"cent", 162},{"chi", 967},{"circ", 710},{"clubs", 9827},
{"cong", 8773},{"copy", 169},{"crarr", 8629},{"cup", 8746},{"curren", 164},{"dArr", 8659},
{"dagger", 8224},{"darr", 8595},{"deg", 176},{"delta", 948},{"diams", 9830},{"divide", 247},
{"eacute", 233},{"ecirc", 234},{"egrave", 232},{"empty", 8709},{"emsp", 8195},{"ensp", 8194},
{"epsilon", 949},{"equiv", 8801},{"eta", 951},{"eth", 240},{"euml", 235},{"euro", 8364},
{"exist", 8707},{"fnof", 402},{"forall", 8704},{"frac12", 189},{"frac14", 188},{"frac34", 190},
{"frasl", 8260},{"gamma", 947},{"ge", 8805},{"gt", 62},{"hArr", 8660},{"harr", 8596},
{"hearts", 9829},{"hellip", 8230},{"iacute", 237},{"icirc", 238},{"iexcl", 161},{"igrave", 236},
{"image", 8465},{"infin", 8734},{"int", 8747},{"iota", 953},{"iquest", 191},{"isin", 8712},
{"iuml", 239},{"kappa", 954},{"lArr", 8656},{"lambda", 955},{"lang", 9001},{"laquo", 171},
{"larr", 8592},{"lceil", 8968},{"ldquo", 8220},{"le", 8804},{"lfloor", 8970},{"lowast", 8727},
{"loz", 9674},{"lrm", 8206},{"lsaquo", 8249},{"lsquo", 8216},{"lt", 60},{"macr", 175},
{"mdash", 8212},{"micro", 181},{"middot", 183},{"minus", 8722},{"mu", 956},{"nabla", 8711},
{"nbsp", 160},{"ndash", 8211},{"ne", 8800},{"ni", 8715},{"not", 172},{"notin", 8713},
{"nsub", 8836},{"ntilde", 241},{"nu", 957},{"oacute", 243},{"ocirc", 244},{"oelig", 339},
{"ograve", 242},{"oline", 8254},{"omega", 969},{"omicron", 959},{"oplus", 8853},{"or", 8744},
{"ordf", 170},{"ordm", 186},{"oslash", 248},{"otilde", 245},{"otimes", 8855},{"ouml", 246},
{"para", 182},{"part", 8706},{"permil", 8240},{"perp", 8869},{"phi", 966},{"pi", 960},
{"piv", 982},{"plusmn", 177},{"pound", 163},{"prime", 8242},{"prod", 8719},{"prop", 8733},
{"psi", 968},{"quot", 34},{"rArr", 8658},{"radic", 8730},{"rang", 9002},{"raquo", 187},
{"rarr", 8594},{"rceil", 8969},{"rdquo", 8221},{"real", 8476},{"reg", 174},{"rfloor", 8971},
{"rho", 961},{"rlm", 8207},{"rsaquo", 8250},{"rsquo", 8217},{"sbquo", 8218},{"scaron", 353},
{"sdot", 8901},{"sect", 167},{"shy", 173},{"sigma", 963},{"sigmaf", 962},{"sim", 8764},
{"spades", 9824},{"sub", 8834},{"sube", 8838},{"sum", 8721},{"sup", 8835},{"sup1", 185},
{"sup2", 178},{"sup3", 179},{"supe", 8839},{"szlig", 223},{"tau", 964},{"there4", 8756},
{"theta", 952},{"thetasym", 977},{"thinsp", 8201},{"thorn", 254},{"tilde", 732},{"times", 215},
{"trade", 8482},{"uArr", 8657},{"uacute", 250},{"uarr", 8593},{"ucirc", 251},{"ugrave", 249},
{"uml", 168},{"upsih", 978},{"upsilon", 965},{"uuml", 252},{"weierp", 8472},{"xi", 958},
{"yacute", 253},{"yen", 165},{"yuml", 255},{"zeta", 950},{"zwj", 8205},{"zwnj", 8204}};


struct htmlesc_val bhpm_hecval[191] =
{{"aacute", 193}, {"acirc", 194},
{"acute", 180}, {"aelig", 198}, {"agrave", 192}, {"alefsym", 8501}, {"alpha", 913}, {"amp", 38},
{"and", 8743}, {"ang", 8736}, {"aring", 197}, {"asymp", 8776}, {"atilde", 195}, {"auml", 196},
{"bdquo", 8222}, {"beta", 914}, {"brvbar", 166}, {"bull", 8226}, {"cap", 8745}, {"ccedil", 199},
{"cedil", 184}, {"cent", 162}, {"chi", 935}, {"circ", 710}, {"clubs", 9827}, {"cong", 8773},
{"copy", 169}, {"crarr", 8629}, {"cup", 8746}, {"curren", 164}, {"dagger", 8224}, {"darr", 8595},
{"deg", 176}, {"delta", 916}, {"diams", 9830}, {"divide", 247}, {"eacute", 201}, {"ecirc", 202},
{"egrave", 200}, {"empty", 8709}, {"emsp", 8195}, {"ensp", 8194}, {"epsilon", 917}, {"equiv", 8801},
{"eta", 919}, {"eth", 208}, {"euml", 203}, {"euro", 8364}, {"exist", 8707}, {"fnof", 402},
{"forall", 8704}, {"frac12", 189}, {"frac14", 188}, {"frac34", 190}, {"frasl", 8260}, {"gamma", 915},
{"ge", 8805}, {"gt", 62}, {"harr", 8596}, {"hearts", 9829}, {"hellip", 8230}, {"iacute", 205},
{"icirc", 206}, {"iexcl", 161}, {"igrave", 204}, {"image", 8465}, {"infin", 8734}, {"int", 8747},
{"iota", 921}, {"iquest", 191}, {"isin", 8712}, {"iuml", 207}, {"kappa", 922}, {"lambda", 923},
{"lang", 9001}, {"laquo", 171}, {"larr", 8592}, {"lceil", 8968}, {"ldquo", 8220}, {"le", 8804},
{"lfloor", 8970}, {"lowast", 8727}, {"loz", 9674}, {"lrm", 8206}, {"lsaquo", 8249}, {"lsquo", 8216},
{"lt", 60}, {"macr", 175}, {"mdash", 8212}, {"micro", 181}, {"middot", 183}, {"minus", 8722},
{"mu", 924}, {"nabla", 8711}, {"nbsp", 160}, {"ndash", 8211}, {"ne", 8800}, {"ni", 8715},
{"not", 172}, {"notin", 8713}, {"nsub", 8836}, {"ntilde", 209}, {"nu", 925}, {"oacute", 211},
{"ocirc", 212}, {"oelig", 338}, {"ograve", 210}, {"oline", 8254}, {"omega", 937}, {"omicron", 927},
{"oplus", 8853}, {"or", 8744}, {"ordf", 170}, {"ordm", 186}, {"oslash", 216}, {"otilde", 213},
{"otimes", 8855}, {"ouml", 214}, {"para", 182}, {"part", 8706}, {"permil", 8240}, {"perp", 8869},
{"phi", 934}, {"pi", 928}, {"piv", 982}, {"plusmn", 177}, {"pound", 163}, {"prime", 8242},
{"prod", 8719}, {"prop", 8733}, {"psi", 936}, {"quot", 34}, {"radic", 8730}, {"rang", 9002},
{"raquo", 187}, {"rarr", 8594}, {"rceil", 8969}, {"rdquo", 8221}, {"real", 8476}, {"reg", 174},
{"rfloor", 8971}, {"rho", 929}, {"rlm", 8207}, {"rsaquo", 8250}, {"rsquo", 8217}, {"sbquo", 8218},
{"scaron", 352}, {"sdot", 8901}, {"sect", 167}, {"shy", 173}, {"sigma", 931}, {"sigmaf", 962},
{"sim", 8764}, {"spades", 9824}, {"sub", 8834}, {"sube", 8838}, {"sum", 8721}, {"sup", 8835}, {"sup1", 185},
{"sup2", 178}, {"sup3", 179}, {"sup", 8835}, {"supe", 8839}, {"szlig", 223}, {"tau", 932},
{"there4", 8756}, {"theta", 920}, {"thetasym", 977}, {"thinsp", 8201}, {"thorn", 222}, {"tilde", 732},
{"times", 215}, {"trade", 8482}, {"uacute", 218}, {"uarr", 8593}, {"ucirc", 219}, {"ugrave", 217},
{"uml", 168}, {"upsih", 978}, {"upsilon", 933}, {"uuml", 220}, {"weierp", 8472}, {"xi", 926},
{"yacute", 221}, {"yen", 165}, {"yuml", 255}, {"zeta", 918}, {"zwj", 8205}, {"zwnj", 8204}};

int	_id=0;

//static inline void add_to_summary( char *str, yyscan_t scanner )
void add_to_summary( char *str, yyscan_t scanner )
{
    struct bhpm_yy_extra	*he = bhpmget_extra(scanner);

    if (he->invisible_text) return;
//    if (_id<25)
//    printf("%s (%i,%i,%i,%i,%i,%i,%i)\n", str, he->newhead, he->newdiv, he->newspan, he->inhead, he->indiv, he->inspan, he->space);

    if (he->title)
	{
	    if (he->title_nr==1)
		{
		    if (he->Btitle->pos > 0 && he->space)
			{
			    bprintf(he->Btitle, " ");
			}

    		    he->space = 0;

		    bprintf(he->Btitle, "%s", str);
		}
        }
    else
        {
	    if (he->inlink && ((!he->alink) || (he->nlink)))
		{
		    bprintf(he->Bbody, "</link>");
		    he->inlink = 0;
		}

	    if (he->newdiv)
	        {
//		    if (he->inlink) { bprintf(he->Bbody, "</link>"); he->inlink = 0; }
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
		    if (he->indiv) bprintf(he->Bbody, "</div>\n");
		    bprintf(he->Bbody, "<div>\n");
		    if (he->newhead)
			{
			    bprintf(he->Bbody, "  <h2>\n    <span>");
			    he->inhead = 1;
			}
		    else
			{
			    bprintf(he->Bbody, "    <span>");
			    he->inhead = 0;
			    he->h = 0;
			}

		    he->inspan = 1;
		    he->indiv = 1;
//		    if (he->alink) { bprintf(he->Bbody, "<link>"); he->inlink = 1; }
		}
	    else if (he->newhead)
	        {
		    // Denne viser kun headere deklarert med h[1-6]-tags, de andre ignorerer vi.
//		    if (he->inlink) { bprintf(he->Bbody, "</link>"); he->inlink = 0; }
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
		    bprintf(he->Bbody, "  <h2>\n    <span>");
		    he->inspan = 1;
		    he->inhead = 1;
//		    if (he->alink) { bprintf(he->Bbody, "<link>"); he->inlink = 1; }
		}
	    else if (he->newendhead)
		{
//		    if (he->inlink) { bprintf(he->Bbody, "</link>"); he->inlink = 0; }
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
		    bprintf(he->Bbody, "    <span>");
		    he->inspan = 1;
		    he->inhead = 0;
		    he->h = 0;
//		    if (he->alink) { bprintf(he->Bbody, "<link>"); he->inlink = 1; }
		}
	    else if (he->newspan)
	        {
//		    if (he->inlink) { bprintf(he->Bbody, "</link>"); he->inlink = 0; }
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    bprintf(he->Bbody, "    <span>");
		    he->inspan = 1;
//		    if (he->alink) { bprintf(he->Bbody, "<link>"); he->inlink = 1; }
		}
//	    else if (he->alink && !he->inlink)
//		{
//		    bprintf(he->Bbody, " <link>");
//		    he->inlink = 1;
//		}
//	    else if (!he->alink && he->inlink)
//		{
//		    bprintf(he->Bbody, "</link> ");
//		    he->inlink = 0;
//		}
	    else if (he->space)
	        {
		    bprintf(he->Bbody, " ");
		}

	    he->space = 0;
	    he->newhead = 0;
	    he->newdiv = 0;
	    he->newspan = 0;
	    he->newendhead = 0;

	    if (he->nlink)
		{
		    bprintf(he->Bbody, "<link>");
		    he->nlink = 0;
		    he->inlink = 1;
		}

	    bprintf(he->Bbody, "%s", str);
	}
}


static inline void send_word( char *str, yyscan_t scanner )
{
    // Fjern alle ' som pre/postfix (skal kun være infix)
    while (str[0]=='\'') str++;

    int		end = strlen(str)-1;
    while (end>=0 && str[end]=='\'')
	{
	    str[end] = '\0';
	    end--;
	}

    if (utf8_strlen((utf8_byte*)str) < 1) return;  // One char minimum

    utf8_strtolower((utf8_byte*)str);

    struct bhpm_yy_extra	*he = bhpmget_extra(scanner);

    if (he->alink)
        {
//	    he->user_fn( bhpm_translate((char*)$1), he->wordcount++, pu_linkword, puf_none, he->wordlist );	// [linkword]
	    if (he->invisible_text)
		he->user_fn( str, he->wordcount++, pu_cloaked_linkword, puf_none, he->wordlist );	// [linkword]
	    else
		he->user_fn( str, he->wordcount++, pu_linkword, puf_none, he->wordlist );	// [linkword]
	}
    else
        {
	    enum parsed_unit_flag	puf = puf_none;
	    if (he->title) puf = puf_title;
	    else if (he->h==1) puf = puf_h1;
	    else if (he->h==2) puf = puf_h2;
	    else if (he->h==3) puf = puf_h3;
	    else if (he->h==4) puf = puf_h4;
	    else if (he->h==5) puf = puf_h5;
	    else if (he->h==6) puf = puf_h6;
//	    he->user_fn( bhpm_translate((char*)$1), he->wordcount++, pu_word, puf, he->wordlist );	// new word

	    if (he->invisible_text)
		he->user_fn( str, he->wordcount++, pu_cloaked_word, puf, he->wordlist );	// new word
	    else
		he->user_fn( str, he->wordcount++, pu_word, puf, he->wordlist );	// new word
	}
}



static inline void bhpm_newstring( char *str, yyscan_t scanner, int tt )
{
    struct bhpm_yy_extra	*he = bhpmget_extra( scanner );

//    printf("(%s)", str); fflush(stdout);

    if (he->tt == WORD && (tt != WORD || he->space == 1 || he->flush))
	{
//	    printf("\033[1;33mword: %s\033[0m\n", he->stringcircle[he->stringtop]);
	    send_word( he->stringcircle[he->stringtop], scanner );
	}

    if ((tt == he->tt && he->space == 0 && (tt == WORD || tt == ESC)) || (tt==TEXTFIELD && he->flush==0))
	{
	    strncpy( &(he->stringcircle[he->stringtop][he->slen]), str, maxNewString - he->slen );
	    he->slen = strlen( he->stringcircle[he->stringtop] );
	}
    else
	{
	    he->tt = tt;
	    he->stringtop++;
	    if (he->stringtop >= 32) he->stringtop = 0;

	    #ifdef DEBUG
		//Runar
		// Bruker statisk minne, ikke dynamisk, da vi en skjelden gang får segfeil i free
		// desuten er det raskere, ved at vi ikke gjør systemkallet malloc heletiden
	        //har fast lengde på stringene. Hvis den er for lang vil ikke alt bli kopiert over.
		// Ax: Strenger lengre enn 2k er uansett søppel som ikke bør vises i sammendraget.
	        if (strlen(str) > maxNewString) {
	                printf("New string is to long, will be truncated. Length was %i\n",strlen(str));
	        }
	    #endif

	    strncpy( he->stringcircle[he->stringtop], str, maxNewString );
	    he->stringcircle[he->stringtop][maxNewString] = '\0';		// I tilfelle 'str' er lengre enn maxlengden.

	    he->slen = strlen( he->stringcircle[he->stringtop] );
	    *bhpmget_lval(scanner) = (int)he->stringcircle[he->stringtop];
	}

    if (tt == WORD || tt == ESC)
	{
	    add_to_summary( str, scanner );
	}

    he->flush = 0;
}


/*
 * Konverterer val (iso10646/unicode nummerverdi for karakter) til utf-8:
 */
static inline void bhpm_newutf8( int val, yyscan_t scanner, int tt )
{
    struct bhpm_yy_extra	*he = bhpmget_extra( scanner );
    int				old_slen;

//    printf("\033[1;33m%i\033[0m\n", val);

    if (he->tt == WORD && (tt != WORD || he->space == 1 || he->flush))
	{
//	    printf("\033[1;33mword: %s\033[0m\n", he->stringcircle[he->stringtop]);
	    send_word( he->stringcircle[he->stringtop], scanner );
	}

    if (!((tt == he->tt && he->space == 0) || (tt == TEXTFIELD && he->flush == 0)))
	{
	    he->stringtop++;
	    if (he->stringtop >= 32) he->stringtop = 0;
	    he->slen = 0;
	    he->tt = tt;
	    *bhpmget_lval(scanner) = (int)he->stringcircle[he->stringtop];
	}
    else if (he->slen >= maxNewString - 5)
	return;

    old_slen = he->slen;

    if (val < 128)
	{
	    he->stringcircle[he->stringtop][he->slen++] = (char)val;
	}
    else if (val < 2048)
	{
	    he->stringcircle[he->stringtop][he->slen++] = (char)(192 + ((val>>6) & 0x1f));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + (val & 0x3f));
	}
    else if (val < 65536)
	{
	    he->stringcircle[he->stringtop][he->slen++] = (char)(224 + ((val>>12) & 0xf));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + ((val>>6) & 0x3f));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + (val & 0x3f));
	}
    else
	{
	    he->stringcircle[he->stringtop][he->slen++] = (char)(240 + ((val>>18) & 0x7));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + ((val>>12) & 0x3f));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + ((val>>6) & 0x3f));
	    he->stringcircle[he->stringtop][he->slen++] = (char)(128 + (val & 0x3f));
	}

    he->stringcircle[he->stringtop][he->slen] = '\0';

    if (tt == WORD || tt == ESC)
	{
//	    add_to_summary( "\033[1;33m", scanner );
	    add_to_summary( &(he->stringcircle[he->stringtop][old_slen]), scanner );
//	    add_to_summary( "\033[0m", scanner );
	}
}


static inline int bhpm_htmlesc_val_compare( const void *a, const void *b )
{
//    printf("%s %s\n", (char*)a, ((struct htmlesc_val*)b)->str );
    return strncmp( (char*)a, ((struct htmlesc_val*)b)->str, strlen(((struct htmlesc_val*)b)->str) );
}


static inline int bhpm_htmlesc_val_casecompare( const void *a, const void *b )
{
//    printf("%s %s\n", (char*)a, ((struct htmlesc_val*)b)->str );
    return strncasecmp( (char*)a, ((struct htmlesc_val*)b)->str, strlen(((struct htmlesc_val*)b)->str) );
}


static inline void bhpm_new_esc_to_utf8( const char* str, yyscan_t scanner, int tt )
{
    struct htmlesc_val	*escape;

    escape = bsearch( &(str[1]), bhpm_heval, 252, sizeof(struct htmlesc_val), bhpm_htmlesc_val_compare );

    if (escape == NULL)
	{
	    // Dette skjer kun hvis escapen ikke har rett på store eller små bokstaver.
	    escape = bsearch( &(str[1]), bhpm_hecval, 191, sizeof(struct htmlesc_val), bhpm_htmlesc_val_casecompare );
	}

    // escape skal ikke kunne være null, ettersom str allerede er verifisert å skulle eksistere.
    bhpm_newutf8( escape->val, scanner, tt );
}


/*
 *  Sjekker om unicode-verdi er bokstav, og sender videre til utf-8-funksjonen.
 */
static inline void bhpm_newunicode( int val, yyscan_t scanner )
{
    if (U_isletter(val))
	{
	    bhpm_newutf8( val, scanner, WORD );
	}
    else
	{
	    bhpm_newutf8( val, scanner, ESC );
	}
}

