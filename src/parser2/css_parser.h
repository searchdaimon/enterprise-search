
#ifndef CSS_PARSER_H
#define CSS_PARSER_H


#include "../ds/dcontainer.h"

//#define SHOWRULES
//#define DEBUG
//#define DEBUG_2
//#define FULL_PARSING



enum css_unit	{ css_tag, css_class, css_id, css_descendant, css_child, css_sibling };

typedef struct
{
    enum css_unit	type;
    char		*str;
} css_token;


typedef struct
{
    int			ruleno;
#ifdef FULL_PARSING
    int			decl_start, decl_stop;
#endif
    int			color, background, fontsize;
    char		hidden;
    container		*pattern;
} css_selector;


typedef struct
{
    container		*selectors, *tag_selectors, *class_selectors, *id_selectors;
} css_selector_block;


// Viktig: Kjør denne på returverdien fra css_parser_run for å frigi minne.
void destroy_selectors(css_selector_block *selector_block);

void css_parser_init();
css_selector_block* css_parser_run( char *text, int textsize );
void css_parser_exit();

int parse_color( char *color );

#endif	// CSS_PARSER_H
