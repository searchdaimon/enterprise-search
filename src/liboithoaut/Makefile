# The compiler to be used
CC = gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS =

All: boithoadClientLib

boithoadClientLib: ../boithoadClientLib/boithoadClientLib.c
	$(CC) -c $(CFLAGS) ../boithoadClientLib/boithoadClientLib.c ../common/daemon.c
	ar rc liboithoaut.a boithoadClientLib.o daemon.o
	ranlib liboithoaut.a

