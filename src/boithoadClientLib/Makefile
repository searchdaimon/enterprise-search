# The compiler to be used
CC = gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS = -fPIC

All: boithoadClientLib

boithoadClientLib: boithoadClientLib.c
	$(CC) -c $(CFLAGS) boithoadClientLib.c daemon.c
	ar rc liboithoaut.a boithoadClientLib.o daemon.o
	ranlib liboithoaut.a

