# Makefile

# The compiler to be used
CC = /usr/local/bin/gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS = -g

# Used to add debugging to boitho-bbdn and crawlManager
WITHDEBUG = #-DDEBUG

# The dynamic libraries that the executable needs to be linked to
# fjerner -ldb -static. Må legge dette til der de skal være
LDFLAGS = -lm -lz -D_FILE_OFFSET_BITS=64 -O2

#pop rank bibloteket
LIBS = src/common/libcommon.a

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
SMBCLIENT=/home/boitho/src/samba-3.0.24/source/bin/libsmbclient.a -I/home/boitho/src/samba-3.0.24/source/include/
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

MYSQL_THREAD = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

#LIBXML = -I/usr/include/libxml2 -L/usr/lib -lxml2
LIBXML = -I/usr/include/libxml2  -lxml2


HTMLPARSER=src/parser/lex.bhpm.c src/parser/y.tab.c  
#HTMLPARSER=src/parser/libhtml_parser.a

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

bb : src/common/libcommon.a getFileType searchddep searchdbb dispatcher_allbb crawlManager infoquery crawlSMB boitho-bbdn PageInfobb boitho-bbdn IndexerLotbb LotInvertetIndexMaker2  mergeIIndex mergeUserToSubname bbdocumentConvertTest ShowThumbbb everrun dictionarywordsLot webadmindep

webadmindep: YumWrapper NetConfig InitServices setuidcaller yumupdate

tempFikes: IndexerLot_fik32bitbug DIconvert


wordConverter: src/wordConverter/main.c
	$(CC) src/wordConverter/main.c -o bin/wordConverter


getFileType:
	(cd src/getFiletype/; make)

#brukte før src/parser/libhtml_parser.a, byttet til src/parser/lex.yy.c src/parser/lex.yy.c slik at vi kan bruke gdb
IndexerLot= $(CFLAGS) $(LIBS) src/IndexerRes/IndexerRes.c src/IndexerLot/main.c src/searchFilters/searchFilters.c $(HTMLPARSER) $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS


src/common/libcommon.a:
	(cd src/common; make clean)
	(cd src/common; make)

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


Suggest:
	@echo ""
	@echo "$@:"

	(cd src/suggestrpc; make)
	cp src/suggestrpc/suggest_server bin/
	(cd src/suggestwebclient; make)
	cp src/suggestwebclient/suggest_webclient bin/

IndexerLot: src/IndexerLot/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) -lpthread -DWITH_THREAD -o bin/IndexerLot $(LIBS)

IndexerLotbb: src/IndexerLot/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(IndexerLot) -D BLACK_BOKS -D PRESERVE_WORDS src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/IndexerLotbb $(LIBS)

baddsPageAnalyser: src/baddsPageAnalyser/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerRes/IndexerRes.c src/baddsPageAnalyser/main.c  src/httpGet/httpGet.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/baddsPageAnalyser $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(CURLLIBS) -DDEBUG_ADULT $(LIBS)

rreadWithRank: src/rreadWithRank/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/rreadWithRank/main.c  -o bin/rreadWithRank $(LDFLAGS)  $(LIBS)

IndexerLot_langtest: src/IndexerLot_langtest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLot_langtest/main.c  src/parser/lex.yy.c src/parser/y.tab.c -o bin/IndexerLot_langtest $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS -DDEBUG_ADULT $(LIBS)
	

IndexerLot_fik32bitbug: src/IndexerLot_fik32bitbug/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLot_fik32bitbug/main.c  src/parser/libhtml_parser.a -o bin/IndexerLot_fik32bitbug $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(LIBS)

IndexerLot_getno: src/IndexerLot_getno/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLot_getno/main.c  src/parser/lex.yy.c src/parser/lex.yy.c -o bin/IndexerLot_getno $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(LIBS)

dictionarywordsLot: src/dictionarywordsLot/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/dictionarywordsLot/main.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/dictionarywordsLot $(LDFLAGS) $(LIBS)

lotlistDispatcher: src/lotlistDispatcher/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/lotlistDispatcher/main.c -o bin/lotlistDispatcher $(LDFLAGS) $(LIBS)

searchfilterTest: src/searchfilterTest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/searchfilterTest/main.c src/searchfilter/searchfilter.c -o bin/searchfilterTest $(LDFLAGS) $(LIBS)

infoquery: src/infoquery/main.c $(LIBS)
	@echo ""
	@echo "$@:"
	$(CC) $(CFLAGS) src/maincfg/maincfg.c src/infoquery/main.c src/acls/acls.c src/crawlManager/client.c src/boithoadClientLib/boithoadClientLib.c $(BBDOCUMENT) -o bin/infoquery $(LDFLAGS) $(LIBCONFIG) $(LIBS)

GetIndexAsArrayTest: src/GetIndexAsArrayTest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/GetIndexAsArrayTest/main.c -o bin/GetIndexAsArrayTest $(LDFLAGS) $(LIBS)

bbdocumentWebAdd: src/bbdocumentWebAdd/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/bbdocumentWebAdd/main.c src/base64/base64.c -o bin/bbdocumentWebAdd $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS $(LIBS)

bbdocumentMakeLotUrlsdb: src/bbdocumentMakeLotUrlsdb/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/bbdocumentMakeLotUrlsdb/main.c -o bin/bbdocumentMakeLotUrlsdb $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS $(LIBS)

bbdocumentConvertTest: src/bbdocumentConvertTest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/bbdocumentConvertTest/main.c -o bin/bbdocumentConvertTest $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS $(LIBS)


analyseShortRank: src/analyseShortRank/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/analyseShortRank/main.c -o bin/analyseShortRank $(LDFLAGS)

DIconvert: src/DIconvert/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/DIconvert/main.c -o bin/DIconvert $(LDFLAGS) $(LIBS)

boithoad: src/boithoad/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithoad/main.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/boithoad $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS -D WITH_CONFIG -DDEBUG $(LIBS)

boithoadtest: src/boithoadtest/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithoadtest/main.c src/boithoadClientLib/liboithoaut.a -o bin/boithoadtest $(LDFLAGS)


lotlistPrint: src/lotlistPrint/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/lotlistPrint/main.c  -o bin/lotlistPrint $(LDFLAGS) $(LIBS)

cleanCacheDir: src/cleanCacheDir/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/cleanCacheDir/main.c -o bin/cleanCacheDir

lotcp: src/lotcp/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/lotcp/main.c  -o bin/lotcp $(LDFLAGS) $(LIBS)

missinglotDetectLocal: src/missinglotDetectLocal/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/missinglotDetectLocal/main.c  -o bin/missinglotDetectLocal $(LDFLAGS) $(LIBS)

missinglotRemoveFormUdfile: src/missinglotRemoveFormUdfile/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/missinglotRemoveFormUdfile/main.c  -o bin/missinglotRemoveFormUdfile $(LDFLAGS) $(LIBS)

missinglotGetFormUdfile: src/missinglotGetFormUdfile/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/missinglotGetFormUdfile/main.c  -o bin/missinglotGetFormUdfile $(LDFLAGS) $(LIBS)

sortLinkdb: src/linkdbTools/sortLinkdb.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/linkdbTools/sortLinkdb.c  -o bin/sortLinkdb $(LDFLAGS) $(LIBS)

BrankCalculate2GetPageElements: src/BrankCalculate2GetPageElements/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/BrankCalculate2GetPageElements/main.c  -o bin/BrankCalculate2GetPageElements $(LDFLAGS) $(LIBS)


BrankCalculate2Publish: src/BrankCalculate2Publish/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/BrankCalculate2Publish/main.c  -o bin/BrankCalculate2Publish $(LDFLAGS) $(LIBS)


vipurls: src/vipurls/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/3pLibs/keyValueHash/hashtable.c src/vipurls/main.c  -o bin/vipurls $(LDFLAGS) $(LIBS)

urldispatcher: src/urldispatcher/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/urldispatcher/main.c  -o bin/urldispatcher $(LDFLAGS) $(LIBS)

urlsDocumentIndexadd: src/urlsDocumentIndexadd/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/urlsDocumentIndexadd/main.c  -o bin/urlsDocumentIndexadd -D DI_FILE_CASHE $(LDFLAGS) $(LIBS)

netlotStart: src/netlotStart/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/netlotStart/main.c  -o bin/netlotStart $(LDFLAGS) $(LIBS)

netlotEnd: src/netlotEnd/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/netlotEnd/main.c  -o bin/netlotEnd $(LDFLAGS) $(LIBS)

LotNyeurlerSort: src/LotNyeurlerSort/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/LotNyeurlerSort/main.c  -o bin/LotNyeurlerSort $(LDFLAGS) $(LIBS)

LotNyeurlerMove: src/LotNyeurlerMove/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/LotNyeurlerMove/main.c  -o bin/LotNyeurlerMove $(LDFLAGS) $(LIBS)

crawlFiles: src/crawlFiles/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/crawlFiles/main.c  -o bin/crawlFiles $(LDFLAGS) src/boitho-bbdn/bbdnclient.c -D BLACK_BOKS $(LIBS)

	

testGetNextLotForIndex: src/testGetNextLotForIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/testGetNextLotForIndex/main.c  -o bin/testGetNextLotForIndex $(LDFLAGS) $(LIBS)

everrun: src/everrun/catchdump.c
	@echo ""
	@echo "$@:"

	$(CC) src/everrun/catchdump.c -o bin/everrun

searchcl : src/searchkernel/searchcl.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/query/lex.query.o src/searchkernel/cgi-util.c src/searchkernel/parseEnv.c src/searchkernel/searchkernel.c src/searchkernel/search.c src/searchkernel/searchcl.c src/parse_summary/libsummary.a -o bin/searchcl $(LDFLAGS) $(LIBS)

#dropper -D WITH_MEMINDEX og -D WITH_RANK_FILTER for n
#SEARCHCOMMAND = $(CFLAGS) src/query/lex.query.c src/3pLibs/keyValueHash/hashtable.c src/3pLibs/keyValueHash/hashtable_itr.c src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c src/parse_summary/libsummary.a src/parse_summary/libhighlight.a  $(LDFLAGS) -lpthread -D WITH_THREAD $(LIBCONFIG) $(LIBS)
SEARCHCOMMAND = $(CFLAGS) src/maincfg/maincfg.c src/searchkernel/shortenurl.c src/query/lex.query.o src/3pLibs/keyValueHash/hashtable.c src/3pLibs/keyValueHash/hashtable_itr.c src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c $(HTMLPARSER) src/generateSnippet/libsnippet_generator.a  src/ds/libds.a src/utf8-filter/lex.u8fl.o $(LDFLAGS) -lpthread $(LIBCONFIG) -D EXPLAIN_RANK


searchddep:
	#ting searchd trenger
	@echo ""
	@echo "$@:"
	for i in src/query src/parser src/generateSnippet src/ds src/utf8-filter src/getdate; do\
           (cd $$i; $(MAKE) all);\
        done

searchd : src/searchkernel/searchd.c $(LIBS)
	@echo ""
	@echo "$@:"
	
	$(CC) $(SEARCHCOMMAND) -D WITH_RANK_FILTER -D WITH_THREAD -D DEFLOT -o bin/searchd  $(LIBS)

searchdbb : src/searchkernel/searchd.c $(LIBS)
	@echo ""
	@echo "$@:"
	$(CC) $(SEARCHCOMMAND) $(BDB) src/getdate/dateview.c src/crawlManager/client.c src/boithoadClientLib/boithoadClientLib.c -D BLACK_BOKS -o bin/searchdbb src/getdate/getdate.tab.o src/getFiletype/getfiletype.o src/ds/libds.a $(LIBS)

mergeUserToSubname: src/mergeUserToSubname/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/mergeUserToSubname/main.c src/acls/acls.c -o bin/mergeUserToSubname $(LDFLAGS) -DBLACK_BOKS $(BDB) $(LIBS)

boithoads: src/boithoads/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS) -D EXPLAIN_RANK $(LIBS)

boithoadshread: src/boithoads/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL_THREAD) $(LIBXML) $(CURLLIBS) -D WITH_THREAD -lpthread $(LIBS)

addout.cgi: src/addout.cgi/main.c
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/addout.cgi/main.c src/cgi-util/cgi-util.c -o bin/addout.cgi $(LDFLAGS) $(MYSQL) 

ppcXmlParserTest: src/ppcXmlParserTest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/ppcXmlParserTest/main.c src/parse_summary/libsummary.a src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/parse_summary/libsummary.a src/httpGet/httpGet.c -o bin/ppcXmlParserTest $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS) $(LIBS)

dispatcherCOMAND = $(CFLAGS) src/maincfg/maincfg.c src/dispatcher_all/main.c src/tkey/tkey.c src/cgi-util/cgi-util.c src/searchFilters/searchFilters.c -D EXPLAIN_RANK $(LDFLAGS) $(MYSQL)

dispatcher_all: src/dispatcher_all/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) $(LIBGeoIP) -D WITH_CASHE -o cgi-bin/dispatcher_all $(LIBCONFIG) $(LIBS)

dispatcher_allbb: src/dispatcher_all/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(dispatcherCOMAND) -D BLACK_BOKS -o cgi-bin/dispatcher_allbb $(LIBCONFIG) $(LIBS)


dispatcher: src/dispatcher/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/dispatcher/main.c src/cgi-util/cgi-util.c -o bin/dispatcher $(LDFLAGS) $(LIBS)

putFilesIntoFileTree: src/putFilesIntoFileTree/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/putFilesIntoFileTree/main.c -o bin/putFilesIntoFileTree $(LDFLAGS) $(LIBS)

nyeurlerIntegeryCheck: src/nyeurlerIntegeryCheck/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/nyeurlerIntegeryCheck/main.c -o bin/nyeurlerIntegeryCheck $(LDFLAGS) $(LIBS)

rread : src/rread/rread.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/rread/rread.c -o bin/rread $(LDFLAGS) $(LIBS)


convertReposetoryCOMAND = $(CFLAGS) src/convertReposetory/main.c -o bin/convertReposetory $(LDFLAGS) $(LIBS)

convertReposetory: src/convertReposetory/main.c $(LIBS)
	$(CC) $(convertReposetoryCOMAND)

convertReposetorybb: src/convertReposetory/main.c
	$(CC) $(convertReposetoryCOMAND) -D BLACK_BOKS


makeSumaryCashe: src/makeSumaryCashe/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/makeSumaryCashe/main.c src/parse_summary/libsummary.a -o bin/makeSumaryCashe $(LDFLAGS) -D NOWARNINGS $(LIBS)

rread_getsomeurls: src/rread_getsomeurls/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/rread_getsomeurls/main.c -o bin/rread_getsomeurls $(LDFLAGS) $(LIBS)

IndexerLotLite: src/IndexerLotLite/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLotLite/main.c -o bin/IndexerLotLite $(LDFLAGS) $(LIBS)

readMainIndex: src/readMainIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/readMainIndex/main.c -o bin/readMainIndex $(LDFLAGS) $(LIBS)

readMainIndexSearchSha1: src/readMainIndexSearchSha1/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/readMainIndexSearchSha1/main.c -o bin/readMainIndexSearchSha1 $(LDFLAGS) $(LIBS)

LotInvertetIndexMaker: src/LotInvertetIndexMaker/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/LotInvertetIndexMaker/main.c -o bin/LotInvertetIndexMaker $(LDFLAGS) $(LIBS)

LotInvertetIndexMakerSplice: src/LotInvertetIndexMakerSplice/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/LotInvertetIndexMakerSplice/main.c -o bin/LotInvertetIndexMakerSplice $(LDFLAGS) $(LIBS)

listLostLots: src/listLostLots/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/listLostLots/main.c -o bin/listLostLots $(LDFLAGS) $(LIBS)

recoverUrlForLot: src/recoverUrlForLot/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/recoverUrlForLot/main.c -o bin/recoverUrlForLot $(LDFLAGS) $(LIBS)

removeUnnecessaryRevindex: src/removeUnnecessaryRevindex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/removeUnnecessaryRevindex/main.c -o bin/removeUnnecessaryRevindex $(LDFLAGS) $(LIBS)

LotInvertetIndexMaker2:	src/LotInvertetIndexMaker2/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/LotInvertetIndexMaker2/main.c -o bin/LotInvertetIndexMaker2 $(LDFLAGS) $(LIBS)

ThumbnaleDemon: src/ThumbnaleDemon/main.c $(LIBS)
	$(CC) $(CFLAGS) src/ThumbnaleDemon/main.c -o bin/ThumbnaleDemon $(LDFLAGS) $(LIBS)

PoprankMerge: src/PoprankMerge/poptorank.c $(LIBS)
	$(CC) $(CFLAGS) src/PoprankMerge/poptorank.c -o bin/PoprankMerge $(LDFLAGS) $(LIBS)

ipbann: src/ipbann/main.c $(LIBS)
	$(CC) $(CFLAGS) src/ipbann/main.c  -o bin/ipbann $(LDFLAGS) $(LIBS)

addanchors: src/addanchors/main.c $(LIBS)
	$(CC) $(CFLAGS) src/addanchors/main.c -o bin/addanchors $(LDFLAGS) $(LIBS)

IndexerLotAnchors: src/IndexerLotAnchors/main.c $(LIBS)
	$(CC) $(CFLAGS) src/IndexerLotAnchors/main.c -o bin/IndexerLotAnchors $(LDFLAGS) $(LIBS)

readNyeUrlerFil: src/readNyeUrlerFil/main.c $(LIBS)
	$(CC) $(CFLAGS) src/readNyeUrlerFil/main.c  -o bin/readNyeUrlerFil $(LDFLAGS) $(LIBS)

filterNyeUrlerFil: src/filterNyeUrlerFil/main.c $(LIBS)
	$(CC) $(CFLAGS) src/filterNyeUrlerFil/main.c  -o bin/filterNyeUrlerFil $(LDFLAGS) $(LIBS)

addUrlsToIndex: src/addUrlsToIndex/main.c $(LIBS)
	$(CC) $(CFLAGS) src/addUrlsToIndex/main.c -o bin/addUrlsToIndex $(LDFLAGS) $(LIBS)

readLinkFile: src/readLinkFile/main.c $(LIBS)
	$(CC) $(CFLAGS) src/readLinkFile/main.c -o bin/readLinkFile $(LDFLAGS) $(LIBS)

mergeLinkDB:
	(cd src/mergeLinkDB/; make)
	cp src/mergeLinkDB/mergeLinkDB bin/mergeLinkDB

anchorread: src/anchorread/main.c $(LIBS)
	$(CC) $(CFLAGS) src/anchorread/main.c -o bin/anchorread $(LDFLAGS) $(LIBS)

BrankCalculate:	src/BrankCalculate/main.c $(LIBS)
	$(CC) $(CFLAGS) src/BrankCalculate/*.c -o bin/BrankCalculate $(LDFLAGS) $(LIBS)

BrankCalculate2: src/BrankCalculate2/main.c $(LIBS)
	$(CC) $(CFLAGS) src/BrankCalculate2/*.c -o bin/BrankCalculate2 $(LDFLAGS) $(LIBS)

BrankCalculate3: src/BrankCalculate3/main.c $(LIBS)
	$(CC) $(CFLAGS) src/3pLibs/keyValueHash/hashtable.c src/BrankCalculate3/*.c -o bin/BrankCalculate3 $(LDFLAGS) $(LIBS)

BrankMerge: src/BrankMerge/main.c $(LIBS)
	$(CC) $(CFLAGS) src/BrankMerge/*.c -o bin/BrankMerge $(LDFLAGS) $(LIBS)

readLinkDB: src/readLinkDB/main.c $(LIBS)
	$(CC) $(CFLAGS) src/readLinkDB/main.c -o bin/readLinkDB $(LDFLAGS) $(LIBS)

SortUdfile: src/SortUdfile/main.c $(LIBS)
	$(CC) $(CFLAGS) src/SortUdfile/main.c -o bin/SortUdfile $(LDFLAGS) $(LIBS)

SortUdfileToNewFiles: src/SortUdfileToNewFiles/main.c $(LIBS)
	$(CC) $(CFLAGS) src/SortUdfileToNewFiles/main.c -o bin/SortUdfileToNewFiles $(LDFLAGS) $(LIBS)

PageInfoComand= src/PageInfo/main.c src/parser/lex.bhpm.c src/parser/y.tab.c $(LDFLAGS)

PageInfo: src/PageInfo/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) -o bin/PageInfo $(LIBS)

PageInfobb: src/PageInfo/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) $(PageInfoComand) -o bin/PageInfobb -D BLACK_BOKS $(LIBS)

addManuellUrlFile: src/addManuellUrlFile/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/addManuellUrlFile/main.c  -o bin/addManuellUrlFile $(LDFLAGS) $(LIBS)

dumpLotAsFiles: src/dumpLotAsFiles/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/dumpLotAsFiles/main.c -o bin/dumpLotAsFiles $(LDFLAGS) $(LIBS)

boithold: src/boithold/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boithold/getpath.c src/boithold/main.c -o bin/boithold $(LDFLAGS) $(LIBS)

boitho-bbdn: src/boitho-bbdn/bbdnserver.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS)  src/boitho-bbdn/bbdnserver.c src/maincfg/maincfg.c -o bin/boitho-bbdn $(BBDOCUMENT) -D BLACK_BOKS $(BBDOCUMENT_IMAGE) -static $(LIBCONFIG) $(WITHDEBUG) $(LIBS) $(LDFLAGS)


boitholdTest: src/boitholdTest/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/boitholdTest/main.c -o bin/boitholdTest $(LDFLAGS) $(LIBS)

SplittUdfileByLot: src/SplittUdfileByLot/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/SplittUdfileByLot/main.c -o bin/SplittUdfileByLot $(LDFLAGS) $(LIBS)

UrlToDocIDIndexer: src/UrlToDocIDIndexer/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/UrlToDocIDIndexer/main.c -o bin/UrlToDocIDIndexer $(LDFLAGS) $(BDB) $(LIBS)

UrlToDocIDQuery: src/UrlToDocIDQuery/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/getDocIDFromUrl/getDocIDFromUrl.c src/UrlToDocIDQuery/main.c -o bin/UrlToDocIDQuery $(LDFLAGS) $(BDB) $(LIBS)

UrlToDocIDSplitUdfile: src/UrlToDocIDSplitUdfile/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/UrlToDocIDSplitUdfile/main.c -o bin/UrlToDocIDSplitUdfile $(LDFLAGS) $(LIBS)

addReposerotyToIndex: src/addReposerotyToIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/addReposerotyToIndex/main.c -o bin/addReposerotyToIndex $(LDFLAGS) $(LIBS)


readDocumentIndex: src/readDocumentIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/readDocumentIndex/main.c -o bin/readDocumentIndex $(LDFLAGS) $(LIBS)

readDocumentIndexWithRank: src/readDocumentIndexWithRank/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/readDocumentIndexWithRank/main.c -o bin/readDocumentIndexWithRank $(LDFLAGS) $(LIBS)

adultBuildIndex: src/adultBuildIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/adultBuildIndex/main.c -o bin/adultBuildIndex $(LDFLAGS) $(LIBS)

ipdbBuildLotIndex: src/ipdbBuildLotIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/ipdbBuildLotIndex/main.c -o bin/ipdbBuildLotIndex $(LDFLAGS) $(LIBS)

ipdbMakeMain: src/ipdbMakeMain/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/ipdbMakeMain/main.c -o bin/ipdbMakeMain $(LDFLAGS) $(LIBS)

IndexerLotUrl: src/IndexerLotUrl/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLotUrl/main.c -o bin/IndexerLotUrl $(LDFLAGS) $(LIBS)

IndexerLotDomainId: src/IndexerLotDomainId/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/IndexerLotDomainId/main.c -o bin/IndexerLotDomainId $(LDFLAGS) $(LIBS)

fixsAsciixBug: src/fixsAsciixBug/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/fixsAsciixBug/main.c -o bin/fixsAsciixBug $(LDFLAGS) $(LIBS)

getUncrawledPages: src/getUncrawledPages/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/getUncrawledPages/main.c -o bin/getUncrawledPages $(LDFLAGS) $(LIBS)

cpLotFile: src/cpLotFile/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/cpLotFile/main.c -o bin/cpLotFile $(LDFLAGS) $(LIBS)


SHOWTHUMBCMANDS = $(CFLAGS) src/ShowThumb/main.c src/cgi-util/cgi-util.c $(LDFLAGS)

ShowThumb: src/ShowThumb/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(SHOWTHUMBCMANDS) -o cgi-bin/ShowThumb $(LIBS)

ShowThumbbb: src/ShowThumb/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(SHOWTHUMBCMANDS) -o cgi-bin/ShowThumbbb -D BLACK_BOKS $(LIBS)

ShowCacheCOMMAND = $(CFLAGS) src/ShowCache/main.c src/cgi-util/cgi-util.c $(LDFLAGS) -o cgi-bin/ShowCache

ShowCache: src/ShowCache/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(ShowCacheCOMMAND) $(LIBS)

ShowCachebb: src/ShowCache/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(ShowCacheCOMMAND) -D BLACK_BOKS $(LIBS)

boithoshmd: src/boithoshmd/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/boithoshmd/main.c -o bin/boithoshmd $(LDFLAGS) $(LIBS)

builIpDB: src/builIpDB/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) -ldb src/builIpDB/main.c -o bin/builIpDB $(LDFLAGS) $(LIBS)

ConvertRankToShortRankFile: src/ConvertRankToShortRankFile/main.c
	@echo ""
	@echo "$@:"

	$(CC) src/ConvertRankToShortRankFile/main.c -lm -o bin/ConvertRankToShortRankFile

readIIndex: src/readIIndex/main.c
	@echo ""
	@echo "$@:"

	$(CC) src/readIIndex/main.c -o bin/readIIndex

mergeIIndex: src/mergeIIndex/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	$(CC) $(CFLAGS) src/mergeIIndex/main.c -o bin/mergeIIndex $(LDFLAGS) $(LIBS)


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

crawlManager: src/crawlManager/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	#22 feb 2007, fjerner -static
	$(CC) $(CFLAGS) src/maincfg/maincfg.c src/crawl/crawl.c src/boitho-bbdn/bbdnclient.c src/crawlManager/main.c src/3pLibs/keyValueHash/hashtable.c -o bin/crawlManager $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS $(BBDOCUMENT) $(LIBCONFIG) $(WITHDEBUG) $(LIBS)


crawlSMB: src/crawlSMB/main.c $(LIBS)
	@echo ""
	@echo "$@:"

	flex -f -8 -i -o src/crawlSMB/lex.acl.c src/crawlSMB/acl.parser.l


	$(CC) $(CFLAGS) -fPIC -shared -D BLACK_BOKS -g -Wl,-static src/crawlSMB/cleanresource.c src/crawlSMB/scan.c src/crawlSMB/lex.acl.c src/crawlSMB/crawlsmb.c src/crawl/crawl.c src/crawlSMB/main.c src/boitho-bbdn/bbdnclient.c -o src/crawlSMB/crawlSMB.so $(LDFLAGS) $(SMBCLIENT) $(LIBS)
	mkdir -p crawlers/crawlSMB
	cp src/crawlSMB/crawlSMB.so crawlers/crawlSMB/

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

