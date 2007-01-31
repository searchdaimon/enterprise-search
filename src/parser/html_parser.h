
#ifndef _HTML_PARSER_H_
#define _HTML_PARSER_H_

enum parsed_unit
{
    pu_word, pu_link, pu_baselink, pu_linkword, pu_meta_keywords, pu_meta_author, pu_meta_description
};

enum parsed_unit_flag
{
    puf_none, puf_title, puf_h1, puf_h2, puf_h3, puf_h4, puf_h5, puf_h6
};

void run_html_parser( char *url, char text[], int text_size, void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag) );

#endif	// _HTML_PARSER_H_
