CFLAGS=	-DWITH_PUBLIC_FOLDERS
PROG=	crawlExchangePublic

VPATH=	../crawlExchange

cleanfirst:
	(cd ../crawlExchange; make clean)
	make run

run: all

include ../crawlExchange/Makefile
