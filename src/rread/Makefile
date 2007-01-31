# Makefile

# The compiler to be used
CC = gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS =

# The dynamic libraries that the executable needs to be linked to
LDFLAGS = -lm -lz


#pop rank bibloteket
LIBS = ../common/lot.c ../common/reposetory.c

# The Dependency Rules
# They take the form
# target : dependency1 dependency2...
#        Command(s) to generate target from dependencies

# Dummy target that is processed by default. It specifies al list of
# other targets that are processed if make is invoked with no arguments
# However if you invoke make as "make output-data", it will only try to 
# generate the file output-data and its dependencies, not plot.png 
all : program

program : rread.c
	$(CC) rread.c -o rread $(LIBS) $(CFLAGS) $(LDFLAGS)  


# The clean target is used to remove all machine generated files 
# and start over from a clean slate. This will prove extremely
# useful. It is an example of a dummy target, as there will never be a
# file named clean. Thus "make clean" will always cause the following
# command to be executed.

#clean :
#	rm -f program *.png output-data
