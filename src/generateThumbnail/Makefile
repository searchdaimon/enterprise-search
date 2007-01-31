WC=~eirik/.root/bin/Wand-config
CC=gcc -g
CFLAGS=`$(WC) --cflags`
INCLUDE=-I/home/eirik/.root/include
LIBS=`$(WC) --ldflags --libs`

all:
	$(CC) $(CFLAGS) -c generate_thumbnail.c $(INCLUDE)
	$(CC) -o genthumb generate_thumbnail.o test.c $(LIBS)
