// Generert av tags.awk, (C) 2007 Boitho AS, Magnus Galåen.
// Forandringer til denne fila vil bli overskrevet, se tags.conf og tags.awk istedet.


#ifndef _HTML_TAGS_H_
#define _HTML_TAGS_H_


#define tagf_space	1
#define tagf_head	2
#define tagf_div	4
#define tagf_span	8
#define tagf_block	16
#define tagf_inline	32
#define tagf_empty	64

char*		tags[] = {"body", "head", "html", "frameset", "style", "title", "area", "base", "col", "frame", "link", "meta", "param", "basefont", "br", "hr", "img", "input", "isindex", "address", "blockquote", "center", "dir", "div", "dl", "fieldset", "form", "h1", "h2", "h3", "h4", "h5", "h6", "menu", "ol", "p", "pre", "table", "ul", "a", "abbr", "acronym", "applet", "b", "bdo", "big", "button", "cite", "code", "dfn", "em", "font", "i", "iframe", "kbd", "label", "map", "object", "q", "s", "samp", "script", "select", "small", "span", "strike", "strong", "sub", "sup", "textarea", "tt", "u", "var", "caption", "colgroup", "dd", "dt", "li", "optgroup", "option", "tbody", "td", "tfoot", "th", "thead", "tr", "del", "ins", "legend"};
enum		{tag_body=0, tag_head, tag_html, tag_frameset, tag_style, tag_title, tag_area, tag_base, tag_col, tag_frame, tag_link, tag_meta, tag_param, tag_basefont, tag_br, tag_hr, tag_img, tag_input, tag_isindex, tag_address, tag_blockquote, tag_center, tag_dir, tag_div, tag_dl, tag_fieldset, tag_form, tag_h1, tag_h2, tag_h3, tag_h4, tag_h5, tag_h6, tag_menu, tag_ol, tag_p, tag_pre, tag_table, tag_ul, tag_a, tag_abbr, tag_acronym, tag_applet, tag_b, tag_bdo, tag_big, tag_button, tag_cite, tag_code, tag_dfn, tag_em, tag_font, tag_i, tag_iframe, tag_kbd, tag_label, tag_map, tag_object, tag_q, tag_s, tag_samp, tag_script, tag_select, tag_small, tag_span, tag_strike, tag_strong, tag_sub, tag_sup, tag_textarea, tag_tt, tag_u, tag_var, tag_caption, tag_colgroup, tag_dd, tag_dt, tag_li, tag_optgroup, tag_option, tag_tbody, tag_td, tag_tfoot, tag_th, tag_thead, tag_tr, tag_del, tag_ins, tag_legend};
const int	tag_flags[] = {tagf_block, tagf_block, tagf_block, 0, 0, 0, tagf_empty, tagf_empty, tagf_empty, tagf_empty, tagf_empty, tagf_empty, tagf_empty, tagf_empty, tagf_empty|tagf_space, tagf_empty|tagf_space, tagf_empty|tagf_space, tagf_space, tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_head|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_block|tagf_div|tagf_space, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline|tagf_space, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline|tagf_space, tagf_inline|tagf_space, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_inline, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_block|tagf_span|tagf_space, tagf_inline, tagf_inline, tagf_inline};
const int	tags_size = 89;
automaton	*tags_automaton = NULL;


#endif	// _HTML_TAGS_H_
