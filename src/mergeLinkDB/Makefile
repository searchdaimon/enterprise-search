CC = gcc
CCFLAGS = -Wall -g -c

mergeLinkDB: main.o ../common/define.h
	$(CC) $(LDFLAGS) $^ -o $@

main.o: main.c main.h
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -rf *.o mergeLinkDB
