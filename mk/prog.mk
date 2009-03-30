include ${BOITHOHOME}/config.mk

include ${BOITHOHOME}/mk/setup.mk
include ${BOITHOHOME}/mk/cc.mk
include ${BOITHOHOME}/mk/objdir.mk

all: $(PROG)

ifdef WANT_COMMON
LDFLAGS+=	${LIBDIR}/libcommon.a
endif

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


OBJPROG=	${OBJDIR}/${PROG}

$(OBJPROG): $(addprefix $(OBJDIR)/, $(OBJS))
	${CC} -o $@ $^ ${LDFLAGS}

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(PROG): objdir $(OBJPROG)
	cp $(OBJPROG) ${BIN}/${PROG}

clean:
	rm -f $(OBJDIR)/$(OBJPROG) $(addprefix $(OBJDIR)/, $(OBJS))

.PHONY: clean
