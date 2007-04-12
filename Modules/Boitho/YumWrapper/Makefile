CC = gcc
CCFLAGS = -Wall -g -c

yumwrapper: yumwrapper.o ../../../../websearch/src/common/exeoc.c
	$(CC) $(LDFLAGS) $^ -o $@

yumwrapper.o: yumwrapper.c 
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -rf *.o yumwrapper
