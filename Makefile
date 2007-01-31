# Makefile

# The compiler to be used
CC = /usr/local/bin/gcc

# Arguments passed to the compiler: -g causes the compiler to insert
# debugging info into the executable and -Wall turns on all warnings
CFLAGS = -g

# The dynamic libraries that the executable needs to be linked to
# fjerner -ldb -static. Må legge dette til der de skal være
LDFLAGS = -lm -lz -D_FILE_OFFSET_BITS=64 -O2

#pop rank bibloteket
LIBS = src/common/

LIBGeoIP = -lGeoIP

#bruker culr isteden da det er threadsafe
#LINWWWCFCLAGS = `libwww-config --cflags`
#LINWWWLIBS = `libwww-config --libs`

CURLLIBS = `curl-config --libs`

BBDOCUMENT = src/bbdocument/bbdocument.c -I/usr/local/BerkeleyDB.4.5/include/ -L/usr/local/BerkeleyDB.4.5/lib/ -ldb

#openldap med venner. Må linke det statisk inn, å bare bruke -lldap fungerer ikke
#
#LDAP = -DWITH_OPENLDAP /usr/lib/libldap.a -lsasl2 -lsasl -lcrypt -lssl
#LDAP = -DWITH_OPENLDAP -lldap -llber
LDAP = -DWITH_OPENLDAP /usr/lib/libldap.so.2 /usr/lib/liblber.so.2

#flag for å inkludere mysql
#MYSQL = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient
MYSQL = -I/usr/include/mysql /usr/lib/mysql/libmysqlclient.a

MYSQL_THREAD = -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient_r

#LIBXML = -I/usr/include/libxml2 -L/usr/lib -lxml2
LIBXML = -I/usr/include/libxml2  -lxml2

# The Dependency Rules
# They take the form
# target : dependency1 dependency2...
#        Command(s) to generate target from dependencies

# Dummy target that is processed by default. It specifies al list of
# other targets that are processed if make is invoked with no arguments
# However if you invoke make as "make output-data", it will only try to 
# generate the file output-data and its dependencies, not plot.png 
all : analyseShortRank makeSumaryCashe IndexerLotUrl LotInvertetIndexMaker getUncrawledPages fixsAsciixBug lotlistDispatcher dispatcher_all wordConverter filterNyeUrlerFil IndexerLot nyeurlerIntegeryCheck searchfilterTest lotlistPrint testGetNextLotForIndex everrun ipbann BrankMerge ShowCache mergeIIndex readIIndex cpLotFile readLinkDB builIpDB ConvertRankToShortRankFile boithoshmd readDocumentIndex ShowThumb UrlToDocIDSplitUdfile UrlToDocIDQuery UrlToDocIDIndexer boithold boitholdTest searchd searchcl addanchors readNyeUrlerFil anchorread PoprankMerge readLinkFile IndexerLotLite addUrlsToIndex rread ThumbnaleDemon dumpLotAsFiles webpublish addManuellUrlFile BrankCalculate SortUdfile PageInfo readMainIndex SplittUdfileByLot

tempFikes: IndexerLot_fik32bitbug DIconvert


wordConverter: src/wordConverter/main.c
	$(CC) src/wordConverter/main.c -o bin/wordConverter

#brukte før src/parser/libhtml_parser.a, byttet til src/parser/lex.yy.c src/parser/lex.yy.c slik at vi kan bruke gdb
IndexerLot= $(CFLAGS) $(LIBS)*.c src/IndexerRes/IndexerRes.c src/IndexerLot/main.c src/searchFilters/searchFilters.c src/parser/lex.bhpm.c src/parser/y.tab.c -o bin/IndexerLot $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

IndexerLot: src/IndexerLot/main.c
	$(CC) $(IndexerLot)

IndexerLotbb: src/IndexerLot/main.c
	$(CC) $(IndexerLot) -D BLACK_BOKS src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c

baddsPageAnalyser: src/baddsPageAnalyser/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerRes/IndexerRes.c src/baddsPageAnalyser/main.c  src/httpGet/httpGet.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/baddsPageAnalyser $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS $(CURLLIBS) -DDEBUG_ADULT

IndexerLot_langtest: src/IndexerLot_langtest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_langtest/main.c  src/parser/lex.yy.c src/parser/y.tab.c -o bin/IndexerLot_langtest $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS -DDEBUG_ADULT
	

IndexerLot_fik32bitbug: src/IndexerLot_fik32bitbug/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_fik32bitbug/main.c  src/parser/libhtml_parser.a -o bin/IndexerLot_fik32bitbug $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

IndexerLot_getno: src/IndexerLot_getno/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLot_getno/main.c  src/parser/lex.yy.c src/parser/lex.yy.c -o bin/IndexerLot_getno $(LDFLAGS) -D DI_FILE_CASHE -D NOWARNINGS

lotlistDispatcher: src/lotlistDispatcher/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/lotlistDispatcher/main.c -o bin/lotlistDispatcher $(LDFLAGS)

searchfilterTest: src/searchfilterTest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/searchfilterTest/main.c src/searchfilter/searchfilter.c -o bin/searchfilterTest $(LDFLAGS)

infoquery: src/infoquery/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/infoquery/main.c src/boithoadClientLib/liboithoaut.a -o bin/infoquery $(LDFLAGS)

GetIndexAsArrayTest: src/GetIndexAsArrayTest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/GetIndexAsArrayTest/main.c -o bin/GetIndexAsArrayTest $(LDFLAGS)

bbdocumentWebAdd: src/bbdocumentWebAdd/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentWebAdd/main.c src/base64/base64.c -o bin/bbdocumentWebAdd $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS

bbdocumentMakeLotUrlsdb: src/bbdocumentMakeLotUrlsdb/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentMakeLotUrlsdb/main.c -o bin/bbdocumentMakeLotUrlsdb $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS

bbdocumentConvertTest: src/bbdocumentConvertTest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/bbdocumentConvertTest/main.c -o bin/bbdocumentConvertTest $(LDFLAGS) $(LIBXML) $(BBDOCUMENT) -D BLACK_BOKS


analyseShortRank: src/analyseShortRank/main.c
	$(CC) $(CFLAGS) src/analyseShortRank/main.c -o bin/analyseShortRank $(LDFLAGS)

DIconvert: src/DIconvert/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/DIconvert/main.c -o bin/DIconvert $(LDFLAGS)

boithoad: src/boithoad/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoad/main.c src/3pLibs/keyValueHash/hashtable_itr.c src/3pLibs/keyValueHash/hashtable.c -o bin/boithoad $(LDFLAGS) $(LDAP) $(MYSQL) -D BLACK_BOKS -D WITH_CONFIG

boithoadtest: src/boithoadtest/main.c
	$(CC) $(CFLAGS) src/boithoadtest/main.c src/boithoadClientLib/liboithoaut.a -o bin/boithoadtest $(LDFLAGS)


lotlistPrint: src/lotlistPrint/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/lotlistPrint/main.c  -o bin/lotlistPrint $(LDFLAGS)

cleanCacheDir: src/cleanCacheDir/main.c
	$(CC) $(CFLAGS) src/cleanCacheDir/main.c -o bin/cleanCacheDir

lotcp: src/lotcp/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/lotcp/main.c  -o bin/lotcp $(LDFLAGS)

crawlFiles: src/crawlFiles/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/crawlFiles/main.c  -o bin/crawlFiles $(LDFLAGS) src/boitho-bbdn/bbdnclient.c -D BLACK_BOKS

crawlSMB: src/crawlSMB/main.c
#<<<<<<< Makefile
#	flex -f -8 -i src/crawlSMB/acl.parser.l
#	$(CC) $(CFLAGS) $(LIBS)*.c -lsmbclient src/crawlSMB/lex.yy.c src/boitho-bbdn/bbdnclient.c src/crawlSMB/crawlsmb.c src/crawlSMB/main.c -o bin/crawlSMB $(LDFLAGS) -D BLACK_BOKS
#=======
#	flex -f -8 -i -o src/crawlSMB/lex.acl.c src/crawlSMB/acl.parser.l
#	$(CC) $(CFLAGS) $(LIBS)*.c -lsmbclient src/crawlSMB/lex.acl.c src/crawlSMB/crawlsmb.c src/crawlSMB/main.c  -o bin/crawlSMB $(LDFLAGS) -D BLACK_BOKS
#>>>>>>> 1.15
	flex -f -8 -i -o src/crawlSMB/lex.acl.c src/crawlSMB/acl.parser.l
	$(CC) $(CFLAGS) $(LIBS)*.c -lsmbclient src/crawlSMB/lex.acl.c src/crawlSMB/crawlsmb.c src/crawlSMB/main.c src/boitho-bbdn/bbdnclient.c -o bin/crawlSMB $(LDFLAGS) -D BLACK_BOKS

	

testGetNextLotForIndex: src/testGetNextLotForIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/testGetNextLotForIndex/main.c  -o bin/testGetNextLotForIndex $(LDFLAGS)

everrun: src/everrun/main.c
	$(CC) src/everrun/main.c -o bin/everrun

searchcl : src/searchkernel/searchcl.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/query/lex.query.o src/searchkernel/cgi-util.c src/searchkernel/parseEnv.c src/searchkernel/searchkernel.c src/searchkernel/search.c src/searchkernel/searchcl.c src/parse_summary/libsummary.a -o bin/searchcl $(LDFLAGS)

#dropper -D WITH_MEMINDEX og -D WITH_RANK_FILTER for nå
SEARCHCOMMAND = $(CFLAGS) $(LIBS)*.c src/query/lex.query.c src/3pLibs/keyValueHash/hashtable.c src/3pLibs/keyValueHash/hashtable_itr.c src/searchkernel/searchkernel.c src/searchFilters/searchFilters.c src/searchkernel/search.c src/searchkernel/searchd.c src/parse_summary/libsummary.a src/parse_summary/libhighlight.a -o bin/searchd $(LDFLAGS) -lpthread -D WITH_THREAD

searchd : src/searchkernel/searchd.c
	$(CC) $(SEARCHCOMMAND) -D WITH_RANK_FILTER

searchdbb : src/searchkernel/searchd.c
	$(CC) $(SEARCHCOMMAND) -D BLACK_BOKS

boithoads: src/boithoads/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS)

boithoadshread: src/boithoads/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithoads/main.c src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/httpGet/httpGet.c src/parse_summary/libsummary.a -o bin/boithoads $(LDFLAGS) $(MYSQL_THREAD) $(LIBXML) $(CURLLIBS) -D WITH_THREAD -lpthread

addout.cgi: src/addout.cgi/main.c
	$(CC) $(CFLAGS) src/addout.cgi/main.c src/cgi-util/cgi-util.c -o bin/addout.cgi $(LDFLAGS) $(MYSQL) 

ppcXmlParserTest: src/ppcXmlParserTest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ppcXmlParserTest/main.c src/parse_summary/libsummary.a src/ppcXmlParser/cleanString.c src/ppcXmlParser/ppcXmlProviders.c src/ppcXmlParser/ppcXmlParserAmazon.c src/ppcXmlParser/ppcXmlParser.c src/searchFilters/searchFilters.c src/parse_summary/libsummary.a src/httpGet/httpGet.c -o bin/ppcXmlParserTest $(LDFLAGS) $(MYSQL) $(LIBXML) $(CURLLIBS)

dispatcherCOMAND = $(CFLAGS) $(LIBS)*.c src/dispatcher_all/main.c src/tkey/tkey.c src/cgi-util/cgi-util.c src/searchFilters/searchFilters.c -o bin/dispatcher_all $(LDFLAGS) $(MYSQL)

dispatcher_all: src/dispatcher_all/main.c
		$(CC) $(dispatcherCOMAND) $(LIBGeoIP) -D WITH_CASHE

dispatcher_allbb: src/dispatcher_all/main.c
		$(CC) $(dispatcherCOMAND) -D BLACK_BOKS


dispatcher: src/dispatcher/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/dispatcher/main.c src/cgi-util/cgi-util.c -o bin/dispatcher $(LDFLAGS)

putFilesIntoFileTree: src/putFilesIntoFileTree/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/putFilesIntoFileTree/main.c -o bin/putFilesIntoFileTree $(LDFLAGS)

nyeurlerIntegeryCheck: src/nyeurlerIntegeryCheck/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/nyeurlerIntegeryCheck/main.c -o bin/nyeurlerIntegeryCheck $(LDFLAGS)

rread : src/rread/rread.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/rread/rread.c -o bin/rread $(LDFLAGS)

convertReposetoryCOMAND = $(CFLAGS) $(LIBS)*.c src/convertReposetory/main.c -o bin/convertReposetory $(LDFLAGS)
convertReposetory: src/convertReposetory/main.c
	$(CC) $(convertReposetoryCOMAND)

convertReposetorybb: src/convertReposetory/main.c
	$(CC) $(convertReposetoryCOMAND) -D BLACK_BOKS


makeSumaryCashe: src/makeSumaryCashe/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/makeSumaryCashe/main.c src/parse_summary/libsummary.a -o bin/makeSumaryCashe $(LDFLAGS) -D NOWARNINGS

rread_getsomeurls: src/rread_getsomeurls/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/rread_getsomeurls/main.c -o bin/rread_getsomeurls $(LDFLAGS)

IndexerLotLite: src/IndexerLotLite/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotLite/main.c -o bin/IndexerLotLite $(LDFLAGS)

readMainIndex: src/readMainIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readMainIndex/main.c -o bin/readMainIndex $(LDFLAGS)

readMainIndexSearchSha1: src/readMainIndexSearchSha1/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readMainIndexSearchSha1/main.c -o bin/readMainIndexSearchSha1 $(LDFLAGS)

LotInvertetIndexMaker: src/LotInvertetIndexMaker/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker/main.c -o bin/LotInvertetIndexMaker $(LDFLAGS)

listLostLots: src/listLostLots/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/listLostLots/main.c -o bin/listLostLots $(LDFLAGS)

recoverUrlForLot: src/recoverUrlForLot/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/recoverUrlForLot/main.c -o bin/recoverUrlForLot $(LDFLAGS)

removeUnnecessaryRevindex: src/removeUnnecessaryRevindex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/removeUnnecessaryRevindex/main.c -o bin/removeUnnecessaryRevindex $(LDFLAGS)

LotInvertetIndexMaker2:	src/LotInvertetIndexMaker2/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/LotInvertetIndexMaker2/main.c -o bin/LotInvertetIndexMaker2 $(LDFLAGS)

ThumbnaleDemon: src/ThumbnaleDemon/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ThumbnaleDemon/main.c -o bin/ThumbnaleDemon $(LDFLAGS)

PoprankMerge: src/PoprankMerge/poptorank.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/PoprankMerge/poptorank.c -o bin/PoprankMerge $(LDFLAGS)

ipbann: src/ipbann/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ipbann/main.c  -o bin/ipbann $(LDFLAGS)

addanchors: src/addanchors/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addanchors/main.c -o bin/addanchors $(LDFLAGS)

readNyeUrlerFil: src/readNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readNyeUrlerFil/main.c  -o bin/readNyeUrlerFil $(LDFLAGS)

filterNyeUrlerFil: src/filterNyeUrlerFil/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/filterNyeUrlerFil/main.c  -o bin/filterNyeUrlerFil $(LDFLAGS)

addUrlsToIndex: src/addUrlsToIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addUrlsToIndex/main.c -o bin/addUrlsToIndex $(LDFLAGS)

readLinkFile: src/readLinkFile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readLinkFile/main.c -o bin/readLinkFile $(LDFLAGS)

anchorread: src/anchorread/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/anchorread/main.c -o bin/anchorread $(LDFLAGS)

BrankCalculate:	src/BrankCalculate/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankCalculate/*.c -o bin/BrankCalculate $(LDFLAGS)

BrankMerge: src/BrankMerge/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/BrankMerge/*.c -o bin/BrankMerge $(LDFLAGS)

readLinkDB: src/readLinkDB/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/readLinkDB/main.c -o bin/readLinkDB $(LDFLAGS)

SortUdfile: src/SortUdfile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SortUdfile/main.c -o bin/SortUdfile $(LDFLAGS)

PageInfo: src/PageInfo/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/PageInfo/main.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/PageInfo $(LDFLAGS)

PageInfobb: src/PageInfo/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/PageInfo/main.c src/parser/lex.yy.c src/parser/y.tab.c -o bin/PageInfo $(LDFLAGS) -D BLACK_BOKS

addManuellUrlFile: src/addManuellUrlFile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/addManuellUrlFile/main.c  -o bin/addManuellUrlFile $(LDFLAGS)

dumpLotAsFiles: src/dumpLotAsFiles/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/dumpLotAsFiles/main.c -o bin/dumpLotAsFiles $(LDFLAGS)

boithold: src/boithold/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/boithold/getpath.c src/boithold/main.c -o bin/boithold $(LDFLAGS)

boitho-bbdn: src/boitho-bbdn/bbdnserver.c
	$(CC) $(CFLAGS) $(LIBS)*.c  src/boitho-bbdn/bbdnserver.c -o bin/boitho-bbdn $(LDFLAGS) $(BBDOCUMENT) -D BLACK_BOKS


boitholdTest: src/boitholdTest/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/boitholdTest/main.c -o bin/boitholdTest $(LDFLAGS)

SplittUdfileByLot: src/SplittUdfileByLot/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/SplittUdfileByLot/main.c -o bin/SplittUdfileByLot $(LDFLAGS)

UrlToDocIDIndexer: src/UrlToDocIDIndexer/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/UrlToDocIDIndexer/main.c -o bin/UrlToDocIDIndexer $(LDFLAGS)

UrlToDocIDQuery: src/UrlToDocIDQuery/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/UrlToDocIDQuery/main.c -o bin/UrlToDocIDQuery $(LDFLAGS)

UrlToDocIDSplitUdfile: src/UrlToDocIDSplitUdfile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/UrlToDocIDSplitUdfile/main.c -o bin/UrlToDocIDSplitUdfile $(LDFLAGS)

addReposerotyToIndex: src/addReposerotyToIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/addReposerotyToIndex/main.c -o bin/addReposerotyToIndex $(LDFLAGS)


readDocumentIndex: src/readDocumentIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/readDocumentIndex/main.c -o bin/readDocumentIndex $(LDFLAGS)

adultBuildIndex: src/adultBuildIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/adultBuildIndex/main.c -o bin/adultBuildIndex $(LDFLAGS)

ipdbBuildLotIndex: src/ipdbBuildLotIndex/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ipdbBuildLotIndex/main.c -o bin/ipdbBuildLotIndex $(LDFLAGS)

ipdbMakeMain: src/ipdbMakeMain/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ipdbMakeMain/main.c -o bin/ipdbMakeMain $(LDFLAGS)

IndexerLotUrl: src/IndexerLotUrl/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/IndexerLotUrl/main.c -o bin/IndexerLotUrl $(LDFLAGS)

fixsAsciixBug: src/fixsAsciixBug/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/fixsAsciixBug/main.c -o bin/fixsAsciixBug $(LDFLAGS)

getUncrawledPages: src/getUncrawledPages/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/getUncrawledPages/main.c -o bin/getUncrawledPages $(LDFLAGS)

cpLotFile: src/cpLotFile/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/cpLotFile/main.c -o bin/cpLotFile $(LDFLAGS)


SHOWTHUMBCMANDS = $(CFLAGS) $(LIBS)*.c src/ShowThumb/main.c src/cgi-util/cgi-util.c -o bin/ShowThumb $(LDFLAGS)

ShowThumb: src/ShowThumb/main.c
	$(CC) $(SHOWTHUMBCMANDS)

ShowThumbbb: src/ShowThumb/main.c
	$(CC) $(SHOWTHUMBCMANDS) -D BLACK_BOKS

ShowCache: src/ShowCache/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c src/ShowCache/main.c src/cgi-util/cgi-util.c -o bin/ShowCache $(LDFLAGS)


boithoshmd: src/boithoshmd/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/boithoshmd/main.c -o bin/boithoshmd $(LDFLAGS)

builIpDB: src/builIpDB/main.c
	$(CC) $(CFLAGS) $(LIBS)*.c -ldb src/builIpDB/main.c -o bin/builIpDB $(LDFLAGS)

ConvertRankToShortRankFile: src/ConvertRankToShortRankFile/main.c
	$(CC) src/ConvertRankToShortRankFile/main.c -lm -o bin/ConvertRankToShortRankFile

readIIndex: src/readIIndex/main.c
	$(CC) src/readIIndex/main.c -o bin/readIIndex

mergeIIndex: src/mergeIIndex/main.c
	$(CC) src/mergeIIndex/main.c src/common/bfileutil.c src/common/lot.c src/common/bstr.c -o bin/mergeIIndex -lm -D_FILE_OFFSET_BITS=64


boithoadClientLib: src/boithoadClientLib/boithoadClientLib.c
	$(CC) -c $(CFLAGS) src/common/daemon.c -o src/boithoadClientLib/daemon.o  -g
	$(CC) -c $(CFLAGS) src/boithoadClientLib/boithoadClientLib.c -o src/boithoadClientLib/boithoadClientLib.o  -g
	ar rc src/boithoadClientLib/liboithoaut.a src/boithoadClientLib/boithoadClientLib.o src/boithoadClientLib/daemon.o
	ranlib src/boithoadClientLib/liboithoaut.a

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

