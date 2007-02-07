
#ifndef _HTML_PARSER_H_
#define _HTML_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif


enum parsed_unit
{
    pu_word, pu_link, pu_baselink, pu_linkword, pu_meta_keywords, pu_meta_author, pu_meta_description, pu_meta_redirect
};

enum parsed_unit_flag
{
    puf_none, puf_title, puf_h1, puf_h2, puf_h3, puf_h4, puf_h5, puf_h6
};

void html_parser_init();
void html_parser_exit();
void html_parser_run( char *url, char text[], int textsize, char **output_title, char **output_body, void (*fn)(char*,int,enum parsed_unit,enum parsed_unit_flag,void* wordlist), void* wordlist );


#ifdef __cplusplus
}
#endif

#endif	// _HTML_PARSER_H_
