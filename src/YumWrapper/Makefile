CC = gcc
CCFLAGS = -Wall -g -c

yumwrapper: yumwrapper.o ../common/exeoc.c
	$(CC) $(LDFLAGS) $^ -o $@

yumwrapper.o: yumwrapper.c 
	$(CC) $(CCFLAGS) -o $@ $<

clean:
	rm -rf *.o yumwrapper
