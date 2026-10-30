# C unit tests. Each test is a small TAP-emitting program linked against the
# static libraries `make dep` leaves in $(BOITHOHOME)/lib and the flex-generated
# objects in src/query and src/utf8-filter, i.e. the exact code the daemons use.
#
#   make                 build every test into bin/
#   make check           build and run them through prove
#   make SANITIZE=1      build with AddressSanitizer + UBSan
#
# Requires `make dep` to have been run at the top level first.

include $(BOITHOHOME)/mk/setup.mk

SRC=		$(BOITHOHOME)/src
BINDIR=		bin

CFLAGS+=	-Wall -Wno-unused-function -I$(SRC) -I.
ifdef SANITIZE
CFLAGS+=	-fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS+=	-fsanitize=address,undefined
endif

LIBCOMMON=	$(LIBDIR)/libcommon.a
LIBDS=		$(LIBDIR)/libds.a
# libcommon has internal cross references, so it is listed twice for the linker.
COMMON_LIBS=	$(LIBCOMMON) $(LIBCOMMON) -lm -lpthread

TESTS=		$(BINDIR)/test_bstr \
		$(BINDIR)/test_bfileutil \
		$(BINDIR)/test_crc32 \
		$(BINDIR)/test_lot \
		$(BINDIR)/test_url \
		$(BINDIR)/test_utf8filter \
		$(BINDIR)/test_query \
		$(BINDIR)/test_html_parser

all: $(TESTS)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/test_bstr: test_bstr.c tap.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_bfileutil: test_bfileutil.c tap.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_crc32: test_crc32.c tap.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_lot: test_lot.c tap.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_url: test_url.c tap.h | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_utf8filter: test_utf8filter.c tap.h $(SRC)/utf8-filter/lex.u8fl.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(SRC)/utf8-filter/lex.u8fl.o $(LDFLAGS)

$(BINDIR)/test_query: test_query.c tap.h $(SRC)/query/lex.query.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(SRC)/query/lex.query.o $(LIBDS) $(COMMON_LIBS) $(LDFLAGS)

$(BINDIR)/test_html_parser: test_html_parser.c tap.h $(SRC)/parser2/libhtml_parser.a $(SRC)/parser2/libcss_parser.a | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(SRC)/parser2/libhtml_parser.a $(SRC)/parser2/libcss_parser.a $(LIBDS) $(COMMON_LIBS) $(LDFLAGS)

check: all
	prove $(BINDIR)

clean:
	rm -rf $(BINDIR)

.PHONY: all check clean
