# The compiler to be used
CC = gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS = -fPIC -g -Wall -O



tester: hashtable.o tester.o hashtable_itr.o
	$(CC) $(CFLAGS) -lm -o tester hashtable.o hashtable_itr.o tester.o

all: tester old_tester

tester.o:	tester.c
	$(CC) $(CFLAGS) -c tester.c -o tester.o

old_tester: hashtable_powers.o tester.o hashtable_itr.o
	$(CC) $(CFLAGS) -o old_tester hashtable_powers.o hashtable_itr.o tester.o

hashtable_powers.o:	hashtable_powers.c
	$(CC) $(CFLAGS) -c hashtable_powers.c -o hashtable_powers.o

hashtable.o:	hashtable.c
	$(CC) $(CFLAGS) -c hashtable.c -o hashtable.o

hashtable_itr.o: hashtable_itr.c
	$(CC) $(CFLAGS) -c hashtable_itr.c -o hashtable_itr.o

tidy:
	rm *.o

clean: tidy
	rm -f tester old_tester
