
CPP=g++ -O2

all: libppt ppt2html

clean:
	rm -rf *.o libppt.a ppt2html

libppt:
	$(CPP) -c slide.cpp
	$(CPP) -c objects.cpp
	$(CPP) -c presentation.cpp
	$(CPP) -c pole.cpp
	$(CPP) -c powerpoint.cpp
	$(CPP) -c ustring.cpp
	ar rc libppt.a slide.o objects.o presentation.o pole.o powerpoint.o ustring.o
	ranlib libppt.a

ppt2html:
	$(CPP) -o ppt2html ppt2html.cpp libppt.a


.PHONY:	all clean libppt ppt2html

