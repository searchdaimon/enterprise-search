
CC=/usr/local/bin/gcc -O2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE

all: build_index search_index

build_index:
	$(CC) -o build_index build_index.c

search_index:
	$(CC) -c search_index.c

test:
	$(CC) -o url2docid test.c search_index.o ../common/sha1.o

clean:
	rm -rf test search_index.o build_index

.PHONY:	all clean test
