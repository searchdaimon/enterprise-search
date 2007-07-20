
#ifndef _DEFINE__H_
#define _DEFINE__H_

#include <time.h>

//#include "DocumentIndex.h"
//#include "lot.h"

#define maxTotalIindexHits MaxTermHit * maxIndexElements

#define NrOfDataDirectorys 64

#define NEWURLFILES_NR 5

#define MAXFILTYPES 100

#define BMAX_RANKARRAY 20

#ifdef BLACK_BOKS
	#define NrofDocIDsInLot 5000
#else
	#define NrofDocIDsInLot 500000
#endif
//maks sider vi leser fra indeksen. Er denne på 20k og vi har 30k treff får vi bare returnert 20k
#define maxIndexElements 400000

//totalt maks lotter vi kan ha.
#define maxLots 10000

#define MAXFILTYPES 100

#define MAX_COLLECTIONS 20

//#define BOITHO_MYSQL_DB "boithobbdemo"
#define BOITHO_MYSQL_DB "boithobb"

#define AdultWeightForXXX 50


#define BoithosAmazonAssociateTag "boitho-20"
#define BoithosAmazonSubscriptionId "0WG4PH85SE9114TS7JR2"

#define MAX_USER_NAME_LEN 64

#define maxWordlLen 30

#define rNetTrabsferBlok 65536


//#define maxSubnameLength 64
#define maxSubnameLength 512 //512 fra 6 nov

//#define BSDPORT 6501 //search system
#define BLDPORT 3490 //lot deamon

//flyttet til boithoad.h
//#define BADPORT 3491 //Boitho autentifisering
//#define MAX_LDAP_ATTR_LEN 512
//
//#define bad_askToAuthenticate		1
//#define bad_listUsers 		10
//#define bad_listGroups		11
//#define bad_groupsForUser		20
//#define bad_getPassword		30
//#define ad_userauthenticated_OK  111
//#define ad_userauthenticated_ERROR  000

#define searchd_responstype_error		10
#define searchd_responstype_normalsearch 	20
#define searchd_responstype_ranking		30


//#define CMDPORT 3492 //boitho crawler manager

#define nrOfHttpResponsCodes 700

//maxs hits i et dokument som skal lagres, ikke som skal vises i søk
#define MaxsHitsInIndex 20


#define MaxQueryLen 100


#define BLDPROTOCOLVERSION 1

#define nrOfUrlToDocIDFiles 64

#define mineAthorTermSizeForMemory 100
#define mineMainTermSizeForMemory 100

//#define URLTODOCIDINDEX "/home/boitho/boithoTools/indexses/UrlToDocID/"
#define URLTODOCIDINDEX "/mnt/hda4/UrlToDocID/"

#define POPFILE "/home/boitho/config/popindex"
#define SHORTPOPFILE "/home/boitho/config/shortRank"
#define IPDBPATH "/home/boitho/config/ipdb"

//maks hit vi kan ha i en side
//skal erstatets av MaxsHitsInIndex
#define MaxTermHit 20 


#define AntallBarrals 64

//komandoer
#define C_rGetNext 	1
#define C_rLotData 	2
#define C_DIWrite	3
#define C_DIRead	4
#define C_DIGetIp	5
#define C_rGetIndexTime	6
#define C_rSetIndexTime 7
#define C_rSendFile	8
#define C_rGetFile	9
#define C_rmkdir	10
#define C_rComand	11
#define C_rGetSize	12
#define C_rEOF 		101

#define net_CanDo		13
#define net_nomatch		14
#define net_match		15



#define siderType_normal 	1
#define siderType_ppcside 	2
#define siderType_ppctop 	3

#define cm_crawlcollection	1
#define cm_crawlcanconect	2
#define cm_scan			3
#define cm_pathaccess		4
#define cm_recrawlcollection	10
#define cm_deleteCollection	50
#define cm_rewriteurl		60

//verdier av forskjelige term posisjoner
#define poengForBody 1	
#define poengForHeadline 4
#define poengForTittel 15
#define poengForUrlSub 2
#define poengForUrlMain 30 //temp. til vi har url filtrering i dispatsjer


#define maxPoengBody 5
#define MaxPoengHeadline 12
#define MaxPoengTittel 17
#define maxPoengAthor 260

#define complicacy_maxPoengAthorPhraserank 260
#define complicacy_maxPoengAthorSimple 6

//Lengden på en okument post lengde
//#define DocumentIndexPOSTLENGTH 253

struct subnamesFiltypesFormat {
	char name[5]; //4 for navn og \0
	int nrof;
};

struct filteronFormat {

	char *filetype;
	char *collection;
	char *date;
};

struct subnamesConfigFormat {
        const char *summary;
     char filterSameUrl; //bool
     char filterSameDomain;
     char filterTLDs;
     char filterResponse;
     char filterSameCrc32;

        char rankAthorArray[BMAX_RANKARRAY];
     char rankAthorArrayLen;

        char rankTittelArray[BMAX_RANKARRAY];
     char rankTittelArrayLen;
        char rankTittelFirstWord;

        char rankHeadlineArray[BMAX_RANKARRAY];
     char rankHeadlineArrayLen;

        char rankBodyArray[BMAX_RANKARRAY];
     char rankBodyArrayLen;

        char rankUrlArray[BMAX_RANKARRAY];
     char rankUrlArrayLen;
        char rankUrlMainWord;

     const char *defaultthumbnail;

     char isPaidInclusion; //bool
};
struct subnamesFormat {
	char subname[64];
	int hits;
	struct subnamesConfigFormat config;
	//28 mai 2007: tar bort, ser ikke ut til å være brukt lengde, etter at vi implementerete filtyper som filter
	//int nrOfFiletypes;
	//struct subnamesFiltypesFormat filtypes[MAXFILTYPES];
};

struct brankPageElementsFormat {
	unsigned long int IPAddress;
	unsigned char nrOfOutLinks;
	unsigned short response;
};

//formatet på dokumenet indeks posten
struct DocumentIndexFormat {
	char Url[201];
	char Sprok[4];
	unsigned short Offensive_code;
	char Dokumenttype[4];
	unsigned int CrawleDato;
	unsigned short AntallFeiledeCrawl;
	unsigned short AdultWeight;
	unsigned int RepositoryPointer;
	unsigned short htmlSize;
        unsigned short imageSize;
	unsigned int ResourcePointer;
	unsigned short ResourceSize;
        unsigned long int IPAddress;
        unsigned short response;
        unsigned short userID;
        double clientVersion;

        //new: 28.mars 2006
        unsigned char nrOfOutLinks;
        unsigned int SummaryPointer;
        unsigned short SummarySize;
        unsigned int crc32;
	#ifdef BLACK_BOKS
		char reservedSpace[64]; //3 now
	#endif

};

#define MaxReposetoryContent 30000

struct ReposetoryFormat {
        int DocID;
        char url[201];
        char content_type[7];
        char content[MaxReposetoryContent];
	unsigned long int IPAddress;
	unsigned short response;
};

struct ReposetoryHeaderFormat {
	unsigned int DocID;
        char url[200];
        char content_type[4];
        unsigned long int IPAddress;
        unsigned short response;
	unsigned short htmlSize;
	unsigned short imageSize;
	unsigned long int time;
	unsigned short userID;
	double clientVersion;
	#ifdef BLACK_BOKS
		//int aclSize;
		int acl_allowSize;
		#ifdef IIACL
		int acl_deniedSize;
		#endif
		time_t storageTime; //3 now
		char doctype[4]; //3 now
		char reservedSpace[64]; //3 now
	#endif
};


struct StorageDirectorysFormat {
	char Name[64];
};
//

//formater på filene som ineholder ordbokene
struct DictionaryFormat {
	unsigned long WordID;
	unsigned long Adress;
	unsigned long SizeForTerm;
};

struct indexFilteredFormat {

	char filename; //bool
	char date;
	char subname;
};

struct rank_explaindFormat {
	unsigned short rankBody;
	unsigned short rankHeadline;
	unsigned short rankTittel;
	unsigned short rankAthor;
	unsigned short rankUrl_mainbody;
	unsigned short rankUrl;

	unsigned short nrAthorPhrase;
	unsigned short nrAthor;	
};

struct hitsFormat {
	unsigned short pos;
	char phrase;
};

// Formatett på treff i indeksen
struct iindexMainElements {
	int DocID;
	int TermAntall;
	//unsigned short hits[MaxTermHit];
	//unsigned short *hits;
	struct hitsFormat *hits;
	int TermRank;
	int PopRank;
	unsigned long int allrank; //byttet til unsigned long 24. okt 2006. unsigned long er 4 bit int
	char phraseMatch;
	unsigned char langnr;
	struct subnamesFormat *subname;
	//unsigned long int date; //16 nov 2006
	//19.04.07
	time_t date; //16 nov 2006

	#ifdef BLACK_BOKS
		char filetype[5];
		char deleted;
		struct indexFilteredFormat indexFiltered;
	#endif

	#ifdef EXPLAIN_RANK
		struct rank_explaindFormat rank_explaind;
	#endif
};


struct iindexFormat {
	//unsigned short hits[maxTotalIindexHits];
	struct hitsFormat hits[maxTotalIindexHits];
	int nrofHits;
	struct iindexMainElements iindex[maxIndexElements];
};

// formatet for dataene for sidene skal være
struct SiderFormat {
	struct DocumentIndexFormat DocumentIndex;
	struct iindexMainElements iindex;
	//struct ReposetoryFormat Reposetory;	
	//char htmlBuffer[30000];
	//char *htmlBuffer;
	int htmlSize;
	//char description[250]; 
	char description[300]; 
	char title [70];
	char thumbnale[128];
	int thumbnailwidth;
	int thumbnailheight;
	char cacheLink[128];
	char domain[65];
	char servername[32];
	//29 mai 2007. Gjør om til int, ser ikke ut til å bruke flyttetall her
	//float posisjon;
	int posisjon;
	int deletet;
	unsigned int crc32;
	int type;

	#ifdef BLACK_BOKS
		char uri[1024];
		char url[1024];
	#else
		char uri[201];
		char url[201];
	#endif
	char user[21];
	float bid;
	struct subnamesFormat subname;
	char HtmlPreparsed;
	double pageGenerateTime;

	unsigned char pathlen;

	unsigned short DomainID;
};

struct queryTimeFormat {
	double total;
	double indexSort;
	double dictionarySearch;
	double AthorSearch;
	double AthorRank;
	double MainSearch;
	double MainRank;
	double MainAthorMerge;
	double allrankCalc;
	double popResult;
	double adultcalk;
	double dateview;
	double iintegerGetValueDate;
	double filetypes;
	double crawlManager;
	double getUserObjekt;
	double cmc_conect;
	double UrlSearch;
	double popRank;
	double responseShortning;
	double searchSimple;
};

struct filtersTrapedFormat {
	int filterAdultWeight_bool;
	int filterAdultWeight_value;
	int filterSameCrc32_1;
	int filterSameUrl;
	int find_domain_no_subname;
	int filterSameDomain;
	int filterTLDs;
	int filterResponse;
	int cantpopResult;
	int cmc_pathaccess;
	int filterSameCrc32_2;
	int cantDIRead;
	int getingDomainID;
	int sameDomainID;
	int filterNoUrl;
};

struct filtypesFormat {
	char name[5]; //4 for navn og \0
	int nrof;
};

#define MAXFILTERELEMENTS 30

struct filterinfoElementsFormat {
	char name[20];
	char longname[20];
	char query[MaxQueryLen];
	int nrof;
	char checked; //bool
};

struct filterinfoFormat {
	struct filterinfoElementsFormat elements[MAXFILTERELEMENTS];
	int nrof;
};

struct filtersFormat {
	 #ifdef BLACK_BOKS
	struct filterinfoFormat filtypes;
	struct filterinfoFormat collections;	
	#endif
};

struct SiderHederFormat {
	int filtered;
	int showabal;
	int TotaltTreff;
	double total_usecs;
	char hiliteQuery[MaxQueryLen];
	struct queryTimeFormat queryTime;
	char servername[32];
	struct subnamesFormat subnames[MAX_COLLECTIONS];
	int nrOfSubnames;
	//int *dates[11]; //= {0,0,0,0,0,0,0,0,0,0,0};
	struct filtersTrapedFormat filtersTraped;
	int dates[11];
	int filtypesnrof;
	//struct filtypesFormat filtypes[MAXFILTYPES];
        char errorstr[250];
        int errorstrlen;
	int responstype;
	struct filtersFormat filters;
};

//formatet på anchor filer
struct anchorfileFormat {
	unsigned int DocID;
	char text[50];
};

struct linkdb_block
{
	unsigned int        DocID_from;
	unsigned int        DocID_to;
};


struct udfileFormat {
	unsigned char url[200];
	unsigned int DocID;
};

struct updateFormat
{
    unsigned char       sha1[20];
    unsigned char       url[200];
    unsigned char       linktext[50];
    unsigned int        DocID_from;
};

struct mainIndexFormat
{
    unsigned char       sha1[20];
    unsigned int        DocID;
};

struct wordsFormat {
        //#ifdef DEBUG_ADULT
        #ifdef PRESERVE_WORDS
                char word[maxWordlLen];
        #endif
        unsigned int WordID;
        unsigned int position;
        unsigned short occurrence;
	int unsortetIndexPosition;
};



struct queryNodeHederFormat
{
	//toDo, skal ikke dette være MaxQueryLen ?
	char query[MaxQueryLen];
	char subname[maxSubnameLength];
	int MaxsHits;
	int start;
	int filterOn;
	unsigned int getRank;
	char userip[16];
	char GeoIPcontry[3];
	char search_user[21];
	char HTTP_ACCEPT_LANGUAGE[12];
        char HTTP_USER_AGENT[64];
        char HTTP_REFERER[255];
	char AmazonAssociateTag[50];
        char AmazonSubscriptionId[50];
//v3	char languageFilter[12];
	char orderby[10];
};

//for "struct query"
#include "../query/query_parser.h"

// formater for de forskjelige delene i et query
struct QueryDataForamt {
        char query[MaxQueryLen];
        //struct query queryParsed;
	query_array queryParsed;
	//char queryEscaped[MaxQueryLen*2+1];
	char queryhtml[MaxQueryLen*2+1];
	char userip[16];
	char subname[maxSubnameLength];
        //char hiliteQuery[MaxQueryLen];
	int MaxsHits;
	int start;
	int filterOn;
	char GeoIPcontry[3];
	char search_user[21];
	char HTTP_ACCEPT_LANGUAGE[12];
        char HTTP_USER_AGENT[64];
        char HTTP_REFERER[255];
	char AmazonAssociateTag[50];
	char AmazonSubscriptionId[50];
//v3	char languageFilter[12];
	char orderby[10];
	char tkey[33]; // 32 bytes key +1 for \0
	char rankUrl[256];

	query_array search_user_as_query;
};


enum platform_type {
	UNKNOWN_PLATFORM = 0,
	UNIX,
	WINDOWS,
	MAC,
};

enum browser_type {
	UNKNOWN_BROWSER = 0,
	MOZILLA,
	OPERA,
	IE,
};

struct rewriteFormat {
	enum platform_type ptype;
	enum browser_type btype;
	char collection[64];
	char uri[512];
};



#endif //_DEFINE__H_

