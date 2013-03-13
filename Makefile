# Makefile
#global Make setup
include mk/setup.mk


# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS += -g -DATTRIBUTES
# -DDEBUG

# The dynamic libraries that the executable needs to be linked to
LDFLAGS += -lm -lz -D_FILE_OFFSET_BITS=64 -O2 -DIIACL lib/libds.a

#pop rank bibloteket
LIBS += src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c src/base64/base64.c src/common/

LIBGeoIP = -lGeoIP

#bruker culr isteden da det er threadsafe
#LINWWWCFCLAGS = `libwww-config --cflags`
#LINWWWLIBS = `libwww-config --libs`

CURLLIBS = `curl-config --libs`

#LIBCONFIG= -lconfig
#LIBCONFIG=  /usr/local/lib/libconfig.a
LIBCONFIG=  /usr/local/lib/libconfig.a
LIBCACHE=       src/libcache/libcache.c

IM = /home/eirik/.root/lib/libMagick.a /home/eirik/.root/lib/libWand.a -I/home/eirik/.root/include
#IM = -L/home/eirik/.root/lib -I/home/eirik/.root/include `/home/eirik/.root/bin/Wand-config --ldflags --libs`
#IM = /home/eirik/.root/lib/libMagick.a -I/home/eirik/.root/include `/home/eirik/.root/bin/Wand-config --ldflags --libs`

# #bbh1:
# BDB = -I/usr/local/BerkeleyDB.4.5/include/ /usr/local/BerkeleyDB.4.5/lib/libdb.a
# MYSQL = -I/usr/include/mysql /usr/lib/mysql/libmysqlclient.a
# CC+=-D NO_64_BIT_DIV
# #MYSQL4 = -I/home/eirik/.root/include/mysql -L/home/eirik/.root/lib/mysql/ -lmysqlclient -DMYSQLFOUR
# MYSQL4 = -I/home/eirik/.root/include/mysql /home/eirik/.root/lib/mysql/libmysqlclient.a -DMYSQLFOUR
# #PERLEMBED = lib/libperlembed.a -rdynamic `perl -MExtUtils::Embed -e ccopts -e ldopts` -I/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE   -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl 

# ##bbh2
# #BDB = -I/usr/include/db4 /usr/lib/libdb-4.6.a -lpthread
# #MYSQL = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient
# #CC+=-m32
# # !!!!!!!!! mangler MYSQL4 !!!!!!!!
# #PERLEMBED = src/perlembed/*.c -rdynamic -Wl,-E -Wl,-rpath,/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE  /usr/lib/perl5/5.8.8/i386-linux-thread-multi/auto/DynaLoader/DynaLoader.a -L/usr/lib/perl5/5.8.8/i386-linux-thread-multi/CORE -lperl 

#SMBCLIENT=-lsmbclient
#skrur dette på igjen. Brukte det og segfeile når vi hadde det med statisk?
# !! av ukjenet grunner ser dette ut til og altid må være sist hvis vi skal linke statisk

#SMBCLIENT=src/3pLibs/samba-3.0.24/source/bin/libsmbclient.a -Isrc/3pLibs/samba-3.0.24/source/include/
#SMBCLIENT=/home/boitho/src/samba-3.0.24/source/bin/libsmbclient.a -I/home/boitho/src/samba-3.0.24/source/include/
SMBCLIENT=src/3pLibs/samba-3.0.25b/source/bin/libsmbclient.a -Isrc/3pLibs/samba-3.0.25b/source/include/
#SMBCLIENT=-Isrc/3pLibs/samba-3.0.24/source/include/ -Lsrc/3pLibs/samba-3.0.24/source/lib/ -lsmbclient

BBDOCUMENT = src/bbdocument/bbdocument.c src/bbdocument/bbfilters.c lib/libds.a $(BDB_INC) $(BDB_LIB) -D BLACK_BOKS  
#BBDOCUMENT_IMAGE = src/generateThumbnail/generate_thumbnail.c -DBBDOCUMENT_IMAGE $(IM)
BBDOCUMENT_IMAGE = src/generateThumbnail/generate_thumbnail_by_convert.c -DBBDOCUMENT_IMAGE_BY_CONVERT

#openldap med venner. Må linke det statisk inn, å bare bruke -lldap fungerer ikke
#
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a -lsasl2 -lsasl -lcrypt -lssl
#LDAP = -DWITH_OPENLDAP -lldap -llber
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.so.2 /usr/lib/liblber.so.2
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a /usr/lib/libcrypto.a -lssl
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a src/3pLibs/openssl-0.9.8d/libssl.a src/3pLibs/openssl-0.9.8d/libcrypto.a -ldl 
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a /usr/local/lib/libssl.a /usr/local/lib/libcrypto.a -ldl 
#LDAP = -DWITH_OPENLDAP -I/home/boitho/.root/ -L/home/boitho/.root/ -lldap 
LDAP = -DWITH_OPENLDAP -I/home/boitho/.root/ -L/home/boitho/.root/ /usr/lib/libcrypto.a -lldap 
LDAPBB = -DWITH_OPENLDAP -lldap 

#flag for å inkludere mysql
#MYSQL = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient
MYSQL = -I/usr/include/mysql /usr/lib/mysql/libmysqlclient.a

SLICENCE=	src/slicense/base32.c  src/slicense/license.c


MYSQL_THREAD = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

#LIBXML = -I/usr/include/libxml2 -L/usr/lib -lxml2
LIBXML = -I/usr/include/libxml2  -lxml2

#HTMLPARSER=src/parser/lex.bhpm.c src/parser/y.tab.c  
#har rullet tilbake, og bruker gammel html parser for nå, så trenger dermed ikke i ha med css parseren
HTMLPARSER1=src/parser/libhtml_parser.a lib/libds.a
HTMLPARSER2=src/parser2/libhtml_parser.a src/parser2/libcss_parser.a lib/libds.a



# The Dependency Rules
# They take the form
# target : dependency1 dependency2...
#        Command(s) to generate target from dependencies

# Dummy target that is processed by default. It specifies al list of
# other targets that are processed if make is invoked with no arguments
# However if you invoke make as "make output-data", it will only try to 
# generate the file output-data and its dependencies, not plot.png 


all : init.d.stop getFiletype dep searchdbb dispatcher_allbb crawlManager2 infoquery crawlSMB crawlExchange boitho-bbdn PageInfobb IndexerLotbb mergeIIndex mergeUserToSubname ShowThumbbb everrun dictionarywordsLot boithoad Suggest gcRepobb gcAuthoritybb sdperl readUserToSubname bbdocumentWebAdd slicense_info usSQLBB usAD ShowCache2bb list_collections crawlExchangePublic LotInvertetIndexMaker3bb readIIndex rreadbb readDocumentIndexbb usSQLBB usAD crawlPush init.d.start

init.d.stop:
	@echo ""
	@echo "$@:"

	perl stopstart.pl stop	

init.d.start:
	@echo ""
	@echo "$@:"

	perl stopstart.pl start

perlembed:
	@echo ""
	@echo "$@:"

	(cd src/perlembed && make clean && make)

sdperl:
	@echo ""
	@echo "$@:"

	(cd src/perl && make clean && make)

dppreload:
	@echo ""
	@echo "$@:"

	$(CC) -shared -fPIC src/dp/preload.c src/common/timediff.c -o bin/dppreload.so -ldl -Wall $(LDFLAGS)

dptest:
	@echo ""
	@echo "$@:"

	$(CC) src/dp/test.c src/dp/dp.c -o bin/dptest -Wall $(LDFLAGS)

24sevenoffice:
	@echo ""
	@echo "$@:"

	env 24SEVENOFFICE=-D_24SEVENOFFICE make bb

setuid: YumWrapper NetConfig InitServices setuidcaller repomodwrap yumupdate

tempFikes: IndexerLot_fik32bitbug DIconvert


wordConverter: src/wordConverter/main.c
	$(CC) src/wordConverter/main.c -o bin/wordConverter

getFiletype:
	@echo ""
	@echo "$@:"

	(cd src/getFiletype && make clean && make)

repomodwrap:
	@echo ""
	@echo "$@:"

	$(CC) -Wall -o setuid/repomodwrap src/repomodsetuid/repomodwrap.c -O2

setuidcaller:
	@echo ""
	@echo "$@:"

	(cd src/bb-phone-home/ && make clean &&  make)
	cp src/bb-phone-home/bb-phone-home-client.conf config
	cp src/bb-phone-home/bb-client.pl bin/
	cp src/bb-phone-home/setuidcaller bin/
	cp src/bb-phone-home/bbph-keep-alive.pl bin/

invalidateOldFiles:
	@echo ""
	@echo "$@:"

	(cd src/invalidateOldFiles && make clean && make)
	cp src/invalidateOldFiles/invalidateOldFiles bin/

perlxs-sdcrawl:
	@echo ""
	@echo "$@:"

	(cd perlxs/SD-Crawl && ../../bin/perl Makefile.PL && make clean && perl Makefile.PL && make)

bbdocumentWebAdd:
	@echo ""
	@echo "$@:"

	(rm -f src/base64/base64.o && cd src/bbdocumentWebAdd/ && make clean && make)

bbdocumentHttpApi:
	@echo ""
	@echo "$@:"

	(rm -f src/base64/base64.o && cd src/bbdocumentHttpApi/ && make clean && make)


Suggest:
	@echo ""
	@echo "$@:"

	(cd src/suggestrpc && make clean && make)
	cp src/suggestrpc/suggest_server bin/
	(cd src/suggestwebclient && make clean && make)
	cp src/suggestwebclient/suggest_webclient cgi-bin/

SuggestOEM:
	@echo ""
	@echo "$@:"

	(cd src/suggestrpc && make clean && make)
	cp src/suggestrpc/suggest_server bin/
	(cd src/suggestwebclient && make clean && env EXTRA_CFLAGS=-DSUGGEST_OEM make)
	cp src/suggestwebclient/suggest_webclient cgi-bin/


#brukte før src/parser/libhtml_parser.a, byttet til src/parser/lex.yy.c src/parser/lex.yy.c slik at vi kan bruke gdb
IndexerLot= $(CFLAGS) $(LIBS)*.c src/dictionarywordsLot/set.c src/dictionarywordsLot/acl.c src/acls/acls.c src/IndexerRes/IndexerRes.c src/IndexerLot/main.c src/searchFilters/searchFilters.c lib/libds.a $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS -D WITHOUT_DIWRITE_FSYNC -D EXPLAIN_RANK

IndexerLot: src/IndexerLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) $(HTMLPARSER2) src/maincfg/maincfg.c src/dispatcher_all/library.c -lpthread -DWITH_THREAD src/banlists/ban.c -o bin/IndexerLot $(LIBCONFIG)

IndexerLotbb: src/IndexerLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) $(HTMLPARSER2) $(BDB_INC) $(BDB_LIB) -D BLACK_BOKS -D PRESERVE_WORDS -o bin/IndexerLotbb -DIIACL

baddsPageAnalyser: src/baddsPageAnalyser/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerRes/IndexerRes.c src/baddsPageAnalyser/main.c  src/httpGet/httpGet.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/baddsPageAnalyser $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(CURLLIBS) -DDEBUG_ADULT

rreadWithRank: src/rreadWithRank/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rreadWithRank/main.c  -o bin/rreadWithRank $(LDFLAGS)  

sortCrc32attrMap: src/sortCrc32attrMap/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/sortCrc32attrMap/main.c -o bin/sortCrc32attrMap $(LDFLAGS)


IndexerLot_langtest: src/IndexerLot_langtest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_langtest/main.c  src/parser/lex.yy.c src/parser/y.tab.c -o bin/IndexerLot_langtest $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS -DDEBUG_ADULT
	

IndexerLot_fik32bitbug: src/IndexerLot_fik32bitbug/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_fik32bitbug/main.c  src/parser/libhtml_parser.a -o bin/IndexerLot_fik32bitbug $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

IndexerLot_getno: src/IndexerLot_getno/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_getno/main.c  src/parser/lex.yy.c src/parser/lex.yy.c -o bin/IndexerLot_getno $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

dictionarywordsLot: src/dictionarywordsLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/dictionarywordsLot/main.c src/dictionarywordsLot/acl.c src/dictionarywordsLot/set.c   -o bin/dictionarywordsLot $(LDFLAGS) -DBLACK_BOKS

lotlistDispatcher: src/lotlistDispatcher/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/lotlistDispatcher/main.c -o bin/lotlistDispatcher $(LDFLAGS)

searchfilterTest: src/searchfilterTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/searchfilterTest/main.c src/searchfilter/searchfilter.c -o bin/searchfilterTest $(LDFLAGS)

infoquery: src/infoquery/main.c
	@echo ""
	@echo "$@:"
	(cd src/infoquery && make clean && make)

GetIndexAsArrayTest: src/GetIndexAsArrayTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/GetIndexAsArrayTest/main.c -o bin/GetIndexAsArrayTest $(LDFLAGS)

bbdocumentMakeLotUrlsdb: src/bbdocumentMakeLotUrlsdb/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentMakeLotUrlsdb/main.c -o bin/bbdocumentMakeLotUrlsdb $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS

bbdocumentConvertTest: src/bbdocumentConvertTest/main.c
	@echo ""
	@echo "$@:"

	#$(CC) $(CFLAGS) $(LIBS)*.c src/acls/acls.c src/bbdocumentConvertTest/main.c -o bin/bbdocumentConvertTest $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS
	(cd src/bbdocumentConvertTest && make clean && make)


analyseShortRank: src/analyseShortRank/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/analyseShortRank/main.c -o bin/analyseShortRank $(LDFLAGS)

DIconvert: src/DIconvert/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIconvert/main.c -o bin/DIconvert $(LDFLAGS)

boithoad: src/boithoad/main.c
	@echo ""
	@echo "$@:"
	#for lokalt på bb: gcc -g src/common/*.c src/boithoad/main.c   -o bin/boithoad -lm -lz -D_FILE_OFFSET_BITS=64 -O2 -DIIACL -DWITH_OPENLDAP /usr/lib64/libcrypt.a  /usr/lib64/libssl.a -I/usr/include/mysql/ -L/usr/lib64/mysql/ -ldl   -D BLACK_BOKS -D WITH_CONFIG -DDEBUG ../../openldap-2.3.32/libraries/libldap/.libs/libldap.a ../../openldap-2.3.32/libraries/liblber/.libs/liblber.a -lmysqlclient -lsasl2
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoad/main.c $(SLICENCE) -o bin/boithoad $(LDFLAGS) $(LDAP) $(MYSQL) $(LIBCACHE) $(OPENSSL) -pthread -DWITH_DAEMON_THREAD -D BLACK_BOKS -D WITH_CONFIG -DWITH_THREAD -DLIBCACHE_SHARE

PiToWWWDocID: src/PiToWWWDocID/main.c
	@echo ""
	@echo "$@:"
	$(CC) $(CFLAGS) $(LIBS)*.c src/PiToWWWDocID/main.c src/UrlToDocID/search_index.c -o bin/PiToWWWDocID $(LDFLAGS)  $(MYSQL) $(BDB_INC) $(BDB_LIB) -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE


boithoadtest: src/boithoadtest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithoadtest/main.c src/boithoadClientLib/liboithoaut.a -o bin/boithoadtest $(LDFLAGS)


lotlistPrint: src/lotlistPrint/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/lotlistPrint/main.c  -o bin/lotlistPrint $(LDFLAGS)

cleanCacheDir: src/cleanCacheDir/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/cleanCacheDir/main.c -o bin/cleanCacheDir

lotcp: src/lotcp/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/lotcp/main.c  -o bin/lotcp $(LDFLAGS)

missinglotDetectLocal: src/missinglotDetectLocal/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/missinglotDetectLocal/main.c  -o bin/missinglotDetectLocal $(LDFLAGS)

missinglotRemoveFormUdfile: src/missinglotRemoveFormUdfile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/missinglotRemoveFormUdfile/main.c  -o bin/missinglotRemoveFormUdfile $(LDFLAGS)

missinglotGetFormUdfile: src/missinglotGetFormUdfile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/missinglotGetFormUdfile/main.c  -o bin/missinglotGetFormUdfile $(LDFLAGS)

sortLinkdb: src/linkdbTools/sortLinkdb.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/linkdbTools/sortLinkdb.c  -o bin/sortLinkdb $(LDFLAGS)

BrankCalculate2GetPageElements: src/BrankCalculate2GetPageElements/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate2GetPageElements/main.c  -o bin/BrankCalculate2GetPageElements $(LDFLAGS)


BrankCalculateGetUrls: src/BrankCalculateGetUrls/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculateGetUrls/main.c  -o bin/BrankCalculateGetUrls $(LDFLAGS)

alllot: src/alllot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/alllot/main.c  -o bin/alllot $(LDFLAGS)


BrankCalculate2Publish: src/BrankCalculate2Publish/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate2Publish/main.c  -o bin/BrankCalculate2Publish $(LDFLAGS)

BrankCalculate5Publish: src/BrankCalculate5Publish/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate5Publish/main.c  -o bin/BrankCalculate5Publish $(LDFLAGS)

BrankCalculate5DocFork: src/BrankCalculate5DocFork/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate5DocFork/main.c  -o bin/BrankCalculate5DocFork $(LDFLAGS) -D_GNU_SOURCE


vipurls: src/vipurls/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c  src/vipurls/main.c  -o bin/vipurls $(LDFLAGS)

shortUrls: src/shortUrls/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/shortUrls/main.c  -o bin/shortUrls $(LDFLAGS)

urldispatcher: src/urldispatcher/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/urldispatcher/main.c  -o bin/urldispatcher $(LDFLAGS)

urlsDocumentIndexadd: src/urlsDocumentIndexadd/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/urlsDocumentIndexadd/main.c  -o bin/urlsDocumentIndexadd -D DI_FILE_CASHE $(LDFLAGS)

netlotStart: src/netlotStart/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/netlotStart/main.c  -o bin/netlotStart $(LDFLAGS)

netlotEnd: src/netlotEnd/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/netlotEnd/main.c  -o bin/netlotEnd $(LDFLAGS)

LotNyeurlerSort: src/LotNyeurlerSort/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotNyeurlerSort/main.c  -o bin/LotNyeurlerSort $(LDFLAGS)

LotNyeurlerMove: src/LotNyeurlerMove/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotNyeurlerMove/main.c  -o bin/LotNyeurlerMove $(LDFLAGS)

crawlFiles: src/crawlFiles/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/crawlFiles/main.c  -o bin/crawlFiles $(LDFLAGS) src/boitho-bbdn/bbdnclient.c -D BLACK_BOKS

crawlPush: src/crawlPush/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g $(CRAWL_STATIC) $(LIBS)*.c src/crawlPush/main.c -o src/crawlPush/crawlPush.so $(LDFLAGS) 
	mkdir -p crawlers/crawlPush
	cp src/crawlPush/crawlPush.so crawlers/crawlPush/
	

testGetNextLotForIndex: src/testGetNextLotForIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/testGetNextLotForIndex/main.c  -o bin/testGetNextLotForIndex $(LDFLAGS)

shortenurl: src/searchkernel/shortenurl.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/searchkernel/shortenurl.c  -o bin/shortenurl $(LDFLAGS) -D WITH_SHORTENURL_MAIN

everrun: src/everrun/catchdump.c
	@echo ""
	@echo "$@:"

	$(CC) src/everrun/main.c -o bin/everrun

searchcl : src/searchkernel/searchcl.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/query/lex.query.o src/searchkernel/cgi-util.c src/searchkernel/parseEnv.c src/searchkernel/searchkernel.c src/searchkernel/search.c src/searchkernel/searchcl.c src/parse_summary/libsummary.a -o bin/searchcl $(LDFLAGS)

#dropper -D WITH_MEMINDEX og -D WITH_RANK_FILTER for nå
#SEARCHCOMMAND = $(CFLAGS) $(LIBS)*.c src/query/lex.query.c   src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c src/parse_summary/libsummary.a src/parse_summary/libhighlight.a  $(LDFLAGS) -lpthread -D WITH_THREAD $(LIBCONFIG)
#må ha -D_GNU_SOURCE for O_DIRECT
SEARCHCOMMAND = $(CFLAGS) $(LIBS)*.c src/searchkernel/htmlstriper.c src/dp/dp.c src/searchkernel/verbose.c src/maincfg/maincfg.c src/searchkernel/shortenurl.c src/query/stemmer.o src/query/lex.query.o src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c $(HTMLPARSER2) src/generateSnippet/libsnippet_generator.a  lib/libds.a src/utf8-filter/lex.u8fl.o $(LDFLAGS) -lpthread $(LIBCONFIG) -D DISK_PROTECTOR


dep:
	#ting searchd trenger
	@echo ""
	@echo "$@:"
	for i in src/query src/generateSnippet src/ds src/utf8-filter src/getdate src/parser2 src/newspelling src/getFiletype src/attributes/ src/base64/ src/common/ src/getdate/ src/3pLibs/keyValueHash/ src/perlembed src/logger src/boithoadClientLib; do\
           echo ""; 												\
	   echo "Making $$i:"; 											\
           (cd $$i && $(MAKE) clean && $(MAKE) all); 										\
	   if [ $$? -ne 0 ];  then echo ""; echo "Sorry, can't build $$i! Please see errors above."; break; fi 	\
        done


searchd : src/searchkernel/searchd.c
	@echo ""
	@echo "$@:"
	
	$(CC) $(SEARCHCOMMAND) -D WITH_RANK_FILTER -D WITH_THREAD -D DEFLOT -o bin/searchd -D EXPLAIN_RANK 


searchdbb : src/searchkernel/searchd.c
	@echo ""
	@echo "$@:"
	(cd src/searchkernel/ && make clean && make)

mergeUserToSubname: src/mergeUserToSubname/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/mergeUserToSubname/main.c src/acls/acls.c -o bin/mergeUserToSubname $(LDFLAGS) -DBLACK_BOKS $(BDB_INC) $(BDB_LIB)

readUserToSubname: src/readUserToSubname/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readUserToSubname/main.c src/acls/acls.c -o bin/readUserToSubname $(LDFLAGS) -DBLACK_BOKS $(BDB_INC) $(BDB_LIB)

readUrls.db: src/readUrls.db/main.c
	@echo ""
	@echo "$@:"

	#$(CC) $(CFLAGS) $(LIBS)*.c src/readUrls.db/main.c src/acls/acls.c $(BBDOCUMENT) -o bin/readUrls.db $(LDFLAGS) -DBLACK_BOKS $(BDB_INC) $(BDB_LIB)
	(cd src/readUrls.db/ && make clean && make)

boithoads: src/boithoads/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS) -D EXPLAIN_RANK

boithoadshread: src/boithoads/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL_THREAD) $(LIBXML) $(CURLLIBS) -D WITH_THREAD -lpthread

addout.cgi: src/addout.cgi/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/addout.cgi/main.c src/cgi-util/cgi-util.c -o cgi-bin/addout.cgi $(LDFLAGS) $(MYSQL) 

ppcXmlParserTest: src/ppcXmlParserTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/ppcXmlParserTest/main.c src/parse_summary/libsummary.a src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/parse_summary/libsummary.a src/httpGet/httpGet.c -o bin/ppcXmlParserTest $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS)

deleteDocIDFormCache:
	$(CC) $(CFLAGS) $(LIBS)*.c src/dispatcher_all/library.c src/deleteDocIDFormCache/main.c -o bin/deleteDocIDFormCache $(LDFLAGS) -D WITH_CASHE -D EXPLAIN_RANK


dispatcherCOMAND = $(CFLAGS) $(LIBS)*.c src/banlists/ban.c src/UrlToDocID/search_index.c src/maincfg/maincfg.c src/dispatcher_all/library.c src/dispatcher_all/main.c src/tkey/tkey.c src/cgi-util/cgi-util.c src/searchFilters/searchFilters.c $(LDFLAGS) src/dispatcher_all/qrewrite.o src/dispatcher_all/cgihandler.c src/dispatcher_all/out/opensearch.c src/dispatcher_all/out/sdxml.c src/key/key.c $(BDB_INC) $(BDB_LIB) -D_GNU_SOURCE 

dispatcher_all: src/dispatcher_all/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(MYSQL) $(LIBGeoIP) -D WITH_CASHE -o cgi-bin/dispatcher_all $(LIBCONFIG) -D EXPLAIN_RANK 

dispatcher_allsql3: src/dispatcher_all/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(MYSQL) $(LIBGeoIP) -D WITH_CASHE -o cgi-bin/dispatcher_all $(LIBCONFIG) -D EXPLAIN_RANK 

dispatcher_allbb: src/dispatcher_all/main.c src/dispatcher_all/qrewrite.o 
	@echo ""
	@echo "$@:"

	@#$(CC) $(dispatcherCOMAND) $(MYSQL4) src/acls/acls.c src/boithoadClientLib/boithoadClientLib.c src/crawlManager/client.c src/query/lex.query.o lib/libds.a -D BLACK_BOKS -o cgi-bin/dispatcher_allbb $(LIBCONFIG) $(24SEVENOFFICE) src/getFiletype/libfte.a src/attributes/libshow_attr.a  -DWITH_SPELLING $(BDB_INC) $(BDB_LIB)
	(cd src/dispatcher_all && make clean && make)

dispatcher_all247: src/dispatcher_all/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(MYSQL) -D BLACK_BOKS -o cgi-bin/dispatcher_allbb $(LIBCONFIG) lib/libds.a -D_24SEVENOFFICE


dispatcher: src/dispatcher/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/dispatcher/main.c src/cgi-util/cgi-util.c lib/libds.a -o bin/dispatcher $(LDFLAGS)

putFilesIntoFileTree: src/putFilesIntoFileTree/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/putFilesIntoFileTree/main.c -o bin/putFilesIntoFileTree $(LDFLAGS)

nyeurlerIntegeryCheck: src/nyeurlerIntegeryCheck/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/nyeurlerIntegeryCheck/main.c -o bin/nyeurlerIntegeryCheck $(LDFLAGS)

DIread : src/DIread/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIread/main.c -o bin/DIread $(LDFLAGS)


NETtest : src/NETtest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/NETtest/main.c -o bin/NETtest $(LDFLAGS)

anchorNetCopy : src/anchorNetCopy/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/anchorNetCopy/main.c -o bin/anchorNetCopy $(LDFLAGS)

threadReadTest: src/threadReadTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/threadReadTest/main.c -o bin/threadReadTest $(LDFLAGS) -lpthread -DWITH_THREAD


rread : src/rread/rread.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rread/rread.c -o bin/rread $(LDFLAGS)

rreadbb : src/rread/rread.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rread/rread.c -o bin/rreadbb $(LDFLAGS) -D BLACK_BOKS

readLotbb : src/readLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readLot/main.c -o bin/readLotbb $(LDFLAGS) -D BLACK_BOKS


convertReposetoryCOMAND = $(CFLAGS) $(LIBS)*.c src/convertReposetory/main.c -o bin/convertReposetory $(LDFLAGS)

convertReposetory: src/convertReposetory/main.c
	$(CC) $(convertReposetoryCOMAND)

convertReposetorybb: src/convertReposetory/main.c
	$(CC) $(convertReposetoryCOMAND) -D BLACK_BOKS


makeSumaryCashe: src/makeSumaryCashe/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/makeSumaryCashe/main.c src/parse_summary/libsummary.a -o bin/makeSumaryCashe $(LDFLAGS) -D NOWARNINGS

rread_getsomeurls: src/rread_getsomeurls/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rread_getsomeurls/main.c -o bin/rread_getsomeurls $(LDFLAGS)

IndexerLotLite: src/IndexerLotLite/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotLite/main.c -o bin/IndexerLotLite $(LDFLAGS)

readMainIndex: src/readMainIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readMainIndex/main.c -o bin/readMainIndex $(LDFLAGS)

readMainIndexSearchSha1: src/readMainIndexSearchSha1/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readMainIndexSearchSha1/main.c -o bin/readMainIndexSearchSha1 $(LDFLAGS)

LotInvertetIndexMaker: src/LotInvertetIndexMaker/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker/main.c lib/libds.a -o bin/LotInvertetIndexMaker $(LDFLAGS)

gcRepobb: src/gcRepo/gcrepo.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/gcRepo/gcrepo.c -o bin/gcRepobb $(LDFLAGS) -D BLACK_BOKS

gcAuthoritybb: src/gcAuthority/main.c
	@echo ""
	@echo "$@:"

	@#$(CC) $(CFLAGS) $(LIBS)*.c src/acls/acls.c $(BBDOCUMENT) src/gcAuthority/main.c -o bin/gcAuthoritybb $(LDFLAGS) -D BLACK_BOKS $(BDB_INC) $(BDB_LIB) $(MYSQL)
	(cd src/gcAuthority && make clean && make)

gcSummary: src/gcSummary/gcsummary.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/gcSummary/gcsummary.c -o bin/gcSummary $(LDFLAGS) -D DI_FILE_CASHE

LotInvertetIndexMakerSplice: src/LotInvertetIndexMakerSplice/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMakerSplice/main.c -o bin/LotInvertetIndexMakerSplice $(LDFLAGS)

listLostLots: src/listLostLots/main.c	
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/listLostLots/main.c -o bin/listLostLots $(LDFLAGS)

recoverUrlForLot: src/recoverUrlForLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/common/lotlist.c src/recoverUrlForLot/main.c -o bin/recoverUrlForLot $(LDFLAGS) -DDEFLOT -Wall

removeUnnecessaryRevindex: src/removeUnnecessaryRevindex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/removeUnnecessaryRevindex/main.c -o bin/removeUnnecessaryRevindex $(LDFLAGS)

LotInvertetIndexMaker2:	src/LotInvertetIndexMaker2/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker2/main.c lib/libds.a -o bin/LotInvertetIndexMaker2 $(LDFLAGS)

LotInvertetIndexMaker3:	src/LotInvertetIndexMaker3/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c  src/searchFilters/searchFilters.c src/LotInvertetIndexMaker3/main.c -o bin/LotInvertetIndexMaker3 $(LDFLAGS)

LotInvertetIndexMaker3bb:	src/LotInvertetIndexMaker3/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c  src/LotInvertetIndexMaker3/main.c -o bin/LotInvertetIndexMaker3bb $(LDFLAGS) -D BLACK_BOKS

readRevIndex:	src/readRevIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readRevIndex/main.c -o bin/readRevIndex $(LDFLAGS)

ThumbnaleDemon: src/ThumbnaleDemon/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ThumbnaleDemon/main.c -o bin/ThumbnaleDemon $(LDFLAGS)

PoprankMerge: src/PoprankMerge/main.c
	$(CC) $(CFLAGS) src/PoprankMerge/main.c -o bin/PoprankMerge $(LDFLAGS)

ipbann: src/ipbann/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ipbann/main.c  -o bin/ipbann $(LDFLAGS)

addanchors: src/addanchors/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addanchors/main.c -o bin/addanchors $(LDFLAGS)

IndexerLotAnchors: src/IndexerLotAnchors/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotAnchors/main.c -o bin/IndexerLotAnchors $(LDFLAGS)

IndexerLotAnchors2: src/IndexerLotAnchors2/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotAnchors2/main.c -o bin/IndexerLotAnchors2 $(LDFLAGS)

readNyeUrlerFil: src/readNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readNyeUrlerFil/main.c  -o bin/readNyeUrlerFil $(LDFLAGS)

filterNyeUrlerFil: src/filterNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/filterNyeUrlerFil/main.c  -o bin/filterNyeUrlerFil $(LDFLAGS)

addUrlsToIndex: src/addUrlsToIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addUrlsToIndex/main.c -o bin/addUrlsToIndex $(LDFLAGS)

readLinkFile: src/readLinkFile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readLinkFile/main.c -o bin/readLinkFile $(LDFLAGS)

mergeLinkDB:
	(cd src/mergeLinkDB/ && make clean && make)
	cp src/mergeLinkDB/mergeLinkDB bin/mergeLinkDB

anchorread: src/anchorread/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/anchorread/main.c -o bin/anchorread $(LDFLAGS)

anchorreadnew: src/anchorreadnew/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/anchorreadnew/main.c -o bin/anchorreadnew $(LDFLAGS)

BrankCalculate:	src/BrankCalculate/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate/*.c -o bin/BrankCalculate $(LDFLAGS)

BrankCalculate2: src/BrankCalculate2/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate2/*.c -o bin/BrankCalculate2 $(LDFLAGS)

BrankCalculate3: src/BrankCalculate3/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c  src/BrankCalculate3/main.c src/BrankCalculate3/res.c -o bin/BrankCalculate3 $(LDFLAGS) -DDEFLOT

BrankCalculate4: src/BrankCalculate4/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/bs/bs.c src/tq/tq.c  src/BrankCalculate4/main.c src/BrankCalculate3/res.c -o bin/BrankCalculate4 -lpthread $(LDFLAGS) -DDEFLOT -DWITH_THREAD

BrankCalculate5: src/BrankCalculate5/main.c
	$(CC) $(CFLAGS) src/3pLibs/keyValueHash/hashtable_itr.c src/common/daemon.c src/common/lotlist.c src/3pLibs/keyValueHash/hashtable.c src/common/reposetoryNET.c src/common/stdlib.c src/common/lot.c src/common/bstr.c src/common/strlcat.c src/common/bfileutil.c src/common/debug.c  src/banlists/ban.c src/bs/bs.c src/tq/tq.c   src/BrankCalculate5/main.c -o bin/BrankCalculate5 -lpthread $(LDFLAGS) -DDEFLOT -D_LARGEFILE64_SOURCE

BrankCalculate5Expand: src/BrankCalculate5Expand/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate3/res.c src/BrankCalculate5Expand/main.c -o bin/BrankCalculate5Expand $(LDFLAGS) -DDEFLOT 

BrankCalculateBanning: src/BrankCalculateBanning/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c  src/BrankCalculateBanning/main.c  -o bin/BrankCalculateBanning $(LDFLAGS) -DDEFLOT

BrankMerge: src/BrankMerge/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankMerge/*.c -o bin/BrankMerge $(LDFLAGS)

readLinkDB: src/readLinkDB/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c  src/readLinkDB/main.c -o bin/readLinkDB $(LDFLAGS)

SortUdfile: src/SortUdfile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SortUdfile/main.c -o bin/SortUdfile $(LDFLAGS)

SortUdfileToNewFiles: src/SortUdfileToNewFiles/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SortUdfileToNewFiles/main.c -o bin/SortUdfileToNewFiles $(LDFLAGS)

PageInfoComand=	$(LIBS)*.c src/PageInfo/main.c $(HTMLPARSER2) $(LDFLAGS) src/IndexerRes/IndexerRes.c src/cgi-util/cgi-util.c

PageInfo: src/PageInfo/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) -o bin/PageInfo

PageInfobb: src/PageInfo/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) src/boithoadClientLib/boithoadClientLib.c -o bin/PageInfobb -D BLACK_BOKS

addManuellUrlFile: src/addManuellUrlFile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/addManuellUrlFile/main.c  -o bin/addManuellUrlFile $(LDFLAGS)

urltest: src/urltest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/urltest/main.c  -o bin/urltest $(LDFLAGS)

dumpLotAsFiles: src/dumpLotAsFiles/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/dumpLotAsFiles/main.c -o bin/dumpLotAsFiles $(LDFLAGS)

boithold: src/boithold/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/boithold/main.c -o bin/boithold $(LDFLAGS)

boitho-bbdn: src/boitho-bbdn/bbdnserver.c src/bbdocument/bbfilters.c
	@#gammelt bygge sys
	@#$(CC) $(CFLAGS) $(LIBS)*.c src/acls/acls.c src/boitho-bbdn/bbdnserver.c src/maincfg/maincfg.c -o bin/boitho-bbdn $(LDFLAGS) $(BBDOCUMENT) -D BLACK_BOKS $(BBDOCUMENT_IMAGE)  $(LIBCONFIG) -DIIACL $(24SEVENOFFICE) -D NO_REUSEADDR -DUSE_LIBEXTRACTOR
	@echo ""
	@echo "$@:"

	(cd src/boitho-bbdn && make clean && make)


boitholdTest: src/boitholdTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/boitholdTest/main.c -o bin/boitholdTest $(LDFLAGS)

SplittUdfileByLot: src/SplittUdfileByLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/SplittUdfileByLot/main.c -o bin/SplittUdfileByLot $(LDFLAGS)

UrlToDocIDIndexer: src/UrlToDocIDIndexer/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/UrlToDocIDIndexer/main.c -o bin/UrlToDocIDIndexer $(LDFLAGS) $(BDB_INC) $(BDB_LIB)

UrlToDocIDQuery: src/UrlToDocIDQuery/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/getDocIDFromUrl/getDocIDFromUrl.c src/UrlToDocIDQuery/main.c -o bin/UrlToDocIDQuery $(LDFLAGS) $(BDB_INC) $(BDB_LIB)

UrlToDocIDSplitUdfile: src/UrlToDocIDSplitUdfile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/UrlToDocIDSplitUdfile/main.c -o bin/UrlToDocIDSplitUdfile $(LDFLAGS)

addReposerotyToIndex: src/addReposerotyToIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/addReposerotyToIndex/main.c -o bin/addReposerotyToIndex $(LDFLAGS)


readDocumentIndex: src/readDocumentIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readDocumentIndex/main.c -o bin/readDocumentIndex $(LDFLAGS)

readDocumentIndexByRe: src/readDocumentIndexByRe/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readDocumentIndexByRe/main.c -o bin/readDocumentIndexByRe $(LDFLAGS)

readAttributeIndex: src/readAttributeIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readAttributeIndex/main.c -o bin/readAttributeIndex $(LDFLAGS)

lookForBadDocID: src/lookForBadDocID/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/lookForBadDocID/main.c -o bin/lookForBadDocID $(LDFLAGS)

lookForBadDocIDbb: src/lookForBadDocID/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/lookForBadDocID/main.c -o bin/lookForBadDocIDbb $(LDFLAGS) -D BLACK_BOKS

readDocumentIndexbb: src/readDocumentIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readDocumentIndex/main.c -o bin/readDocumentIndexbb $(LDFLAGS) -D BLACK_BOKS

readbrankPageElements: src/readbrankPageElements/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readbrankPageElements/main.c -o bin/readbrankPageElements $(LDFLAGS)

makeBrankPageElements: src/makeBrankPageElements/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/makeBrankPageElements/main.c -o bin/makeBrankPageElements $(LDFLAGS)

DIrecrawlSelect: src/DIrecrawlSelect/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIrecrawlSelect/main.c -o bin/DIrecrawlSelect $(LDFLAGS)

gcidentify: src/gcidentify/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c  src/banlists/ban.c src/gcidentify/main.c -o bin/gcidentify $(LDFLAGS)

DIcreateUdfile: src/DIcreateUdfile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIcreateUdfile/main.c -o bin/DIcreateUdfile $(LDFLAGS)


BrankCalculatePI: src/BrankCalculatePI/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/getDocIDFromUrl/getDocIDFromUrl.c src/BrankCalculatePI/main.c -o bin/BrankCalculatePI $(BDB_INC) $(BDB_LIB) $(LDFLAGS)

BrankCalculateMakeSimpeLinkDB: src/BrankCalculateMakeSimpeLinkDB/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculateMakeSimpeLinkDB/main.c -o bin/BrankCalculateMakeSimpeLinkDB $(LDFLAGS)

resolveRedirects: src/resolveRedirects/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/resolveRedirects/main.c src/UrlToDocID/search_index.c -o bin/resolveRedirects $(LDFLAGS) $(BDB_INC) $(BDB_LIB)

redirResource: src/resolveRedirects/redirResource.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/resolveRedirects/redirResource.c src/getDocIDFromUrl/getDocIDFromUrl.c -o bin/redirResource $(LDFLAGS) $(BDB_INC) $(BDB_LIB)

readDocumentIndexWithRank: src/readDocumentIndexWithRank/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/readDocumentIndexWithRank/main.c -o bin/readDocumentIndexWithRank $(LDFLAGS)

adultBuildIndex: src/adultBuildIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/adultBuildIndex/main.c -o bin/adultBuildIndex $(LDFLAGS)

ipdbBuildLotIndex: src/ipdbBuildLotIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/ipdbBuildLotIndex/main.c -o bin/ipdbBuildLotIndex $(LDFLAGS)

ipdbMakeMain: src/ipdbMakeMain/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/ipdbMakeMain/main.c -o bin/ipdbMakeMain $(LDFLAGS)

IndexerLotUrl: src/IndexerLotUrl/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotUrl/main.c -o bin/IndexerLotUrl $(LDFLAGS)

IndexerLotDomainId: src/IndexerLotDomainId/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotDomainId/main.c -o bin/IndexerLotDomainId $(LDFLAGS)

fixsAsciixBug: src/fixsAsciixBug/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/fixsAsciixBug/main.c -o bin/fixsAsciixBug $(LDFLAGS)

getUncrawledPages: src/getUncrawledPages/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/getUncrawledPages/main.c -o bin/getUncrawledPages $(LDFLAGS)

cpLotFile: src/cpLotFile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/cpLotFile/main.c -o bin/cpLotFile $(LDFLAGS)


SHOWTHUMBCMANDS = $(CFLAGS) $(LIBS)*.c src/ShowThumb/main.c src/cgi-util/cgi-util.c $(LDFLAGS)

ShowThumb: src/ShowThumb/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(SHOWTHUMBCMANDS) -o cgi-bin/ShowThumb

ShowThumbbb: src/ShowThumb/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(SHOWTHUMBCMANDS) -o cgi-bin/ShowThumbbb -D BLACK_BOKS

ShowCacheCOMMAND2 = $(CFLAGS) $(LIBS)*.c src/ShowCache2/main.c src/cgi-util/cgi-util.c $(LDFLAGS) 

ShowCache2: src/ShowCache/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(ShowCacheCOMMAND2) -o cgi-bin/ShowCache2

ShowCache2bb: src/ShowCache/main.c
	@echo ""
	@echo "$@:"
	$(CC) $(ShowCacheCOMMAND2) -DBLACK_BOKS -o cgi-bin/ShowCache2bb


ShowCacheCOMMAND = $(CFLAGS) $(LIBS)*.c src/ShowCache/main.c src/cgi-util/cgi-util.c $(LDFLAGS) -o cgi-bin/ShowCache

ShowCache: src/ShowCache/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(ShowCacheCOMMAND)

ShowCachebb: src/ShowCache/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(ShowCacheCOMMAND) -D BLACK_BOKS

boithoshmd: src/boithoshmd/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/boithoshmd/main.c -o bin/boithoshmd $(LDFLAGS)

builIpDB: src/builIpDB/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/builIpDB/main.c -o bin/builIpDB $(LDFLAGS)

ConvertRankToShortRankFile: src/ConvertRankToShortRankFile/main.c
	@echo ""
	@echo "$@:"

	$(CC) src/ConvertRankToShortRankFile/main.c -lm -o bin/ConvertRankToShortRankFile

readIIndex: src/readIIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) src/readIIndex/main.c -o bin/readIIndex

mergeIIndex: src/mergeIIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/mergeIIndex/main.c -o bin/mergeIIndex $(LDFLAGS)


boithoadClientLib: src/boithoadClientLib/boithoadClientLib.c
	$(CC) -c $(CFLAGS) src/common/daemon.c -o src/boithoadClientLib/daemon.o  -g
	$(CC) -c $(CFLAGS) src/boithoadClientLib/boithoadClientLib.c -o src/boithoadClientLib/boithoadClientLib.o  -g
	ar rc src/boithoadClientLib/liboithoaut.a src/boithoadClientLib/boithoadClientLib.o src/boithoadClientLib/daemon.o
	ranlib src/boithoadClientLib/liboithoaut.a


InitServices: src/InitServices/initwrapper.c
	(cd src/InitServices/ && make clean && make)

	cp src/InitServices/initwrapper setuid/

YumWrapper: src/YumWrapper/yumwrapper.c
	(cd src/YumWrapper && make clean && make)
	cp src/YumWrapper/yumwrapper setuid/

NetConfig: src/NetConfig/configwrite.c
	(cd src/NetConfig && make clean && make)
	cp src/NetConfig/configwrite setuid/

yumupdate:
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/common/exeoc.c src/yumupdate/yumupdate.c -o setuid/yumupdate

crawlManager: src/crawlManager/main.c
	@echo ""
	@echo "$@:"

	#22 feb 2007, fjerner -static
	$(CC) $(CFLAGS) -I/home/eirik/.root/include $(LIBS)*.c src/acls/acls.c src/maincfg/maincfg.c src/crawl/crawl.c src/boitho-bbdn/bbdnclient.c src/crawlManager/main.c -o bin/crawlManager $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS $(BBDOCUMENT) $(LIBCONFIG) -DIIACL -DWITH_CONFIG $(24SEVENOFFICE) lib/libds.a -rdynamic

crawlManager2: src/crawlManager2/main.c
	@echo ""
	@echo "$@:"
	(cd src/crawlManager2 && make clean && make all)



crawlManager2perltest: src/crawlManager2/perltest.c
	@echo ""
	@echo "$@:"



	$(CC) $(CFLAGS) $(LIBS)*.c src/crawlManager2/perlxsi.c src/crawlManager2/perlcrawl.c src/crawlManager2/perltest.c src/crawl/crawl.c -o bin/crawlManager2perltest $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS -DIIACL -rdynamic `perl -MExtUtils::Embed -e ccopts -e ldopts` 


crawlSMB: src/crawlSMB/main.c
	@echo ""
	@echo "$@:"

	flex -f -8 -i -o src/crawlSMB/lex.acl.c src/crawlSMB/acl.parser.l


	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g $(CRAWL_STATIC) $(LIBS)*.c src/crawlSMB/cleanresource.c src/crawlSMB/scan.c src/crawlSMB/lex.acl.c src/crawlSMB/crawlsmb.c src/crawl/crawl.c src/crawlSMB/main.c src/boitho-bbdn/bbdnclient.c -o src/crawlSMB/crawlSMB.so $(LDFLAGS) $(SMBCLIENT)
	mkdir -p crawlers/crawlSMB
	cp src/crawlSMB/crawlSMB.so crawlers/crawlSMB/


crawlExchange:
	@echo ""
	@echo "$@:"

	(cd src/crawlExchange && make clean)
	(cd src/crawlExchange && make)

crawlExchangePublic:
	@echo ""
	@echo "$@:"

	(cd src/crawlExchangePublic && make clean)
	(cd src/crawlExchangePublic && make)


crawlSO:
	@echo ""
	@echo "$@:"

	(cd src/crawlSO && make clean)
	(cd src/crawlSO && make)

usAD:
	@echo ""
	@echo "$@:"

	(cd src/us_ad && make clean)
	(cd src/us_ad && make)

usShell:
	@echo ""
	@echo "$@:"

	(cd src/us_shellexe && make clean)
	(cd src/us_shellexe && make)

usMapback:
	@echo ""
	@echo "$@:"

	(cd src/us_mapback && make clean)
	(cd src/us_mapback && make)

usSQLBB:
	@echo ""
	@echo "$@:"

	(cd src/us_sqlbb && make clean)
	(cd src/us_sqlbb && make)


crawl247:
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g $(CRAWL_STATIC) $(LIBS)*.c src/crawl247/crawl.c src/crawl/crawl.c src/boitho-bbdn/bbdnclient.c src/crawl/mailsubject.c -o src/crawl247/crawl247.so $(LDFLAGS) $(SMBCLIENT)
	mkdir -p crawlers/crawl247/
	cp src/crawl247/crawl247.so crawlers/crawl247/
	@#(cd src/crawl247 && make clean)
	@#(cd src/crawl247 && make)



crawlSFTP: src/crawlSFTP/crawlsftp.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -fPIC -shared -o src/crawlSFTP/crawlSFTP.so src/crawlSFTP/crawlsftp.c src/crawlSFTP/rutines.c src/crawl/crawl.c src/3pLibs/libssh2/src/*.o -g -O2 -I/usr/include -I/usr/include -Isrc/3pLibs/libssh2/include/ -L/usr/lib -lcrypto -L/usr/lib -lz
	mkdir -p crawlers/crawlSFTP
	cp src/crawlSFTP/crawlSFTP.so crawlers/crawlSFTP/

mod_auth_boitho_a1:
	@echo ""
	@echo "$@:"

	(cd src/mod_auth_boitho && make clean)
	(cd src/mod_auth_boitho && make Apache1)

mod_auth_boitho_a2:
	@echo ""
	@echo "$@:"

	(cd src/mod_auth_boitho && make clean)
	(cd src/mod_auth_boitho && make Apache2)

list_collections: 
	@echo ""
	@echo "$@:"

	(cd src/list_collections && make clean)
	(cd src/list_collections && make list_collections)
	cp src/list_collections/list_collections bin/list_collections

slicense_info:
	@echo ""
	@echo "$@:"

	(cd src/slicense && make clean)
	(cd src/slicense && make slicense_info)
	cp -v  src/slicense/slicense_info bin/slicense_info



#kopierer filer slik at de blir tilgjengelig fra web
webpublish:
	cp bin/searchkernel /home/boitho/cgi-bin/v13.3/bin
	cp bin/ThumbnaleDemon /home/boitho/cgi-bin/v13.3/bin

# The clean target is used to remove all machine generated files 
# and start over from a clean slate. This will prove extremely
# useful. It is an example of a dummy target, as there will never be a
# file named clean. Thus "make clean" will always cause the following
# command to be executed.

#clean :
#	rm -f program *.png output-data

