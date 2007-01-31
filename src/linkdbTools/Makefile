
all: mergeLinkdb sortLinkdb indexLinkdb searchIndexLinkdb searchIndexLinkdbIP


mergeLinkdb: mergeLinkdb.c
		gcc mergeLinkdb.c -o mergeLinkdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

sortLinkdb: sortLinkdb.c
		gcc sortLinkdb.c -o sortLinkdb -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

indexLinkdb: indexLinkdb.c
		gcc indexLinkdb.c -o indexLinkdb ../common/lot.c -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lm

searchIndexLinkdb: searchIndexLinkdb.c
		gcc searchIndexLinkdb.c -o searchIndexLinkdb ../common/*.c -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -lm -lz

searchIndexLinkdbIP: searchIndexLinkdbIP.c
		gcc searchIndexLinkdbIP.c chtbl.c list.c -o searchIndexLinkdbIP -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64
