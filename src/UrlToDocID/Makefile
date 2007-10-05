
CC=/usr/local/bin/gcc -O2 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE

all: build_index search_index test

build_index:
	$(CC) -o url2docid_build_index build_index.c

search_index:
	$(CC) -c search_index.c

bin_sok:
	$(CC) -o bin_sok bin_sok.c ../common/sha1.c

test:
	$(CC) -o url2docid test.c search_index.o ../common/sha1.c

clean:
	rm -rf test search_index.o url2docid_build_index url2docid

.PHONY:	all clean test bin_sok
