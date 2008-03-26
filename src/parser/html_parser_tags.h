// Generert av tags.awk, (C) 2007 Boitho AS, Magnus Galåen.
// Forandringer til denne fila vil bli overskrevet, se tags.conf og tags.awk istedet.

#define tagf_space	1
#define tagf_head	2
#define tagf_div	4
#define tagf_span	8

char*		tags[] = {"a", "base", "h1", "h2", "h3", "h4", "h5", "h6", "meta", "title", "div", "p", "table", "ol", "ul", "br", "li", "td", "th", "tr", "button", "center", "hr", "img", "label", "map"};
enum		{tag_a=0, tag_base, tag_h1, tag_h2, tag_h3, tag_h4, tag_h5, tag_h6, tag_meta, tag_title, tag_div, tag_p, tag_table, tag_ol, tag_ul, tag_br, tag_li, tag_td, tag_th, tag_tr, tag_button, tag_center, tag_hr, tag_img, tag_label, tag_map};
const int	tag_flags[] = {0, 0, tagf_space|tagf_head, tagf_space|tagf_head, tagf_space|tagf_head, tagf_space|tagf_head, tagf_space|tagf_head, tagf_space|tagf_head, 0, 0, tagf_space|tagf_div, tagf_space|tagf_div, tagf_space|tagf_div, tagf_space|tagf_div, tagf_space|tagf_div, tagf_space, tagf_space|tagf_span, tagf_space|tagf_span, tagf_space|tagf_span, tagf_space|tagf_span, tagf_space, tagf_space, tagf_space, tagf_space, tagf_space, tagf_space};
const int	tags_size = 26;
automaton	*tags_automaton = NULL;
