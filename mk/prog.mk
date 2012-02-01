include ${BOITHOHOME}/mk/setup.mk
include ${BOITHOHOME}/mk/cc.mk
include ${BOITHOHOME}/mk/objdir.mk

all: $(PROG)

ifdef WANT_BASE64
LDFLAGS+=	${LIBDIR}/libbase64.a
endif

ifdef WANT_LIBCONFIG
ifeq ($(WANT_64BIT),1)
LDFLAGS+=	/usr/local/lib64/libconfig.a
else
LDFLAGS+=	/usr/local/lib/libconfig.a
endif
endif

ifdef WANT_MYSQL
LDFLAGS+=	$(MYSQL_LIB)
CFLAGS+=	$(MYSQL_INC)
endif

ifdef WANT_LIBXML
LDFLAGS+=	-lxml2
CFLAGS+=	-I/usr/include/libxml2 
endif

ifdef WANT_BDB
CFLAGS+=	$(BDB_INC)
LDFLAGS+=	$(BDB_LIB)
#CFLAGS+=	-I/usr/local/BerkeleyDB.4.5/include/
#LDFLAGS+=	/usr/local/BerkeleyDB.4.5/lib/libdb.a
endif

ifdef WANT_SPELLING
LDFLAGS+=	${LIBDIR}/libspelling.a
endif

ifdef WANT_GETDATE
LDFLAGS+=	$(LIBDIR)/libgetdate.a
endif

ifdef WANT_COMMON
LDFLAGS+=	${LIBDIR}/libcommon.a
endif

ifdef WANT_DS
LDFLAGS+=	$(LIBDIR)/libds.a
endif

ifdef WANT_HASHTABLE
LDFLAGS+=	$(LIBDIR)/libhashtable.a
endif

ifdef WANT_PERLEMBED
LDFLAGS+=	${LIBDIR}/libperlembed.a
LDFLAGS+=	$(PERL_EMBED_LIB)
CFLAGS+=	$(PERL_EMBED_INC)
endif

ifdef WANT_LOGGER
LDFLAGS+=	${LIBDIR}/liblogger.a
endif

ifndef NO_DEBUG
CFLAGS+=	-g
endif


OBJPROG=	${OBJDIR}/${PROG}

$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJS))
	${CC} -o $@ $^ ${LDFLAGS}

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(PROG): objdir $(OBJPROG)
	cp $(OBJPROG) ${BIN}/${PROG}$(USEPREFIX)

clean:
	rm -f $(OBJDIR)/$(OBJPROG) $(addprefix $(OBJDIR)/, $(OBJS))

.PHONY: clean
