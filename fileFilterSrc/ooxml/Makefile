
CC=gcc -O2 -Wall


all: wash_tags get_rel html2text

wash_tags:
	flex -8 -f -Pwash_tags -i wash_tags.lex
	$(CC) -o wash_tags lex.wash_tags.c

get_rel:
	flex -8 -f -Pget_rel -i get_rel.lex
	$(CC) -o get_rel lex.get_rel.c

html2text:
	flex -8 -f -Phtml2text -i html2text.lex
	$(CC) -o html2text lex.html2text.c

clean:
	rm -rf wash_tags get_rel html2text lex.wash_tags.c lex.get_rel.c lex.html2text.c

.PHONY: wash_tags get_rel html2text
