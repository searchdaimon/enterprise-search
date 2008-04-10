CC = gcc
CCFLAGS = -Wall -g -c -D_FILE_OFFSET_BITS=64

mergeLinkDB: main.o ../common/define.h
	$(CC) $(LDFLAGS) $^ -o $@

main.o: main.c 
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -rf *.o mergeLinkDB
