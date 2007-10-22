# Makefile

# The compiler to be used
CC = gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS = -g

# The dynamic libraries that the executable needs to be linked to
# fjerner -ldb -static. Må legge dette til der de skal være
LDFLAGS = -lm -lz -D_FILE_OFFSET_BITS=64 -O2 -DIIACL

#pop rank bibloteket
LIBS = src/common/

LIBGeoIP = -lGeoIP

#bruker culr isteden da det er threadsafe
#LINWWWCFCLAGS = `libwww-config --cflags`
#LINWWWLIBS = `libwww-config --libs`

CURLLIBS = `curl-config --libs`

#LIBCONFIG= -lconfig
LIBCONFIG=  /usr/local/lib/libconfig.a

IM = /home/eirik/.root/lib/libMagick.a /home/eirik/.root/lib/libWand.a -I/home/eirik/.root/include
#IM = -L/home/eirik/.root/lib -I/home/eirik/.root/include `/home/eirik/.root/bin/Wand-config --ldflags --libs`
#IM = /home/eirik/.root/lib/libMagick.a -I/home/eirik/.root/include `/home/eirik/.root/bin/Wand-config --ldflags --libs`

#BDB = -I/usr/local/BerkeleyDB.4.5/include/ -L/usr/local/BerkeleyDB.4.5/lib/ -ldb
BDB = -I/usr/local/BerkeleyDB.4.5/include/ /usr/local/BerkeleyDB.4.5/lib/libdb.a

#SMBCLIENT=-lsmbclient
#skrur dette på igjen. Brukte det og segfeile når vi hadde det med statisk?
# !! av ukjenet grunner ser dette ut til og altid må være sist hvis vi skal linke statisk

#SMBCLIENT=src/3pLibs/samba-3.0.24/source/bin/libsmbclient.a -Isrc/3pLibs/samba-3.0.24/source/include/
#SMBCLIENT=/home/boitho/src/samba-3.0.24/source/bin/libsmbclient.a -I/home/boitho/src/samba-3.0.24/source/include/
SMBCLIENT=/home/boitho/src/samba-3.0.25b/source/bin/libsmbclient.a -I/home/boitho/src/samba-3.0.25b/source/include/
#SMBCLIENT=-Isrc/3pLibs/samba-3.0.24/source/include/ -Lsrc/3pLibs/samba-3.0.24/source/lib/ -lsmbclient

BBDOCUMENT = src/bbdocument/bbdocument.c $(BDB) -D BLACK_BOKS
#BBDOCUMENT_IMAGE = src/generateThumbnail/generate_thumbnail.c -DBBDOCUMENT_IMAGE $(IM)
BBDOCUMENT_IMAGE = src/generateThumbnail/generate_thumbnail_by_convert.c -DBBDOCUMENT_IMAGE_BY_CONVERT

#openldap med venner. Må linke det statisk inn, å bare bruke -lldap fungerer ikke
#
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a -lsasl2 -lsasl -lcrypt -lssl
#LDAP = -DWITH_OPENLDAP -lldap -llber
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.so.2 /usr/lib/liblber.so.2
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a /usr/lib/libcrypto.a -lssl
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a src/3pLibs/openssl-0.9.8d/libssl.a src/3pLibs/openssl-0.9.8d/libcrypto.a -ldl 
LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a /usr/lib/liblber.a /usr/lib/libsasl.a /usr/local/lib/libssl.a /usr/local/lib/libcrypto.a -ldl 

#flag for å inkludere mysql
#MYSQL = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient
MYSQL = -I/usr/include/mysql /usr/lib/mysql/libmysqlclient.a
MYSQL4 = -I/home/eirik/.root/include/mysql -L/home/eirik/.root/lib/mysql/ -lmysqlclient -DMYSQLFOUR

MYSQL_THREAD = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

#LIBXML = -I/usr/include/libxml2 -L/usr/lib -lxml2
LIBXML = -I/usr/include/libxml2  -lxml2


#HTMLPARSER=src/parser/lex.bhpm.c src/parser/y.tab.c  
#har rullet tilbake, og bruker gammel html parser for nå, så trenger dermed ikke i ha med css parseren
#HTMLPARSER=src/parser/libhtml_parser.a src/parser/libcss_parser.a src/ds/libds.a
HTMLPARSER=src/parser/libhtml_parser.a src/ds/libds.a

# The Dependency Rules
# They take the form
# target : dependency1 dependency2...
#        Command(s) to generate target from dependencies

# Dummy target that is processed by default. It specifies al list of
# other targets that are processed if make is invoked with no arguments
# However if you invoke make as "make output-data", it will only try to 
# generate the file output-data and its dependencies, not plot.png 

all: 
	@echo "enten bygg bb med make bb, eller byg web med make web"

bb : getFiletype searchddep searchdbb dispatcher_allbb crawlManager infoquery crawlSMB crawlExchange boitho-bbdn PageInfobb boitho-bbdn IndexerLotbb LotInvertetIndexMaker2  mergeIIndex mergeUserToSubname bbdocumentConvertTest ShowThumbbb everrun dictionarywordsLot boithoad webadmindep

webadmindep: YumWrapper NetConfig InitServices setuidcaller

tempFikes: IndexerLot_fik32bitbug DIconvert


wordConverter: src/wordConverter/main.c
	$(CC) src/wordConverter/main.c -o bin/wordConverter

getFiletype:
	@echo ""
	@echo "$@:"

	(cd src/getFiletype; make)




setuidcaller:
	@echo ""
	@echo "$@:"

	(cd src/bb-phone-home/; make)
	@if [ `id -u` == 0 ]; then \
		cp -f src/bb-phone-home/setuidcaller bin/; \
		chown root bin/setuidcaller; \
		chmod +s bin/setuidcaller; \
	else \
		echo "################"; \
		echo "You are not root. Run these commands as root"; \
		echo "cp src/bb-phone-home/setuidcaller bin/"; \
		echo "chown root bin/setuidcaller"; \
		echo "chmod +s bin/setuidcaller"; \
		echo "################"; \
	fi

invalidateOldFiles:
	@echo ""
	@echo "$@:"

	(cd src/invalidateOldFiles; make clean; make)
	cp src/invalidateOldFiles/invalidateOldFiles bin/



Suggest:
	@echo ""
	@echo "$@:"

	(cd src/suggestrpc; make)
	cp src/suggestrpc/suggest_server bin/
	(cd src/suggestwebclient; make)
	cp src/suggestwebclient/suggest_webclient bin/

#brukte før src/parser/libhtml_parser.a, byttet til src/parser/lex.yy.c src/parser/lex.yy.c slik at vi kan bruke gdb
IndexerLot= $(CFLAGS) $(LIBS)*.c src/IndexerRes/IndexerRes.c src/IndexerLot/main.c src/searchFilters/searchFilters.c $(HTMLPARSER) $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

IndexerLot: src/IndexerLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) -lpthread -DWITH_THREAD src/banlists/ban.c src/3pLibs/keyValueHash/hashtable.c -o bin/IndexerLot

IndexerLotbb: src/IndexerLot/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) -D BLACK_BOKS -D PRESERVE_WORDS src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/IndexerLotbb -DIIACL

baddsPageAnalyser: src/baddsPageAnalyser/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerRes/IndexerRes.c src/baddsPageAnalyser/main.c  src/httpGet/httpGet.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/baddsPageAnalyser $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(CURLLIBS) -DDEBUG_ADULT

rreadWithRank: src/rreadWithRank/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rreadWithRank/main.c  -o bin/rreadWithRank $(LDFLAGS) 

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

	$(CC) $(CFLAGS) $(LIBS)*.c src/dictionarywordsLot/main.c src/dictionarywordsLot/set.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/dictionarywordsLot $(LDFLAGS)

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
	$(CC) $(CFLAGS) $(LIBS)*.c src/maincfg/maincfg.c src/infoquery/main.c src/acls/acls.c src/crawlManager/client.c src/boithoadClientLib/boithoadClientLib.c $(BBDOCUMENT) -o bin/infoquery $(LDFLAGS) $(LIBCONFIG)

GetIndexAsArrayTest: src/GetIndexAsArrayTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/GetIndexAsArrayTest/main.c -o bin/GetIndexAsArrayTest $(LDFLAGS)

bbdocumentWebAdd: src/bbdocumentWebAdd/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentWebAdd/main.c src/base64/base64.c -o bin/bbdocumentWebAdd $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS

bbdocumentMakeLotUrlsdb: src/bbdocumentMakeLotUrlsdb/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentMakeLotUrlsdb/main.c -o bin/bbdocumentMakeLotUrlsdb $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS

bbdocumentConvertTest: src/bbdocumentConvertTest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentConvertTest/main.c -o bin/bbdocumentConvertTest $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS


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
	#for lokalt på bb: gcc -g src/common/*.c src/boithoad/main.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/boithoad -lm -lz -D_FILE_OFFSET_BITS=64 -O2 -DIIACL -DWITH_OPENLDAP /usr/lib64/libcrypt.a  /usr/lib64/libssl.a -I/usr/include/mysql/ -L/usr/lib64/mysql/ -ldl   -D BLACK_BOKS -D WITH_CONFIG -DDEBUG ../../openldap-2.3.32/libraries/libldap/.libs/libldap.a ../../openldap-2.3.32/libraries/liblber/.libs/liblber.a -lmysqlclient -lsasl2
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoad/main.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/boithoad $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS -D WITH_CONFIG -DDEBUG

PiToWWWDocID: src/PiToWWWDocID/main.c
	@echo ""
	@echo "$@:"
	$(CC) $(CFLAGS) $(LIBS)*.c src/PiToWWWDocID/main.c src/UrlToDocID/search_index.c -o bin/PiToWWWDocID $(LDFLAGS)  $(MYSQL) $(BDB)


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

	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate5DocFork/main.c  -o bin/BrankCalculate5DocFork $(LDFLAGS)


vipurls: src/vipurls/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/3pLibs/keyValueHash/hashtable.c src/vipurls/main.c  -o bin/vipurls $(LDFLAGS)

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

	

testGetNextLotForIndex: src/testGetNextLotForIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/testGetNextLotForIndex/main.c  -o bin/testGetNextLotForIndex $(LDFLAGS)

everrun: src/everrun/catchdump.c
	@echo ""
	@echo "$@:"

	$(CC) src/everrun/catchdump.c -o bin/everrun

searchcl : src/searchkernel/searchcl.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/query/lex.query.o src/searchkernel/cgi-util.c src/searchkernel/parseEnv.c src/searchkernel/searchkernel.c src/searchkernel/search.c src/searchkernel/searchcl.c src/parse_summary/libsummary.a -o bin/searchcl $(LDFLAGS)

#dropper -D WITH_MEMINDEX og -D WITH_RANK_FILTER for nå
#SEARCHCOMMAND = $(CFLAGS) $(LIBS)*.c src/query/lex.query.c src/3pLibs/keyValueHash/hashtable.c src/3pLibs/keyValueHash/hashtable_itr.c src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c src/parse_summary/libsummary.a src/parse_summary/libhighlight.a  $(LDFLAGS) -lpthread -D WITH_THREAD $(LIBCONFIG)
SEARCHCOMMAND = $(CFLAGS) $(LIBS)*.c src/maincfg/maincfg.c src/searchkernel/shortenurl.c src/query/lex.query.o src/3pLibs/keyValueHash/hashtable.c src/3pLibs/keyValueHash/hashtable_itr.c src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c $(HTMLPARSER) src/generateSnippet/libsnippet_generator.a  src/ds/libds.a src/utf8-filter/lex.u8fl.o $(LDFLAGS) -lpthread $(LIBCONFIG) -D EXPLAIN_RANK


searchddep:
	#ting searchd trenger
	@echo ""
	@echo "$@:"
	for i in src/query src/parser src/generateSnippet src/ds src/utf8-filter src/getdate; do\
           (cd $$i; $(MAKE) all);\
        done

searchd : src/searchkernel/searchd.c
	@echo ""
	@echo "$@:"
	
	$(CC) $(SEARCHCOMMAND) -D WITH_RANK_FILTER -D WITH_THREAD -D DEFLOT -o bin/searchd 

searchdbb : src/searchkernel/searchd.c
	@echo ""
	@echo "$@:"
	$(CC) $(SEARCHCOMMAND) $(BDB) src/getdate/dateview.c src/crawlManager/client.c src/boithoadClientLib/boithoadClientLib.c -D BLACK_BOKS -o bin/searchdbb src/getdate/getdate.tab.o src/getFiletype/getfiletype.o src/ds/libds.a -DIIACL

mergeUserToSubname: src/mergeUserToSubname/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/mergeUserToSubname/main.c src/acls/acls.c -o bin/mergeUserToSubname $(LDFLAGS) -DBLACK_BOKS $(BDB)

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

dispatcherCOMAND = $(CFLAGS) $(LIBS)*.c src/UrlToDocID/search_index.c src/maincfg/maincfg.c src/dispatcher_all/main.c src/tkey/tkey.c src/cgi-util/cgi-util.c src/searchFilters/searchFilters.c -D EXPLAIN_RANK $(LDFLAGS) $(BDB) -D_GNU_SOURCE

dispatcher_all: src/dispatcher_all/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(MYSQL4) $(LIBGeoIP) -D WITH_CASHE -o cgi-bin/dispatcher_all $(LIBCONFIG)

dispatcher_allbb: src/dispatcher_all/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(MYSQL) -D BLACK_BOKS -o cgi-bin/dispatcher_allbb $(LIBCONFIG)


dispatcher: src/dispatcher/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/dispatcher/main.c src/cgi-util/cgi-util.c -o bin/dispatcher $(LDFLAGS)

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


rread : src/rread/rread.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rread/rread.c -o bin/rread $(LDFLAGS)

rreadbb : src/rread/rread.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/rread/rread.c -o bin/rreadbb $(LDFLAGS) -D BLACK_BOKS


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

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker/main.c -o bin/LotInvertetIndexMaker $(LDFLAGS)

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

	$(CC) $(CFLAGS) $(LIBS)*.c src/recoverUrlForLot/main.c -o bin/recoverUrlForLot $(LDFLAGS)

removeUnnecessaryRevindex: src/removeUnnecessaryRevindex/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/removeUnnecessaryRevindex/main.c -o bin/removeUnnecessaryRevindex $(LDFLAGS)

LotInvertetIndexMaker2:	src/LotInvertetIndexMaker2/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker2/main.c -o bin/LotInvertetIndexMaker2 $(LDFLAGS)

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

readNyeUrlerFil: src/readNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readNyeUrlerFil/main.c  -o bin/readNyeUrlerFil $(LDFLAGS)

filterNyeUrlerFil: src/filterNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/filterNyeUrlerFil/main.c  -o bin/filterNyeUrlerFil $(LDFLAGS)

addUrlsToIndex: src/addUrlsToIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addUrlsToIndex/main.c -o bin/addUrlsToIndex $(LDFLAGS)

readLinkFile: src/readLinkFile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readLinkFile/main.c -o bin/readLinkFile $(LDFLAGS)

mergeLinkDB:
	(cd src/mergeLinkDB/; make)
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
	$(CC) $(CFLAGS) $(LIBS)*.c src/3pLibs/keyValueHash/hashtable.c src/BrankCalculate3/main.c src/BrankCalculate3/res.c -o bin/BrankCalculate3 $(LDFLAGS) -DDEFLOT

BrankCalculate4: src/BrankCalculate4/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/bs/bs.c src/tq/tq.c src/3pLibs/keyValueHash/hashtable.c src/BrankCalculate4/main.c src/BrankCalculate3/res.c -o bin/BrankCalculate4 -lpthread $(LDFLAGS) -DDEFLOT -DWITH_THREAD

BrankCalculate5: src/BrankCalculate5/main.c
	$(CC) $(CFLAGS) src/common/lot.c src/common/bstr.c src/common/strlcat.c src/common/bfileutil.c src/common/debug.c  src/banlists/ban.c src/bs/bs.c src/tq/tq.c src/3pLibs/keyValueHash/hashtable.c src/BrankCalculate5/main.c -o bin/BrankCalculate5 -lpthread $(LDFLAGS) -DDEFLOT

BrankCalculate5Expand: src/BrankCalculate5Expand/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate3/res.c src/BrankCalculate5Expand/main.c -o bin/BrankCalculate5Expand $(LDFLAGS) -DDEFLOT 

BrankCalculateBanning: src/BrankCalculateBanning/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/3pLibs/keyValueHash/hashtable.c src/BrankCalculateBanning/main.c  -o bin/BrankCalculateBanning $(LDFLAGS) -DDEFLOT

BrankMerge: src/BrankMerge/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankMerge/*.c -o bin/BrankMerge $(LDFLAGS)

readLinkDB: src/readLinkDB/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readLinkDB/main.c -o bin/readLinkDB $(LDFLAGS)

SortUdfile: src/SortUdfile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SortUdfile/main.c -o bin/SortUdfile $(LDFLAGS)

SortUdfileToNewFiles: src/SortUdfileToNewFiles/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SortUdfileToNewFiles/main.c -o bin/SortUdfileToNewFiles $(LDFLAGS)

PageInfoComand=	$(LIBS)*.c src/PageInfo/main.c $(HTMLPARSER) $(LDFLAGS)

PageInfo: src/PageInfo/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) -o bin/PageInfo

PageInfobb: src/PageInfo/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) -o bin/PageInfobb -D BLACK_BOKS

addManuellUrlFile: src/addManuellUrlFile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/addManuellUrlFile/main.c  -o bin/addManuellUrlFile $(LDFLAGS)

dumpLotAsFiles: src/dumpLotAsFiles/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/dumpLotAsFiles/main.c -o bin/dumpLotAsFiles $(LDFLAGS)

boithold: src/boithold/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/boithold/getpath.c src/boithold/main.c -o bin/boithold $(LDFLAGS)

boitho-bbdn: src/boitho-bbdn/bbdnserver.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c  src/boitho-bbdn/bbdnserver.c src/maincfg/maincfg.c -o bin/boitho-bbdn $(LDFLAGS) $(BBDOCUMENT) -D BLACK_BOKS $(BBDOCUMENT_IMAGE) -static $(LIBCONFIG) -DIIACL


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

	$(CC) $(CFLAGS) $(LIBS)*.c src/UrlToDocIDIndexer/main.c -o bin/UrlToDocIDIndexer $(LDFLAGS) $(BDB)

UrlToDocIDQuery: src/UrlToDocIDQuery/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/getDocIDFromUrl/getDocIDFromUrl.c src/UrlToDocIDQuery/main.c -o bin/UrlToDocIDQuery $(LDFLAGS) $(BDB)

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

DIrecrawlSelect: src/DIrecrawlSelect/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIrecrawlSelect/main.c -o bin/DIrecrawlSelect $(LDFLAGS)

gcidentify: src/gcidentify/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/3pLibs/keyValueHash/hashtable.c src/banlists/ban.c src/gcidentify/main.c -o bin/gcidentify $(LDFLAGS)

DIcreateUdfile: src/DIcreateUdfile/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/DIcreateUdfile/main.c -o bin/DIcreateUdfile $(LDFLAGS)


BrankCalculatePI: src/BrankCalculatePI/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/getDocIDFromUrl/getDocIDFromUrl.c src/BrankCalculatePI/main.c -o bin/BrankCalculatePI $(BDB) $(LDFLAGS)

resolveRedirects: src/resolveRedirects/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/resolveRedirects/main.c src/getDocIDFromUrl/getDocIDFromUrl.c -o bin/resolveRedirects $(LDFLAGS) $(BDB)

redirResource: src/resolveRedirects/redirResource.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(LIBS)*.c src/resolveRedirects/redirResource.c src/getDocIDFromUrl/getDocIDFromUrl.c -o bin/redirResource $(LDFLAGS) $(BDB)

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
	(cd src/InitServices/; make)

	cp src/InitServices/initwrapper setuid/

YumWrapper: src/YumWrapper/yumwrapper.c
	(cd src/YumWrapper; make)
	cp src/YumWrapper/yumwrapper setuid/

NetConfig: src/NetConfig/configwrite.c
	(cd src/NetConfig; make)
	cp src/NetConfig/configwrite setuid/

yumupdate:
	$(CC) $(CFLAGS) src/common/exeoc.c src/yumupdate/yumupdate.c -o setuid/yumupdate

crawlManager: src/crawlManager/main.c
	@echo ""
	@echo "$@:"

	#22 feb 2007, fjerner -static
	$(CC) $(CFLAGS) $(LIBS)*.c src/maincfg/maincfg.c src/crawl/crawl.c src/boitho-bbdn/bbdnclient.c src/crawlManager/main.c src/3pLibs/keyValueHash/hashtable.c -o bin/crawlManager $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS $(BBDOCUMENT) $(LIBCONFIG) -DIIACL


crawlSMB: src/crawlSMB/main.c
	@echo ""
	@echo "$@:"

	flex -f -8 -i -o src/crawlSMB/lex.acl.c src/crawlSMB/acl.parser.l


	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g -Wl,-static $(LIBS)*.c src/crawlSMB/cleanresource.c src/crawlSMB/scan.c src/crawlSMB/lex.acl.c src/crawlSMB/crawlsmb.c src/crawl/crawl.c src/crawlSMB/main.c src/boitho-bbdn/bbdnclient.c -o src/crawlSMB/crawlSMB.so $(LDFLAGS) $(SMBCLIENT)
	mkdir -p crawlers/crawlSMB
	cp src/crawlSMB/crawlSMB.so crawlers/crawlSMB/

crawlExchange:
	@echo ""
	@echo "$@:"

	(cd src/crawlExchange; make clean)
	(cd src/crawlExchange; make)


crawl247:
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g -Wl,-static $(LIBS)*.c src/crawl247/crawl.c src/crawl/crawl.c src/boitho-bbdn/bbdnclient.c -o src/crawl247/crawl247.so $(LDFLAGS) $(SMBCLIENT)
	mkdir -p crawlers/crawl247/
	cp src/crawl247/crawl247.so crawlers/crawl247/
	@#(cd src/crawl247; make clean)
	@#(cd src/crawl247; make)



crawlSFTP: src/crawlSFTP/crawlsftp.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -fPIC -shared -o src/crawlSFTP/crawlSFTP.so src/crawlSFTP/crawlsftp.c src/crawlSFTP/rutines.c src/crawl/crawl.c src/3pLibs/libssh2/src/*.o -g -O2 -I/usr/include -I/usr/include -Isrc/3pLibs/libssh2/include/ -L/usr/lib -lcrypto -L/usr/lib -lz
	mkdir -p crawlers/crawlSFTP
	cp src/crawlSFTP/crawlSFTP.so crawlers/crawlSFTP/


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

