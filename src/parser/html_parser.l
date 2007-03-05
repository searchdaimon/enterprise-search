%{
#include "html_parser_common.h"
#include "y.tab.h"
#include "../common/search_automaton.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * Denne lexical analyzeren støtter Latin-1, Latin-1 supplement, utf-8, samt character entities (html escapes)
 * i alle dokumenter. Den ser ikke på hva som er definert character encoding, men antar at en eller flere av
 * disse brukes. Alt (med unntak av quot,amp,lt og gt) oversettes til utf-8. Vi har dog følgende mangler (todo):
 *
 * - Støtter ikke hexadecimale character entities "&#(x|X)[0-9a-fA-F]+;"
 * - Ingen støtte for andre charsets.
 * - Hva er tegn og hva er bokstaver? Akkurat nå er det litt ymse. Bør ryddes opp i.
 *
 * Se forøvrig http://www.w3.org/TR/html4/charset.html
 *
 * (C) Boitho AS 2004-2007, Magnus Galåen (magnusga@idi.ntnu.no)
 */

static inline void bhpm_newstring( char* str, yyscan_t scanner, int tt );
static inline void bhpm_newutf8( int val, yyscan_t scanner, int tt );
static inline void bhpm_new_esc_to_utf8( const char* str, yyscan_t scanner, int tt );

%}

character	[a-z]
legalchar	[a-z0-9'`]
attrchar	[a-z0-9\-_]
legaltegn	[!"#$%()*+,-./:;=?@[\\\]^{|}~]
tfchars		[a-z0-9_`\-!#$%()*+,-./:;=?@[\]^{|}~]
word		[0-9a-z'ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþ]
allemuligetegn	[!"#$%'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþ]
url		[0-9a-z&-.:/\\?=]
blank		[ \t\n_]
tillatesc	(aacute|acirc|aelig|agrave|aring|atilde|auml|ccedil|eacute|ecirc|egrave|eth|euml|iacute|icirc|igrave|iuml|ntilde|oacute|ocirc|ograve|oslash|otilde|ouml|sup1|sup2|sup3|szlig|thorn|uacute|ucirc|ugrave|uuml|yacute|yuml)
tillatnumesc	(#178|#179|#185|#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255)
nestenalleesc	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
letter		[A-Za-z0-9_'´ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
  /* letteresc	(AElig|Aacute|Acirc|Agrave|Aring|Atilde|Auml|Ccedil|ETH|Eacute|Ecirc|Egrave|Euml|Iacute|Icirc|Igrave|Iuml|Ntilde|Oacute|Ocirc|Ograve|Oslash|Otilde|Ouml|THORN|Uacute|Ucirc|Ugrave|Uuml|Yacute|aacute|acirc|aelig|agrave|aring|atilde|auml|ccedil|eacute|ecirc|egrave|eth|euml|iacute|icirc|igrave|iuml|ntilde|oacute|ocirc|ograve|oslash|otilde|ouml|szlig|thorn|uacute|ucirc|ugrave|uuml|yacute|yuml) */
  /* letternumesc	(#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255) */
ikkeletter	[!"#$%()*+,-./:;=?@[\]^{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿×÷]
ikkeletteresc	(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|times|divide|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
egentligesc	[²³¹ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
latin-sup	[ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ]
alletegnutenesc	[\\!"#$%'()*+,\-\./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±´µ¶·¸º»¼½¾¿×÷]
    /* Alle tegn uten html-escapes og utf-8 flerbytekoder: */
nestenalletegn	[\\!"#$%'()*+,\-\./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~¡¢£¤¥¦§¨©ª«¬­®¯°±´µ¶·¸º»¼½¾¿]
    /* Det er ingen html-escapes som ikke er med i disse (252 tilsammen): */
allenumesc	(#161|#162|#163|#164|#165|#166|#167|#168|#169|#170|#171|#172|#173|#174|#175|#176|#177|#178|#179|#180|#181|#182|#183|#184|#185|#186|#187|#188|#189|#190|#191|#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#215|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#247|#248|#249|#250|#251|#252|#253|#254|#255|#402|#913|#914|#915|#916|#917|#918|#919|#920|#921|#922|#923|#924|#925|#926|#927|#928|#929|#931|#932|#933|#934|#935|#936|#937|#945|#946|#947|#948|#949|#950|#951|#952|#953|#954|#955|#956|#957|#958|#959|#960|#961|#962|#963|#964|#965|#966|#967|#968|#969|#977|#978|#982|#8226|#8230|#8242|#8243|#8254|#8260|#8472|#8465|#8476|#8482|#8501|#8592|#8593|#8594|#8595|#8596|#8629|#8656|#8657|#8658|#8659|#8660|#8704|#8706|#8707|#8709|#8711|#8712|#8713|#8715|#8719|#8721|#8722|#8727|#8730|#8733|#8734|#8736|#8743|#8744|#8745|#8746|#8747|#8756|#8764|#8773|#8776|#8800|#8801|#8804|#8805|#8834|#8835|#8836|#8838|#8839|#8853|#8855|#8869|#8901|#8968|#8969|#8970|#8971|#9001|#9002|#9674|#9824|#9827|#9829|#9830|#34|#38|#60|#62|#338|#339|#352|#353|#376|#710|#732|#8194|#8195|#8201|#8204|#8205|#8206|#8207|#8211|#8212|#8216|#8217|#8218|#8220|#8221|#8222|#8224|#8225|#8240|#8249|#8250|#8364)
alleesc		(nbsp|iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
alleesc_minus_nbsp (iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|times|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|divide|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
letteresc	(Agrave|Aacute|Acirc|Atilde|Auml|Aring|AElig|Ccedil|Egrave|Eacute|Ecirc|Euml|Igrave|Iacute|Icirc|Iuml|ETH|Ntilde|Ograve|Oacute|Ocirc|Otilde|Ouml|Oslash|Ugrave|Uacute|Ucirc|Uuml|Yacute|THORN|szlig|agrave|aacute|acirc|atilde|auml|aring|aelig|ccedil|egrave|eacute|ecirc|euml|igrave|iacute|icirc|iuml|eth|ntilde|ograve|oacute|ocirc|otilde|ouml|oslash|ugrave|uacute|ucirc|uuml|yacute|thorn|yuml)
letternumesc	(#192|#193|#194|#195|#196|#197|#198|#199|#200|#201|#202|#203|#204|#205|#206|#207|#208|#209|#210|#211|#212|#213|#214|#216|#217|#218|#219|#220|#221|#222|#223|#224|#225|#226|#227|#228|#229|#230|#231|#232|#233|#234|#235|#236|#237|#238|#239|#240|#241|#242|#243|#244|#245|#246|#248|#249|#250|#251|#252|#253|#254|#255)
tegnesc_u_nbsp		(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|times|divide|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|quot|amp|lt|gt|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
tegnnumesc_u_nbsp	(#161|#162|#163|#164|#165|#166|#167|#168|#169|#170|#171|#172|#173|#174|#175|#176|#177|#178|#179|#180|#181|#182|#183|#184|#185|#186|#187|#188|#189|#190|#191|#215|#247|#402|#913|#914|#915|#916|#917|#918|#919|#920|#921|#922|#923|#924|#925|#926|#927|#928|#929|#931|#932|#933|#934|#935|#936|#937|#945|#946|#947|#948|#949|#950|#951|#952|#953|#954|#955|#956|#957|#958|#959|#960|#961|#962|#963|#964|#965|#966|#967|#968|#969|#977|#978|#982|#8226|#8230|#8242|#8243|#8254|#8260|#8472|#8465|#8476|#8482|#8501|#8592|#8593|#8594|#8595|#8596|#8629|#8656|#8657|#8658|#8659|#8660|#8704|#8706|#8707|#8709|#8711|#8712|#8713|#8715|#8719|#8721|#8722|#8727|#8730|#8733|#8734|#8736|#8743|#8744|#8745|#8746|#8747|#8756|#8764|#8773|#8776|#8800|#8801|#8804|#8805|#8834|#8835|#8836|#8838|#8839|#8853|#8855|#8869|#8901|#8968|#8969|#8970|#8971|#9001|#9002|#9674|#9824|#9827|#9829|#9830|#34|#38|#60|#62|#338|#339|#352|#353|#376|#710|#732|#8194|#8195|#8201|#8204|#8205|#8206|#8207|#8211|#8212|#8216|#8217|#8218|#8220|#8221|#8222|#8224|#8225|#8240|#8249|#8250|#8364)
    /* Her har vi fjernet noen html-spesifikke tegn (lt,gt,amp,quot, samt nbsp): */
tegnesc_minus		(iexcl|cent|pound|curren|yen|brvbar|sect|uml|copy|ordf|laquo|not|shy|reg|macr|deg|plusmn|sup2|sup3|acute|micro|para|middot|cedil|sup1|ordm|raquo|frac14|frac12|frac34|iquest|times|divide|fnof|Alpha|Beta|Gamma|Delta|Epsilon|Zeta|Eta|Theta|Iota|Kappa|Lambda|Mu|Nu|Xi|Omicron|Pi|Rho|Sigma|Tau|Upsilon|Phi|Chi|Psi|Omega|alpha|beta|gamma|delta|epsilon|zeta|eta|theta|iota|kappa|lambda|mu|nu|xi|omicron|pi|rho|sigmaf|sigma|tau|upsilon|phi|chi|psi|omega|thetasym|upsih|piv|bull|hellip|prime|Prime|oline|frasl|weierp|image|real|trade|alefsym|larr|uarr|rarr|darr|harr|crarr|lArr|uArr|rArr|dArr|hArr|forall|part|exist|empty|nabla|isin|notin|ni|prod|sum|minus|lowast|radic|prop|infin|ang|and|or|cap|cup|int|there4|sim|cong|asymp|ne|equiv|le|ge|sub|sup|nsub|sube|supe|oplus|otimes|perp|sdot|lceil|rceil|lfloor|rfloor|lang|rang|loz|spades|clubs|hearts|diams|OElig|oelig|Scaron|scaron|Yuml|circ|tilde|ensp|emsp|thinsp|zwnj|zwj|lrm|rlm|ndash|mdash|lsquo|rsquo|sbquo|ldquo|rdquo|bdquo|dagger|Dagger|permil|lsaquo|rsaquo|euro)
tegnnumesc_minus	(#33|#34|#35|#36|#37|#40|#41|#42|#43|#44|#45|#46|#47|#58|#59|#61|#63|#64|#91|#92|#93|#94|#123|#124|#125|#126|#161|#162|#163|#164|#165|#166|#167|#168|#169|#170|#171|#172|#173|#174|#175|#176|#177|#178|#179|#180|#181|#182|#183|#184|#185|#186|#187|#188|#189|#190|#191|#215|#247|#402|#913|#914|#915|#916|#917|#918|#919|#920|#921|#922|#923|#924|#925|#926|#927|#928|#929|#931|#932|#933|#934|#935|#936|#937|#945|#946|#947|#948|#949|#950|#951|#952|#953|#954|#955|#956|#957|#958|#959|#960|#961|#962|#963|#964|#965|#966|#967|#968|#969|#977|#978|#982|#8226|#8230|#8242|#8243|#8254|#8260|#8472|#8465|#8476|#8482|#8501|#8592|#8593|#8594|#8595|#8596|#8629|#8656|#8657|#8658|#8659|#8660|#8704|#8706|#8707|#8709|#8711|#8712|#8713|#8715|#8719|#8721|#8722|#8727|#8730|#8733|#8734|#8736|#8743|#8744|#8745|#8746|#8747|#8756|#8764|#8773|#8776|#8800|#8801|#8804|#8805|#8834|#8835|#8836|#8838|#8839|#8853|#8855|#8869|#8901|#8968|#8969|#8970|#8971|#9001|#9002|#9674|#9824|#9827|#9829|#9830|#338|#339|#352|#353|#376|#710|#732|#8194|#8195|#8201|#8204|#8205|#8206|#8207|#8211|#8212|#8216|#8217|#8218|#8220|#8221|#8222|#8224|#8225|#8240|#8249|#8250|#8364)
utf-8-2b	[\300-\337][\200-\277]
utf-8-3b	[\340-\357][\200-\277][\200-\277]
utf-8-4b	[\360-\367][\200-\277][\200-\277][\200-\277]
%option	nomain noyywrap reentrant bison-bridge
%x	TAG ENDTAG COMMENT SCRIPT STYLE SELECT TEXTAREA TAG_TF
%%
 /* ({allemuligetegn}|\&{nestenalleesc}|\&{nestenalleesc};|\&\#[0-9]+;)+    { bhpm_newstring(yytext, yyscanner); return WORD; } */
 /* Midlertidig fiks for escapes uten avsluttende semikolon: */
 /* ({allemuligetegn}|\&{nestenalleesc};|\&\#[0-9]+;)+    { bhpm_newstring(yytext, yyscanner); return WORD; } */
 /* \&[^; ]*;		{} // Ignore other escapes. */
 /* \&			{ bhpm_newstring("&amp;", yyscanner); return WORD; } // Send ampersands not valid escapes as &amp; */
 /* \303\246		{ bhpm_newstring("&aelig;", yyscanner); return ESC; } */
 /* \303\270		{ bhpm_newstring("&oslash;", yyscanner); return ESC; } */
 /* \303\245		{ bhpm_newstring("&aring;", yyscanner); return ESC; } */
 /* \303\206		{ bhpm_newstring("&AElig;", yyscanner); return ESC; } */
 /* \303\230		{ bhpm_newstring("&Oslash;", yyscanner); return ESC; } */
 /* \303\205		{ bhpm_newstring("&Aring;", yyscanner); return ESC; } */
({utf-8-2b}|{utf-8-3b}|{utf-8-4b})+	{ bhpm_newstring(yytext, yyscanner, WORD); }
\&{letteresc};?		{ bhpm_new_esc_to_utf8(yytext, yyscanner, WORD); }
{legalchar}+		{ bhpm_newstring(yytext, yyscanner, WORD); }
{legaltegn}+		{ bhpm_newstring(yytext, yyscanner, ESC); }
\&{tegnesc_minus};?	{ bhpm_new_esc_to_utf8(yytext, yyscanner, ESC); }
(\&nbsp;?)+		{ (bhpmget_extra(yyscanner))->space = 1; }
(\&#160;?)+		{ (bhpmget_extra(yyscanner))->space = 1; }
    /* quot,amp,lt og gt bør alltid sendes videre som character entities (html-escapes): */
(\&quot;?|\&#34;?)	{ bhpm_newstring("&quot;", yyscanner, ESC); }
(\&amp;?|\&#38;?)	{ bhpm_newstring("&amp;", yyscanner, ESC); }
(\&lt;?|\&#60;?)	{ bhpm_newstring("&lt;", yyscanner, ESC); }
(\&gt;?|\&#62;?)	{ bhpm_newstring("&gt;", yyscanner, ESC); }
    /* (\&quot;?|\&amp;?|\&lt;?|\&gt;?)+	{ bhpm_newstring(yytext, yyscanner, ESC); } */
    /* (\&#34;?|\&#38;?|\&#60;?|\&#62;?)+	{ bhpm_newstring(yytext, yyscanner, ESC); } */
\&{tegnnumesc_minus};?		{ bhpm_newutf8( atoi(&(yytext[2])), yyscanner, ESC ); }
\&#[0-9]+;		{ bhpm_newutf8( atoi(&(yytext[2])), yyscanner, WORD ); }
\&			{ bhpm_newstring("&amp;", yyscanner, ESC); }
[\ \t\n\r\f_]+		{ (bhpmget_extra(yyscanner))->space = 1; }
{latin-sup}		{ bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, WORD ); }
[\200-\377]		{ bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, ESC); }
\<{blank}*script[^\>]*\> {	// Ignore characters between script-tags.
			    BEGIN SCRIPT;
			}
\<{blank}*style[^\>]*\> {	// Ignore characters between style-tags.
			    BEGIN STYLE;
			}
\<{blank}*select[^\>]*\> {	// Ignore characters between select-tags.
			    BEGIN SELECT;
			}
\<{blank}*textarea[^\>]*\> {	// Ignore characters between textarea-tags.
			    BEGIN TEXTAREA;
			}
\<\/			{
			    BEGIN ENDTAG;
			    return ENDTAG_START;
			}
\<			{
			    BEGIN TAG;
			    return TAG_START;
			}
\<\!\-\-		{
			    BEGIN COMMENT;
			}
<TAG>\/\>		{
			    BEGIN INITIAL;
			    return TAG_ENDTAG_STOPP;
			}
<TAG>\>			{
			    BEGIN INITIAL;
			    return TAG_STOPP;
			}
<TAG>\=			{
			    return EQUALS;
			}
<TAG>{attrchar}+	{
			    bhpm_newstring(yytext, yyscanner, ATTR);
			    return ATTR;
			}
<ENDTAG>\>		{
			    BEGIN INITIAL;
			    return ENDTAG_STOPP;
			}
<ENDTAG>{legalchar}+	{
			    bhpm_newstring(yytext, yyscanner, ATTR);
			    return ATTR;
			}
<TAG>\'			{
			    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
			    he->flush = 1;
			    he->field_delimit = '\'';
			    BEGIN TAG_TF;
			}
<TAG>\"			{
			    struct bhpm_yy_extra	*he = bhpmget_extra(yyscanner);
			    he->flush = 1;
			    he->field_delimit = '"';
			    BEGIN TAG_TF;
			}
<TAG_TF>\\\"		{ bhpm_newstring("\"", yyscanner, TEXTFIELD); }
<TAG_TF>\\\'		{ bhpm_newstring("\'", yyscanner, TEXTFIELD); }
<TAG_TF>\"		{
			    if ((bhpmget_extra(yyscanner))->field_delimit == '"')
				{
				    BEGIN TAG;
				    return TEXTFIELD;
				}
			    else bhpm_newstring("\"", yyscanner, TEXTFIELD);
			}
<TAG_TF>\'		{
			    if ((bhpmget_extra(yyscanner))->field_delimit == '\'')
				{
				    BEGIN TAG;
				    return TEXTFIELD;
				}
			    else bhpm_newstring("\'", yyscanner, TEXTFIELD);
			}

<TAG_TF>\\		{}
<TAG_TF>({utf-8-2b}|{utf-8-3b}|{utf-8-4b})+	{ bhpm_newstring(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>\&{letteresc};?		{ bhpm_new_esc_to_utf8(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>{tfchars}+		{ bhpm_newstring(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>\&{tegnesc_minus};?	{ bhpm_new_esc_to_utf8(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>(\&nbsp;?)+		{ bhpm_newstring(" ", yyscanner, TEXTFIELD); }
<TAG_TF>(\&#160;?)+		{ bhpm_newstring(" ", yyscanner, TEXTFIELD); }
<TAG_TF>(\&quot;?|\&amp;?|\&lt;?|\&gt;?)+	{ bhpm_newstring(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>(\&#34;?|\&#38;?|\&#60;?|\&#62;?)+	{ bhpm_newstring(yytext, yyscanner, TEXTFIELD); }
<TAG_TF>\&{tegnnumesc_minus};?		{ bhpm_newutf8( atoi(&(yytext[2])), yyscanner, TEXTFIELD ); }
<TAG_TF>\&#[0-9]+;		{ bhpm_newutf8( atoi(&(yytext[2])), yyscanner, TEXTFIELD ); }
<TAG_TF>\&			{ bhpm_newstring("&amp;", yyscanner, TEXTFIELD); }
<TAG_TF>[\ \t\n\r\f]+		{ bhpm_newstring(" ", yyscanner, TEXTFIELD); }
<TAG_TF>{latin-sup}		{ bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, TEXTFIELD ); }
<TAG_TF>[\200-\377]		{ bhpm_newutf8( (int)((unsigned char)yytext[0]), yyscanner, TEXTFIELD); }

<COMMENT>\-\-\>		{			BEGIN INITIAL; }
<SCRIPT>\<\/{blank}*script[^\>]*\>	{	BEGIN INITIAL; }
<STYLE>\<\/{blank}*style[^\>]*\>	{	BEGIN INITIAL; }
<SELECT>\<\/{blank}*select[^\>]*\>	{	BEGIN INITIAL; }
<TEXTAREA>\<\/{blank}*textarea[^\>]*\>	{	BEGIN INITIAL; }
    /* Framtving at ord-bufferet tømmes: */
<*><<EOF>>		{ bhpm_newstring("", yyscanner, -1); yyterminate(); }
.|\n			{ (bhpmget_extra(yyscanner))->space = 1; }
<*>.|\n			{}
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

static inline void add_to_summary( char *str, yyscan_t scanner )
{
    struct bhpm_yy_extra	*he = bhpmget_extra(scanner);

//    if (_id<25)
//    printf("(%i,%i,%i,%i,%i,%i,%i)\n", he->newhead, he->newdiv, he->newspan, he->inhead, he->indiv, he->inspan, he->space);

    if (he->title)
	{
	    if (he->Btitle->pos > 0 && he->space)
		{
		    bprintf(he->Btitle, " ");
		}

    	    he->space = 0;

	    bprintf(he->Btitle, "%s", str);
        }
    else
        {
	    if (he->newdiv)
	        {
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
		}
	    else if (he->newhead)
	        {
		    // Denne viser kun headere deklarert med h[1-6]-tags, de andre ignorerer vi.
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
		    bprintf(he->Bbody, "  <h2>\n    <span>");
		    he->inspan = 1;
		    he->inhead = 1;
		}
	    else if (he->newendhead)
		{
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    if (he->inhead) bprintf(he->Bbody, "  </h2>\n");
		    bprintf(he->Bbody, "    <span>");
		    he->inspan = 1;
		    he->inhead = 0;
		    he->h = 0;
		}
	    else if (he->newspan)
	        {
		    if (he->inspan) bprintf(he->Bbody, "</span>\n");
		    bprintf(he->Bbody, "    <span>");
		    he->inspan = 1;
		}
	    else if (he->space)
	        {
		    bprintf(he->Bbody, " ");
		}

	    he->space = 0;
	    he->newhead = 0;
	    he->newdiv = 0;
	    he->newspan = 0;
	    he->newendhead = 0;

	    bprintf(he->Bbody, "%s", str);
	}
}


static inline void send_word( char *str, yyscan_t scanner )
{
    if (strlen(str) < 2) return;

    struct bhpm_yy_extra	*he = bhpmget_extra(scanner);

    if (he->alink)
        {
//	    he->user_fn( bhpm_translate((char*)$1), he->wordcount++, pu_linkword, puf_none, he->wordlist );	// [linkword]
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
	    he->user_fn( str, he->wordcount++, pu_word, puf, he->wordlist );	// new word
	}
}



static inline void bhpm_newstring( char *str, yyscan_t scanner, int tt )
{
    struct bhpm_yy_extra	*he = bhpmget_extra( scanner );


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
