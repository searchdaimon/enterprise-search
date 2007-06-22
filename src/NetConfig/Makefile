CC = /usr/local/bin/gcc
CCFLAGS = -Wall -g -c

configwrite: configwrite.o ../common/exeoc.c
	$(CC) $(LDFLAGS) $^ -o $@

configwrite.o: configwrite.c 
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -rf *.o configwrite
