include ../../mk/setup.mk

CFLAGS += -std=c99 -O2 -g -W -Wall
LDFLAGS += -lz -lm
LIBS += ../3pLibs/keyValueHash/hashtable_itr.c ../3pLibs/keyValueHash/hashtable.c ../base64/base64.c ../../lib/libds.a
OBJS= list_collections.o


list_collections: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ ../common/*.c $(LIBS)

clean:
	rm -rf *.o list_collections
