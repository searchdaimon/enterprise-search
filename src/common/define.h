#ifndef _DEFINE__H_
#define _DEFINE__H_

#include <time.h>
#include <sys/types.h>
#include "../ds/dcontainer.h"
#include "../query/query_parser.h" //for "struct query_array"


#define maxTotalIindexHits MaxTermHit * maxIndexElements
#define NrOfDataDirectorys 64
#define NEWURLFILES_NR 5
#define MAXFILTYPES 100
#define MAX_SUPORTET_LANG 50
#define UrlToDocIDnrOfFiles 999
#define BMAX_RANKARRAY 20

#ifdef BLACK_BOX
	#define NrofDocIDsInLot 5000
#else
	#define NrofDocIDsInLot 500000
#endif

//maks sider vi leser fra indeksen. Er denne på 20k og vi har 30k treff får vi bare returnert 20k
#define maxIndexElements 1500000

//totalt maks lotter vi kan ha.
#define maxLots 10000

#define MAXFILTYPES 100
#define MAX_COLLECTIONS 20
#define BOITHO_MYSQL_DB "boithobb"
#define AdultWeightForXXX 50


#define BoithosAmazonAssociateTag "boitho-20"
#define BoithosAmazonSubscriptionId "0WG4PH85SE9114TS7JR2"

#define MAX_USER_NAME_LEN 64
#define MAX_ATTRIB_LEN	128
#define maxWordlLen 30
#define rNetTrabsferBlok 65536



#define maxSubnameLength 512 //512 fra 6 nov
#define BLDPORT 3490 //lot deamon

//flyttet til boithoad.h
#define MAX_LDAP_ATTR_LEN 512

#define searchd_responstype_error		10
#define searchd_responstype_normalsearch 	20
#define searchd_responstype_ranking		30


#define nrOfHttpResponsCodes 700

//maxs hits i et dokument som skal lagres, ikke som skal vises i søk
#define MaxsHitsInIndex 20


#ifdef BLACK_BOX
    #define MaxQueryLen 256
#else
    #define MaxQueryLen 100
#endif


#define BLDPROTOCOLVERSION 1

#define nrOfUrlToDocIDFiles 64

#define mineAnchorTermSizeForMemory 100
#define mineMainTermSizeForMemory 100

#define MAX_ATTRIBUTES_IN_QUERY	10

#define URLTODOCIDINDEX "/home/boitho/boithoTools/indexses/UrlToDocID/"

#define POPFILE "/home/boitho/config/popindex"
#define SHORTPOPFILE "/home/boitho/config/shortRank"
#define IPDBPATH "/home/boitho/config/ipdb"

//maks hit vi kan ha i en side
//skal erstatets av MaxsHitsInIndex
#define MaxTermHit 20 


#define AntallBarrals 64

//komandoer
#define C_rGetNext 			1
#define C_rLotData 			2
#define C_DIWrite			3
#define C_DIRead			4
#define C_DIGetIp			5
#define C_rGetIndexTime			6
#define C_rSetIndexTime 		7
#define C_rSendFile			8
#define C_rGetFile			9
#define C_rmkdir			10
#define C_rComand			11
#define C_rGetSize			12
#define C_anchorAdd			13
#define C_anchorGet			14
#define C_readHTML			15
#define C_urltodocid			16
#define C_getLotToIndex			17
#define C_getlotHasSufficientSpace 	18
#define C_rEOF 				101

#define C_DOCID_DONE			0
#define C_DOCID_NEXT			1
#define C_DOCID_NODB			2
#define C_DOCID_NOTFOUND 		3
#define C_DOCID_FOUND			4
#define C_DOCID_READY			5

#define net_CanDo			13
#define net_nomatch			14
#define net_match			15



#define siderType_normal 		1
#define siderType_ppcside 		2
#define siderType_ppctop 		3

#define cm_crawlcollection		1
#define cm_crawlcanconect		2
#define cm_scan				3
#define cm_pathaccess			4
#define cm_groupsforuserfromusersystem 	5
#define cm_collectionsforuser		6
#define cm_usersystemfromcollection 	7
#define cm_listusersus			8
#define cm_recrawlcollection		10
#define cm_collectionislocked   	11
#define cm_authenticateuser		12
#define cm_deleteCollection		50
#define cm_rewriteurl			60
#define cm_killcrawl            	70
#define cm_removeForeignUsers		80
#define cm_addForeignUser		81

struct cm_listusers_h {
	int num_users;
	char error[512];
};

//verdier av forskjelige term posisjoner
#define poengForBody 			1	
#define poengForHeadline 		4
#define poengForTittel 			15


#define maxPoengBody 			5
#define MaxPoengHeadline 		12
#define MaxPoengTittel 			17
#define maxPoengAnchor 			260

#define complicacy_maxPoengAnchorPhraserank 260
#define complicacy_maxPoengAnchorSimple 6

#define SUMMARY_NONE 			0
#define SUMMARY_START   		1
#define SUMMARY_SNIPPET 		2
#define SUMMARY_DB 			3

#define NAVMENUCFG_SIZE 		4096
#define MAX_RESULT_OFFSET 		1000

#define LANG_UNKNOWN 			-1
#define LANG_NBO 			1
#define LANG_ENG 			2

#define MAX_SERVERNAME_LEN 		32

#define ANCHORMAGIC 			0xb309a213

#define MAXFILTERELEMENTS 		30

#define __bunused __attribute__((unused))

#define ANONYMOUS_USER "SDESAnonymous"

/* MYSQL login information */
#define MYSQL_HOST "localhost"
#define MYSQL_USER "boitho"
#define MYSQL_PASS "G7J7v5L5Y7"

/* Output formats */
#define _OUT_FOMRAT_SD 			1
#define _OUT_FOMRAT_OPENSEARCH 		2
#define _OUT_FOMRAT_SD_JSON 		3


enum snippet_format { xml_format, json_format };

typedef unsigned int docid;


typedef enum {
	CAL_ACL,
	CAL_USER,
	CAL_GROUP,
	CAL_ANONYMOUS,
} collection_accesslevel_t;


struct subnamesFiltypesFormat {
	char name[5]; //4 for navn og \0
	int nrof;
};


struct filteronFormat {
    container	*attributes;
    char	*collection;
    char	*date;
    char	*sort;
};


struct subnamesConfigFormat {
	char summary; // 23.07.2008 - summary til char (fra const char *), dj
	char filterSameUrl;
	char filterSameDomain;
	char filterTLDs;
	char filterResponse;
	char filterSameCrc32;

	char rankAnchorArray[BMAX_RANKARRAY];
	char rankAnchorArrayLen;

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

	//sql query som vil bli utført hver gang vi ser en side i denne subnamet
	char sqlImpressionsLogQuery[252];

	char cache_link; // 30.09.2008 - show cache link, dj
	collection_accesslevel_t accesslevel; // Different access levels, user, group, acl and anonymous
	char group[128]; /* the group that has access to the collection */
	char has_config; // Set to true if the config has been retrieved for this collection
};


struct subnamesFormat {
	char subname[maxSubnameLength]; // 23.07.08 - fra 64 til maxSubnameLength
	int hits;
	struct subnamesConfigFormat config;
};


struct brank {
	unsigned char rank;
};


struct brankPageElementsFormat {
	unsigned int IPAddress;
	unsigned char nrOfOutLinks;
	unsigned short response;
	unsigned int DomainDI;
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
	unsigned short htmlSize __attribute__ ((deprecated));
        unsigned short imageSize;
	unsigned int ResourcePointer;
	unsigned short ResourceSize;
        unsigned int IPAddress;
        unsigned short response;
        unsigned short userID;
        double clientVersion;
        unsigned char nrOfOutLinks;
        unsigned int SummaryPointer;
        unsigned short SummarySize;
        unsigned int crc32;

	#ifdef BLACK_BOX
		time_t lastSeen;
		unsigned int htmlSize2;
		char reservedSpace[56]; //18 okt 2012
	#else 
		unsigned int htmlSize2;
	#endif
};


struct ReposetoryHeaderFormat {
	unsigned int DocID;
	#ifdef BLACK_BOX
		//Runerb: denne kan settes til deprecated slik når vi skal fase den ut:
		//char url[200] __attribute__((deprecated));
		char url[200];
	#else
		char url[200];
	#endif
	char content_type[4];
	unsigned int IPAddress;
	unsigned short response;
	unsigned short htmlSize __attribute__ ((deprecated));
	unsigned short imageSize;
	unsigned int time;
	unsigned short userID;
	double clientVersion;

	#ifdef BLACK_BOX
		int acl_allowSize;
		#ifdef IIACL
			int acl_deniedSize;
		#endif
		time_t storageTime;
		char doctype[4];
		unsigned short urllen;
		unsigned int attributeslen;
		unsigned int htmlSize2;
		unsigned char PopRank;
		char reservedSpace[49];
	#else
		unsigned int htmlSize2;
	#endif
};


struct StorageDirectorysFormat {
	char Name[64];
	int devid;
};


//formater på filene som ineholder ordbokene
struct DictionaryFormat {
	unsigned int WordID;
	unsigned int Adress;
	unsigned int SizeForTerm;
};


struct indexFilteredFormat {
	char is_filtered; // alle for en
	char filename; //bool
	char date;
	char subname;
	char duplicate;
	char duplicate_in_collection;
	char duplicate_to_show; // Den duplikaten som ikke filtreres bort.
	char attribute;
	char attrib[MAX_ATTRIBUTES_IN_QUERY]; // en for hver attributt
};


struct duplicate_docids {
        container *V;
	struct iindexMainElements *fistiindex;
};


struct rank_explaindFormat {
	unsigned short rankBody;
	unsigned short rankHeadline;
	unsigned short rankTittel;
	unsigned short rankAnchor;
	unsigned short rankUrl_mainbody;
	unsigned short rankUrlDomain;
	unsigned short rankUrlSub;
	unsigned short nrAnchorPhrase;
	unsigned short nrAnchor;	
	unsigned short nrBody;
	unsigned short nrHeadline;
	unsigned short nrTittel;
	unsigned short nrUrl_mainbody;
	unsigned short nrUrlDomain;
	unsigned short nrUrlSub;
	unsigned short maxBody;
	unsigned short maxHeadline;
	unsigned short maxTittel;
	unsigned short maxUrl_mainbody;
	unsigned short maxUrlDomain;
	unsigned short maxUrlSub;
	unsigned short maxAnchor;
};


struct hitsFormat {
	unsigned short pos;
	char phrase;
};


// Formatett på treff i indeksen
struct iindexMainElements {
	int DocID;
	int TermAntall;
	struct hitsFormat *hits;
	int TermRank;
	unsigned char PopRank;
	unsigned int allrank; //byttet til unsigned long 24. okt 2006. unsigned long er 4 bit int. 05 augus 2007: dropper "long", er 8 bytes på x86_64
	char phraseMatch;
	unsigned char langnr;
	struct subnamesFormat *subname;
	time_t date;

	#ifdef BLACK_BOX
		char filetype[5];
		char deleted;
		struct indexFilteredFormat indexFiltered;
	#endif

	#ifdef EXPLAIN_RANK
		struct rank_explaindFormat rank_explaind;
	#endif

	unsigned int originalPosition;

	// Trenger unik nøkkel for duplikater, bruker crc32 checksum.
	unsigned int crc32;
};


struct iindexFormat {
	struct hitsFormat hits[maxTotalIindexHits];
	int nrofHits;
	int attrib_count;
	struct iindexMainElements iindex[maxIndexElements];
	char **subnames_old;
	int phrasenr;
};


struct cache_params {
	docid doc_id;
	time_t time;
	char subname[maxSubnameLength];
	char cache_host[MAX_SERVERNAME_LEN];
	unsigned int signature;

};


// formatet for dataene for sidene skal være
struct SiderFormat {
	struct DocumentIndexFormat DocumentIndex;
	struct iindexMainElements iindex;
	int htmlSize;
	char description[1024];
	char title [70];
	char thumbnale[128];
	int thumbnailwidth;
	int thumbnailheight;
	struct cache_params cache_params;
	char domain[65];
	char servername[32];
	int posisjon;
	int deletet;
	unsigned int crc32;
	int type;

	#ifdef BLACK_BOX
		char uri[1024];
		char url[1024];
		char fulluri[1024];
	#else
		char uri[201];
		char url[201];
	#endif
	char user[64];
	float bid;
	struct subnamesFormat subname;
	char HtmlPreparsed;
	double pageGenerateTime;

	unsigned char pathlen;

	unsigned short DomainID;

	unsigned int n_urls;
	struct {
		char *url;
		char *uri;
		char *fulluri;
		char subname[64];
	} *urls;
	char *attributes;
	unsigned short attributelen;
	//buffering som gjør at vi kan lagre den på fil, og fortsatt bruke gammel data
	char reserved[26];
};


struct queryTimeFormat {
	double total;
	double indexSort;
	double dictionarySearch;
	double AnchorSearch;
	double MainSearch;
	double MainAnchorMerge;
	double allrankCalc;
	double popResult;
	double adultcalk;
	double dateview;
	double iintegerGetValueDate;
	double filetypes;
	double pathaccess;
	double urlrewrite;
	double getUserObjekt;
	double cmc_conect;
	double UrlSearch;
	double popRank;
	double responseShortning;
	double searchSimple;

	#ifdef BLACK_BOX
		double html_parser_run;
		double generate_snippet;
		double duplicat_echecking;
		double FilterCount;
	#endif
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
	int deleted;
};


struct filtypesFormat {
	char name[5]; //4 for navn og \0
	int nrof;
};


struct filterinfoElementsFormat {
	char name[50];
	char longname[50];
	char query[MaxQueryLen];
	int nrof;
	char checked; //bool
};


struct filterinfoFormat {
	struct filterinfoElementsFormat elements[MAXFILTERELEMENTS];
	int nrof;
};


struct filtersFormat {
	#ifdef BLACK_BOX
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
	char spellcheckedQuery[MaxQueryLen];
	struct queryTimeFormat queryTime;
	char servername[32];
	struct subnamesFormat subnames[MAX_COLLECTIONS];
	int nrOfSubnames;
	struct filtersTrapedFormat filtersTraped;
	int dates[11];
	int filtypesnrof;
        char errorstr[250];
        int errorstrlen;
	int responstype;
	struct filtersFormat filters;

	#ifdef ATTRIBUTES
		int	navigation_xml_len;
		char	*navigation_xml;
	#endif
};


//formatet på anchor filer
struct anchorfileFormat {
	unsigned int DocID;
	char text[50];
};


struct linkdb_block {
	unsigned int        DocID_from;
	unsigned int        DocID_to;
};


struct udfileFormat {
	unsigned char url[200];
	unsigned int DocID;
};


struct updateFormat {
    unsigned char       sha1[20];
    unsigned char       url[200];
    unsigned char       linktext[50];
    unsigned int        DocID_from;
};


struct mainIndexFormat {
    unsigned char       sha1[20];
    unsigned int        DocID;
};


struct wordsFormat {
        #ifdef PRESERVE_WORDS
                char word[maxWordlLen];
        #endif
        unsigned int WordID;
        unsigned int position;
        unsigned short occurrence;
	int unsortetIndexPosition;
};


struct queryNodeHederFormat {
	char query[MaxQueryLen];
	char subname[maxSubnameLength];
	int MaxsHits;
	int start;
	int filterOn;
	unsigned int getRank;
	char userip[16];
	char GeoIPcontry[3];
	char search_user[64];
	char HTTP_ACCEPT_LANGUAGE[12];
        char HTTP_USER_AGENT[64];
        char HTTP_REFERER[255];
	char AmazonAssociateTag[50];
        char AmazonSubscriptionId[50];
	char orderby[10];
	int anonymous;
	char navmenucfg[NAVMENUCFG_SIZE];
	int lang; // uses LANG_* constants in define.h 
	int outformat;
};


// formater for de forskjelige delene i et query
struct QueryDataForamt {
        char query[MaxQueryLen];
	query_array queryParsed;
	char queryhtml[MaxQueryLen*2+1];
	char userip[16];
	char subname[maxSubnameLength];
	int MaxsHits;
	int start;
	int filterOn;
	int outformat;
	double version;
	char GeoIPcontry[3];
	char search_user[64];
	char HTTP_ACCEPT_LANGUAGE[12];
        char HTTP_USER_AGENT[64];
        char HTTP_REFERER[255];
	char AmazonAssociateTag[50];
	char AmazonSubscriptionId[50];
	char orderby[10];

	#ifdef BLACK_BOX
		int anonymous;
	#endif
	char tkey[33]; // 32 bytes key +1 for \0
	char rankUrl[256];
	query_array search_user_as_query;
	char navmenucfg[NAVMENUCFG_SIZE]; // jun 16 09
	int lang;
	char nocache;
	char nolog;
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
	char url[1024];
	char uri[1024];
	char fulluri[1024];
};


struct anchorRepo {
	int magic;
	unsigned int DocID;
	size_t len;
};


struct anchorIndexFormat {
	off_t offset;
};


struct Crc32attrMapFormat {
	unsigned int crc32;
	char text[MAX_ATTRIB_LEN];
};

#endif //_DEFINE__H_


