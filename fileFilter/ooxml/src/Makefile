
CC=gcc -O2 -Wall


all: wash_tags get_rel

wash_tags:
	flex -8 -f -Pwash_tags -i wash_tags.lex
	$(CC) -o wash_tags lex.wash_tags.c

get_rel:
	flex -8 -f -Pget_rel -i get_rel.lex
	$(CC) -o get_rel lex.get_rel.c

clean:
	rm -rf wash_tags get_rel lex.wash_tags.c lex.get_rel.c

.PHONY: wash_tags get_rel
