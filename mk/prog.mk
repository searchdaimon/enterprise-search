include ${BOITHOHOME}/mk/setup.mk
include ${BOITHOHOME}/mk/cc.mk
include ${BOITHOHOME}/mk/objdir.mk

all: $(PROG)

ifdef WANT_BASE64
LDFLAGS+=	${LIBDIR}/libbase64.a
endif

ifdef WANT_LIBCONFIG
LDFLAGS+=	/usr/local/lib/libconfig.a
endif

ifdef WANT_MYSQL
LDFLAGS+=	/usr/lib/mysql/libmysqlclient.a
CFLAGS+=	-I/usr/include/mysql
endif

ifdef WANT_LIBXML
LDFLAGS+=	-lxml2
CFLAGS+=	-I/usr/include/libxml2 
endif

ifdef WANT_BDB
CFLAGS+=	-I/usr/local/BerkeleyDB.4.5/include/
LDFLAGS+=	/usr/local/BerkeleyDB.4.5/lib/libdb.a
endif

ifdef WANT_SPELLING
LDFLAGS+=	${LIBDIR}/libspelling.a
endif

ifdef WANT_DS
LDFLAGS+=	$(BOITHOHOME)/src/ds/libds.a
endif

ifdef WANT_GETDATE
LDFLAGS+=	$(LIBDIR)/libgetdate.a
endif

ifdef WANT_COMMON
LDFLAGS+=	${LIBDIR}/libcommon.a
endif

ifdef WANT_HASHTABLE
LDFLAGS+=	$(LIBDIR)/libhashtable.a
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
