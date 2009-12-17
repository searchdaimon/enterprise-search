#include "out_sdxml.h"
#include <locale.h>
#include <stdio.h>
#ifdef ATTRIBUTES
	#include "../attributes/attr_makexml.h"
	#include "qrewrite.h"
#endif
#include "../common/boithohome.h"
#include "../common/langToNr.h" // getLangCode
#include "../common/attributes.h" // next_attribute
#include "../ds/dcontainer.h"
#include "../ds/dset.h"

char *get_filetype_icon(char *ext) {
	static struct fte_data *fdata = NULL;
	if (fdata == NULL)
		fdata = fte_init(bfile("config/file_extensions.conf"));
 	/* TODO? fte_destroy(fdata) */

	static char *icon, *version;
	char *group, *descr;

	if (fdata == NULL) {
		errx(1, "No fte_data %d %s", __LINE__, __FILE__);
		return;
	}
	if (!fte_getdescription(fdata, "eng", ext, &group, &descr, &icon, &version)) {
		warnx("no icon for ext %s\n", ext);
		icon[0] = '\0';
		return icon;
	}

	return icon;
}



void disp_out_sd_v2_0(
	struct SiderHederFormat FinalSiderHeder,
        struct QueryDataForamt QueryData,
	int noDoctype,
        struct SiderHederFormat *SiderHeder,
	int hascashe,
	int hasprequery,
	int nrRespondedServers,
	int num_servers,
	int nrOfAddServers,
	struct filtersTrapedFormat dispatcherfiltersTraped,
	int *sockfd,
	int *addsockfd,
	struct SiderHederFormat *AddSiderHeder,
	struct errorhaFormat errorha,
	struct SiderFormat *Sider,
	struct queryNodeHederFormat queryNodeHeder,
	time_t etime
) {
	int i, x, y;
	char colchecked[20];

    	    printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?> \n");
    	    if (!noDoctype) {
	        printf("<!DOCTYPE family SYSTEM \"http://www.boitho.com/xml/search.dtd\"> \n");
    	    }

    	    printf("<SEARCH>\n");   
	    //får rare svar fra hilite. Dropper å bruke den får nå
	    FinalSiderHeder.hiliteQuery[0] = '\0';
	    #ifdef WITH_SPELLING
	    strsandr(SiderHeder->spellcheckedQuery, "\"","&quot;");
	    #endif
    	    printf("<RESULT_INFO TOTAL=\"%i\" SPELLCHECKEDQUERY=\"%s\" QUERY=\"%s\" HILITE=\"%s\" TIME=\"%f\" FILTERED=\"%i\" \
	        SHOWABAL=\"%i\" CASHE=\"%i\" \
		PREQUERY=\"%i\" GEOIPCONTRY=\"%s\" SUBNAME=\"%s\" BOITHOHOME=\"%s\" NROFSEARCHNODES=\"%i\" XMLVERSION=\"2.0\"/>\n",
		FinalSiderHeder.TotaltTreff,
		#ifdef WITH_SPELLING
		SiderHeder->spellcheckedQuery,
		#else
		"",
		#endif
		QueryData.queryhtml,
		FinalSiderHeder.hiliteQuery,
		FinalSiderHeder.total_usecs,
		FinalSiderHeder.filtered,
		FinalSiderHeder.showabal,
		hascashe,
		hasprequery,
		QueryData.GeoIPcontry,
		QueryData.subname,
		bfile(""),
		nrRespondedServers
	    );

	
	    //viser info om dispatcher_all
	    printf("<DISPATCHER_INFO>\n");
	    printf("\t<FILTERTRAPP>\n");
	    {
		printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",dispatcherfiltersTraped.filterAdultWeight_bool);
		printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",dispatcherfiltersTraped.filterAdultWeight_value);
		printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",dispatcherfiltersTraped.filterSameCrc32_1);
		printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",dispatcherfiltersTraped.filterSameUrl);
		printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",dispatcherfiltersTraped.filterNoUrl);
		printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",dispatcherfiltersTraped.find_domain_no_subname);
		printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",dispatcherfiltersTraped.filterSameDomain);
		printf("\t\t<filterTLDs>%i</filterTLDs>\n",dispatcherfiltersTraped.filterTLDs);
		printf("\t\t<filterResponse>%i</filterResponse>\n",dispatcherfiltersTraped.filterResponse);
		printf("\t\t<cantpopResult>%i</cantpopResult>\n",dispatcherfiltersTraped.cantpopResult);
		printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",dispatcherfiltersTraped.cmc_pathaccess);
		printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",dispatcherfiltersTraped.filterSameCrc32_2);
	    }
	    printf("\t</FILTERTRAPP>\n");
	    printf("</DISPATCHER_INFO>\n");


	    if ((!hascashe) && (!hasprequery)) {

		//viser info om serverne som svarte
		//printf("<SEARCHNODES_INFO NROFSEARCHNODES=\"%i\" />\n",nrRespondedServers);

		for (i=0;i< num_servers;i++) {
			if (sockfd[i] != 0) {
				printf("<SEARCHNODES>\n");
				printf("\t<NODENAME>%s</NODENAME>\n",SiderHeder[i].servername);
				printf("\t<TOTALTIME>%f</TOTALTIME>\n",SiderHeder[i].total_usecs);
				printf("\t<FILTERED>%i</FILTERED>\n",SiderHeder[i].filtered);
				printf("\t<HITS>%i</HITS>\n",SiderHeder[i].TotaltTreff);
				printf("\t<SHOWABAL>%i</SHOWABAL>\n",SiderHeder[i].showabal);

#ifndef DEBUG
				printf("\t<TIMES>\n");
				{
					printf("\t\t<AthorSearch>%f</AthorSearch>\n",SiderHeder[i].queryTime.AthorSearch);
					//printf("\t\t<AthorRank>%f</AthorRank>\n",SiderHeder[i].queryTime.AthorRank);
					printf("\t\t<UrlSearch>%f</UrlSearch>\n",SiderHeder[i].queryTime.UrlSearch);
					printf("\t\t<MainSearch>%f</MainSearch>\n",SiderHeder[i].queryTime.MainSearch);
					//printf("\t\t<MainRank>%f</MainRank>\n",SiderHeder[i].queryTime.MainRank);
					printf("\t\t<MainAthorMerge>%f</MainAthorMerge>\n",SiderHeder[i].queryTime.MainAthorMerge);
					printf("\t\t<popRank>%f</popRank>\n",SiderHeder[i].queryTime.popRank);
					printf("\t\t<responseShortning>%f</responseShortning>\n",SiderHeder[i].queryTime.responseShortning);

					printf("\t\t<allrankCalc>%f</allrankCalc>\n",SiderHeder[i].queryTime.allrankCalc);
					printf("\t\t<indexSort>%f</indexSort>\n",SiderHeder[i].queryTime.indexSort);
					printf("\t\t<searchSimple>%f</searchSimple>\n",SiderHeder[i].queryTime.searchSimple);

					printf("\t\t<popResult>%f</popResult>\n",SiderHeder[i].queryTime.popResult);
					printf("\t\t<adultcalk>%f</adultcalk>\n",SiderHeder[i].queryTime.adultcalk);

#ifdef BLACK_BOKS
					printf("\t\t<filetypes>%f</filetypes>\n",SiderHeder[i].queryTime.filetypes);
					printf("\t\t<iintegerGetValueDate>%f</iintegerGetValueDate>\n",SiderHeder[i].queryTime.iintegerGetValueDate);
					printf("\t\t<dateview>%f</dateview>\n",SiderHeder[i].queryTime.dateview);
					printf("\t\t<FilterCount>%f</FilterCount>\n",SiderHeder[i].queryTime.FilterCount);
					printf("\t\t<pathaccess>%f</pathaccess>\n",SiderHeder[i].queryTime.pathaccess);
					printf("\t\t<urlrewrite>%f</urlrewrite>\n",SiderHeder[i].queryTime.urlrewrite);
					printf("\t\t<getUserObjekt>%f</getUserObjekt>\n",SiderHeder[i].queryTime.getUserObjekt);
					printf("\t\t<cmc_conect>%f</cmc_conect>\n",SiderHeder[i].queryTime.cmc_conect);
#endif
					#ifdef BLACK_BOKS
					printf("\t\t<html_parser_run>%f</html_parser_run>\n",SiderHeder[i].queryTime.html_parser_run);
					printf("\t\t<generate_snippet>%f</generate_snippet>\n",SiderHeder[i].queryTime.generate_snippet);
					#endif
				}
				printf("\t</TIMES>\n");

				printf("\t<FILTERTRAPP>\n");
				{
					printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",SiderHeder[i].filtersTraped.filterAdultWeight_bool);
					printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",SiderHeder[i].filtersTraped.filterAdultWeight_value);
					printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",SiderHeder[i].filtersTraped.filterSameCrc32_1);
					printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",SiderHeder[i].filtersTraped.filterSameUrl);
					printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",SiderHeder[i].filtersTraped.filterNoUrl);
					printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",SiderHeder[i].filtersTraped.find_domain_no_subname);
					printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",SiderHeder[i].filtersTraped.filterSameDomain);
					printf("\t\t<filterTLDs>%i</filterTLDs>\n",SiderHeder[i].filtersTraped.filterTLDs);
					printf("\t\t<filterResponse>%i</filterResponse>\n",SiderHeder[i].filtersTraped.filterResponse);
					printf("\t\t<cantpopResult>%i</cantpopResult>\n",SiderHeder[i].filtersTraped.cantpopResult);
					printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",SiderHeder[i].filtersTraped.cmc_pathaccess);
					printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",SiderHeder[i].filtersTraped.filterSameCrc32_2);
				}
				printf("\t</FILTERTRAPP>\n");
#endif

				printf("</SEARCHNODES>\n");


			}	
		}
	    }
	    else {
		printf("<SEARCHNODES>\n");
		printf("\t<NODENAME>cashe.exactseek.com</NODENAME>\n");
		printf("\t<TOTALTIME>%f</TOTALTIME>\n",FinalSiderHeder.total_usecs);
		printf("\t<FILTERED>0</FILTERED>\n");
		printf("\t<HITS>%i</HITS>\n",FinalSiderHeder.TotaltTreff);
		printf("</SEARCHNODES>\n");
	    }

	    //cashe eller ingen cashe. Adserverene skal vises
	    for (i=0;i<nrOfAddServers;i++) {
       	        if (addsockfd[i] != 0) {
			printf("<SEARCHNODES>\n");
				printf("\t<NODENAME>%s</NODENAME>\n",AddSiderHeder[i].servername);
       	         		printf("\t<TOTALTIME>%f</TOTALTIME>\n",AddSiderHeder[i].total_usecs);
				printf("\t<FILTERED>%i</FILTERED>\n",AddSiderHeder[i].filtered);
				printf("\t<HITS>%i</HITS>\n",AddSiderHeder[i].TotaltTreff);
			printf("</SEARCHNODES>\n");
		}	
	    }


	    //hvis vi har noen feil viser vi de
	    for (i=0;i<errorha.nr;i++) {
		printf("<ERROR>\n");
		printf("  <ERRORCODE>%i</ERRORCODE>\n",errorha.errorcode[i]);
        	printf("  <ERRORMESSAGE>%s</ERRORMESSAGE>\n",errorha.errormessage[i]);
        	printf("</ERROR>\n");
	    }

	    #ifdef BLACK_BOKS

	    for(i=0;i<SiderHeder[0].filters.collections.nrof;i++) {
		if (SiderHeder[0].filters.collections.elements[i].nrof == -1)
			continue;

		if (SiderHeder[0].filters.collections.elements[i].checked) {
			strscpy(colchecked," SELECTED=\"TRUE\"",sizeof(colchecked));
		}
		else {
			strscpy(colchecked,"",sizeof(colchecked));
		}

		printf("<COLLECTION%s>\n",colchecked);
		printf("<NAME>%s</NAME>\n",SiderHeder[0].filters.collections.elements[i].name);
		printf("<TOTALRESULTSCOUNT>%i</TOTALRESULTSCOUNT>\n",SiderHeder[0].filters.collections.elements[i].nrof);

		printf("</COLLECTION>\n");

		
	    }


	    for (i=0;i<SiderHeder[0].filters.filtypes.nrof;i++) {
		printf("<FILETYPE>\n");

		printf("<FILENAME>%s</FILENAME>\n<FILELONGNAME>%s</FILELONGNAME>\n<FILENR>%i</FILENR>",
				SiderHeder[0].filters.filtypes.elements[i].name,
				SiderHeder[0].filters.filtypes.elements[i].longname,
				SiderHeder[0].filters.filtypes.elements[i].nrof);
		
		printf("</FILETYPE>\n");

	    }		

	    #ifdef ATTRIBUTES
	    printf("%s\n", SiderHeder[0].navigation_xml);
	    #endif

	    {
	    char *dateview_type_names[] = {
					"Today",
					"Yesterday",
					"Last 7 days",
					"Last 30 days",
					"This year",
					"Last year",
					"Older than two years"};


	    char *dateview_type_query[] = {
					" date:\"today\"",
					" date:\"yesterday\"",
					" date:\"this week\"",
					" date:\"this month\"",
					" date:\"this year\"",
					" date:\"last year\"",
					" date:\"two years plus\""};

	    char *dateview_type_query_short[] = {
					"today",
					"yesterday",
					"this week",
					"this month",
					"this year",
					"last year",
					"two years plus"};

	    // Bør gjøres sammen med attributter?
	    query_array	qa;
	    get_query(QueryData.query, strlen(QueryData.query), &qa);
	    container	*remove = set_container( int_container() );
	    buffer	*B = buffer_init(-1);
	    int		highlight_date = -1;

	    for (i=0; i<qa.n; i++)
		if (qa.query[i].operand == QUERY_DATE)
		    {
			set_insert(remove, i);
			for (y=0; y<7; y++)
			    {
				char	full_string[64];
				int qs_i;
				int pos = 0;
				for (qs_i=0; qs_i<qa.query[i].n; qs_i++)
				    pos+= sprintf(&(full_string[pos]), "%s%s", qs_i>0?" ":"", qa.query[i].s[qs_i]);

				if (!strcmp(dateview_type_query_short[y], full_string))
				    highlight_date = y;
			    }
		    }

	    bsprint_query_with_remove(B, remove, &qa, 1);
	    char	*basedatequery = buffer_exit(B);
	    char	xmlescapebuf1[2048];
	    char	xmlescapebuf2[2048];

	    printf("<group name=\"Date\" query=\"%s\" expanded=\"true\">\n", basedatequery);
		for (y=0;y<7;y++) {
		    printf("\t<item name=\"%s\" query=\"%s%s\" hits=\"%i\"%s />\n",
			dateview_type_names[y],
			basedatequery,
			xml_escape_attr(dateview_type_query[y], xmlescapebuf2, sizeof(xmlescapebuf2)),
			SiderHeder[0].dates[y],
			highlight_date==y ? " selected=\"true\"" : "");
		}
	    printf("</group>\n");
	    printf("</navigation>\n");

	    destroy(remove);
	    destroy_query(&qa);

	    }


	    #else

		#ifdef DEBUG
        	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-18s|%-10s|%-10s|\n",
                	"AllRank",
                	"TermRank",
                	"PopRank",
                	"Body",
                	"Headline",
                	"Tittel",
                	"Athor (nr)",
                	"UrlM",
                	"UrlDom",
                	"UrlSub"
                );
        	printf("|----------|----------|----------||----------|----------|----------|------------------|----------|----------|----------|\n");

                for(i=0;i<FinalSiderHeder.showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i)|%10i|%10i|%10i| %s\n",

				Sider[i].iindex.allrank,
                                Sider[i].iindex.TermRank,
                                Sider[i].iindex.PopRank,

                                Sider[i].iindex.rank_explaind.rankBody,
                                Sider[i].iindex.rank_explaind.rankHeadline,
                                Sider[i].iindex.rank_explaind.rankTittel,
                                Sider[i].iindex.rank_explaind.rankAthor,
                                Sider[i].iindex.rank_explaind.nrAthor,
                                Sider[i].iindex.rank_explaind.rankUrl_mainbody,
                                Sider[i].iindex.rank_explaind.rankUrlDomain,
                                Sider[i].iindex.rank_explaind.rankUrlSub,

                                Sider[i].DocumentIndex.Url
                                );
                }

		#endif
	#endif

	    //skal printe ut FinalSiderHeder.showabal sider, men noen av sidene kan være slettet

	    //x=0;
	    //i=0;
	    //regner ut hvor vi skal begynne og vise treff. Eks side 2 er fra 11-20
	    //i er hvor vi skal begynne
	    i = QueryData.MaxsHits * (QueryData.start -1);
	    x = i;
	    #ifdef DEBUG	
	    printf("x: %i, MaxsHits %i, start %i, showabal %i\n",x,QueryData.MaxsHits,QueryData.start,
		FinalSiderHeder.showabal);
	    #endif

	    while ((x<(QueryData.MaxsHits*QueryData.start)) && (x<FinalSiderHeder.showabal) && (i < (queryNodeHeder.MaxsHits * num_servers))) {
		
		if (!Sider[i].deletet) {


			#ifdef DEBUG
				printf("i %i, r %i, a: %i, bid : %f, u: %s. DocID: %u\n",i,Sider[i].iindex.allrank,Sider[i].DocumentIndex.AdultWeight,Sider[i].bid,Sider[i].url,Sider[i].iindex.DocID);
			#else


				if (Sider[i].type == siderType_ppctop) {
					printf("<RESULT_PPC>\n");
					printf("\t<BID>%f</BID>\n",Sider[i].bid);
					//++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt
				}
				else if (Sider[i].type == siderType_ppcside) {
					printf("<RESULT_PPCSIDE>\n");
					printf("\t<BID>%f</BID>\n",Sider[i].bid);
					//++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt 
				}
				else {
                			printf("<RESULT>\n");
				}

	                	printf("\t<DOCID>%i-%i</DOCID>\n",Sider[i].iindex.DocID,rLotForDOCid(Sider[i].iindex.DocID));


        	        	printf("\t<TITLE><![CDATA[%s]]></TITLE>\n",Sider[i].title);

                		//DocumentIndex
                		printf("\t<URL><![CDATA[%s]]></URL>\n",Sider[i].url);
                		printf("\t<URI><![CDATA[%s]]></URI>\n",Sider[i].uri);
#ifdef BLACK_BOKS
                		printf("\t<FULLURI><![CDATA[%s]]></FULLURI>\n",Sider[i].fulluri);
#endif

				{
					int j;

					for (j = 0; j < Sider[i].n_urls; j++) {
						printf("\t<DUPLICATESURLS>\n");
							printf("\t\t<URL><![CDATA[%s]]></URL>\n",Sider[i].urls[j].url);
							printf("\t\t<URI><![CDATA[%s]]></URI>\n",Sider[i].urls[j].uri);
							printf("\t\t<FULLURI><![CDATA[%s]]></FULLURI>\n",Sider[i].urls[j].fulluri);
						printf("\t</DUPLICATESURLS>\n");
					}
				}

				#ifdef ATTRIBUTES
				{
					char *o = NULL;
					char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN], keyvalue[MAX_ATTRIB_LEN];

					qrewrite qrewrite;
					qrewrite_init(&qrewrite, QueryData.query);
					char attbuff[MaxQueryLen], attrq_esc[MaxQueryLen * 4], attrq2_esc[MaxQueryLen * 4];
					char ekey[1024], evalue[1024];

					printf("\t<attributes>\n");
					while (next_attribute(Sider[i].attributes, &o, key, value, keyvalue)) {
						query_attr_set_filter(attbuff, sizeof attbuff, &qrewrite, key, value, 0);
						xml_escape_uri(attbuff, attrq_esc, sizeof attrq_esc);
						
						query_attr_set_filter(attbuff, sizeof attbuff, &qrewrite, key, value, 1);
						xml_escape_uri(attbuff, attrq2_esc, sizeof attrq2_esc);
						escapeHTML(ekey, sizeof ekey, key);
						escapeHTML(evalue, sizeof evalue, value);


						printf("\t<attribute key=\"%s\" value=\"%s\" query=\"%s\" attribute_query=\"%s\" />\n", 
							ekey, evalue, 
							attrq_esc, attrq2_esc);
					}
					printf("\t</attributes>\n");
					qrewrite_destroy(&qrewrite);
				}
				#endif

				//gjør om språk fra tall til code
				char documentlangcode[4];
				getLangCode(documentlangcode,atoi(Sider[i].DocumentIndex.Sprok));

				//finner vid
				char vidbuf[64];
        			vid_u(vidbuf,sizeof(vidbuf),salt,Sider[i].iindex.DocID,etime,QueryData.userip);
				printf("\t<VID>%s</VID>\n",vidbuf);


                		printf("\t<DOCUMENTLANGUAGE>%s</DOCUMENTLANGUAGE>\n", documentlangcode);
				printf("\t<DOCUMENTTYPE>%s</DOCUMENTTYPE>\n", Sider[i].DocumentIndex.Dokumenttype);
                		printf("\t<POSISJON>%i</POSISJON>\n",x);
                		printf("\t<REPOSITORYSIZE>%u</REPOSITORYSIZE>\n",Sider[i].DocumentIndex.htmlSize);
				printf("\t<filetype>%s</filetype>\n", Sider[i].iindex.filetype);
				printf("\t<icon>%s</icon>\n", get_filetype_icon(Sider[i].iindex.filetype));


				//if (!getRank) {
					if (Sider[i].thumbnale[0] != '\0') {
						printf("\t<THUMBNAIL>%s</THUMBNAIL>\n",Sider[i].thumbnale);

						printf("\t<THUMBNAILWIDTH>%i</THUMBNAILWIDTH>\n",Sider[i].thumbnailwidth);
						printf("\t<THUMBNAILHEIGHT>%i</THUMBNAILHEIGHT>\n",Sider[i].thumbnailheight);
					}
					else {
						printf("\t<THUMBNAIL></THUMBNAIL>\n");
						printf("\t<THUMBNAILWIDTH></THUMBNAILWIDTH>\n");
						printf("\t<THUMBNAILHEIGHT></THUMBNAILHEIGHT>\n");
					}

					printf("\t<DESCRIPTION_LENGTH>%i</DESCRIPTION_LENGTH>\n",strlen(Sider[i].description));
					printf("\t<DESCRIPTION_MAX>%i</DESCRIPTION_MAX>\n",sizeof(Sider[i].description));
					printf("\t<DESCRIPTION>%s</DESCRIPTION>\n",Sider[i].description);
				//}



				printf("\t<CRC32>%u</CRC32>\n",Sider[i].DocumentIndex.crc32);
	
				//ser ikke ut til at vi teller den
				//printf("\t<PAGEGENERATETIME>%f</PAGEGENERATETIME>\n",Sider[i].pageGenerateTime);

               			printf("\t<TERMRANK>%i</TERMRANK>\n",Sider[i].iindex.TermRank);

               			printf("\t<POPRANK>%i</POPRANK>\n",Sider[i].iindex.PopRank);
       	        		printf("\t<ALLRANK>%i</ALLRANK>\n",Sider[i].iindex.allrank);

                		printf("\t<NROFHITS>%i</NROFHITS>\n",Sider[i].iindex.TermAntall);
                		//printer ut hits (hvor i dokumenetet orde befinner seg ).
				/*
                		printf("\t<HITS>");
                		for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
                	        	printf("%hu ",Sider[i].iindex.hits[y]);
                		}
                		printf("</HITS>\n");
				*/

				printf("\t<RESULT_COLLECTION>%s</RESULT_COLLECTION>\n",Sider[i].subname.subname);


				#ifdef BLACK_BOKS
					char timebuf[64];
					if (Sider[i].DocumentIndex.CrawleDato != 0) {
						printf("\t<TIME_UNIX>%u</TIME_UNIX>\n",Sider[i].DocumentIndex.CrawleDato);
						// Magnus: Konverterer til locale:
				        	setlocale(LC_TIME, "no_NO.utf8");
						strftime(timebuf, 63, "%A %e. %b %Y %k:%M", localtime((time_t *)&Sider[i].DocumentIndex.CrawleDato));
						timebuf[64] = '\0';
						printf("\t<TIME_ISO>%s</TIME_ISO>\n",timebuf);
					}
					// Sender med cache link hvis 
					// collection er konfigurert til aa vise cache.
					if ((int) Sider[i].subname.config.cache_link)
	                			printf("\t<CACHE document=\"%u\" time=\"%u\" signature=\"%u\" collection=\"%s\" host=\"%s\"></CACHE>\n", 
							Sider[i].cache_params.doc_id, Sider[i].cache_params.time, 
							Sider[i].cache_params.signature, Sider[i].cache_params.subname, 
							Sider[i].cache_params.cache_host);

					else 
						printf("\t<CACHE></CACHE>\n");
					
					printf("\t<PAID_INCLUSION>%i</PAID_INCLUSION>\n",(int)Sider[i].subname.config.isPaidInclusion);

				#else
				
	                		printf("\t<DOMAIN>%s</DOMAIN>\n",Sider[i].domain);
	                		printf("\t<DOMAIN_ID>%hu</DOMAIN_ID>\n",Sider[i].DomainID);

					//finer om forige treff hadde samme domene
					if (i>0 && (lastdomain != NULL) && (strcmp(Sider[i].domain,lastdomain) == 0)) {			
		                		printf("\t<DOMAIN_GROUPED>TRUE</DOMAIN_GROUPED>\n");
					}
					else {
		                		printf("\t<DOMAIN_GROUPED>FALSE</DOMAIN_GROUPED>\n");

					}
					// ikke 100% riktig dette, da vi vil få problemer med at ppc reklame får samme side kan 
					// være siste, og da blir treff 1 rykket inn
					lastdomain = Sider[i].domain;

					printf("\t<SERVERNAME>%s</SERVERNAME>\n",Sider[i].servername);

	                		printf("\t<ADULTWEIGHT>%hu</ADULTWEIGHT>\n",Sider[i].DocumentIndex.AdultWeight);
	                		printf("\t<METADESCRIPTION><![CDATA[]]></METADESCRIPTION>\n");
	                		printf("\t<CATEGORY></CATEGORY>\n");
	                		printf("\t<OFFENSIVE_CODE>FALSE</OFFENSIVE_CODE>\n");


					ipaddr.s_addr = Sider[i].DocumentIndex.IPAddress;

                			printf("\t<IPADDRESS>%s</IPADDRESS>\n",inet_ntoa(ipaddr));

                			printf("\t<RESPONSE>%hu</RESPONSE>\n",Sider[i].DocumentIndex.response);
	
					printf("\t<CRAWLERVERSION>%f</CRAWLERVERSION>\n",Sider[i].DocumentIndex.clientVersion);
					printf("\t<HTMLPREPARSED>%i</HTMLPREPARSED>\n",Sider[i].HtmlPreparsed);

	                		printf("\t<CACHE document=\"%u\" time=\"%u\" signature=\"%u\" collection=\"%s\" host=\"%s\"></CACHE>\n", 
							Sider[i].cache_params.doc_id, Sider[i].cache_params.time, 
							Sider[i].cache_params.signature, Sider[i].cache_params.subname, 
							Sider[i].cache_params.cache_host);

	                		printf("\t<PAID_INCLUSION>%i</PAID_INCLUSION>\n",(int)Sider[i].subname.config.isPaidInclusion);

			#endif

			#ifdef EXPLAIN_RANK
				printf("\t<EXPLAIN_RANK>");
				print_explain_rank(&Sider[i],QueryData.queryhtml);
				printf("</EXPLAIN_RANK>\n");
			#endif
		
			if (Sider[i].type == siderType_ppctop ) {
				printf("</RESULT_PPC>\n");
			}
			else if (Sider[i].type == siderType_ppcside ) {
				printf("</RESULT_PPCSIDE>\n");
			}
			else {
                		printf("</RESULT>\n");
			}
		
                
			#endif

			//teller bare normale sider
			if (Sider[i].type == siderType_normal) {
				++x;
			}
		}
		else{ 
			warnx("nr %i er deletet. Rank %i\n",i,Sider[i].iindex.allrank);
		}
		
		++i;
	    }

	    printf("</SEARCH>\n");


}

void disp_out_sd_v2_1(
	struct SiderHederFormat FinalSiderHeder,
        struct QueryDataForamt QueryData,
	int noDoctype,
        struct SiderHederFormat *SiderHeder,
	int hascashe,
	int hasprequery,
	int nrRespondedServers,
	int num_servers,
	int nrOfAddServers,
	struct filtersTrapedFormat dispatcherfiltersTraped,
	int *sockfd,
	int *addsockfd,
	struct SiderHederFormat *AddSiderHeder,
	struct errorhaFormat errorha,
	struct SiderFormat *Sider,
	struct queryNodeHederFormat queryNodeHeder,
	time_t etime
) {
	int i, x, y;
	char colchecked[20];

    printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    	    if (!noDoctype) {
	        printf("<!DOCTYPE searchresults SYSTEM \"http://www.searchdaimon.com/xml/search.dtd\">\n");
    	    }

    	    printf("<search>\n");
	    //får rare svar fra hilite. Dropper å bruke den får nå
	    FinalSiderHeder.hiliteQuery[0] = '\0';
	    #ifdef WITH_SPELLING
	    strsandr(SiderHeder->spellcheckedQuery, "\"","&quot;");
	    #endif
    	    printf("<result_info query=\"%s\" spellcheckedquery=\"%s\" filtered=\"%i\" \
	        shown=\"%i\" total=\"%i\" cache=\"%i\" \
		prequery=\"%i\" time=\"%f\" geoipcountry=\"%s\" subname=\"%s\" boithohome=\"%s\" nrofsearchnodes=\"%i\" xmlversion=\"2.1\"/>\n",
		QueryData.queryhtml,
		#ifdef WITH_SPELLING
		SiderHeder->spellcheckedQuery,
		#else
		"",
		#endif
		FinalSiderHeder.filtered,
		FinalSiderHeder.showabal,
		FinalSiderHeder.TotaltTreff,
		hascashe,
		hasprequery,
		FinalSiderHeder.total_usecs,
		QueryData.GeoIPcontry,
		QueryData.subname,
		bfile(""),
		nrRespondedServers
	    );

	    //viser info om dispatcher_all
	    printf("<dispatcher_info>\n");
	    printf("\t<filters>\n");
	    {
		printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",dispatcherfiltersTraped.filterAdultWeight_bool);
		printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",dispatcherfiltersTraped.filterAdultWeight_value);
		printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",dispatcherfiltersTraped.filterSameCrc32_1);
		printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",dispatcherfiltersTraped.filterSameUrl);
		printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",dispatcherfiltersTraped.filterNoUrl);
		printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",dispatcherfiltersTraped.find_domain_no_subname);
		printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",dispatcherfiltersTraped.filterSameDomain);
		printf("\t\t<filterTLDs>%i</filterTLDs>\n",dispatcherfiltersTraped.filterTLDs);
		printf("\t\t<filterResponse>%i</filterResponse>\n",dispatcherfiltersTraped.filterResponse);
		printf("\t\t<cantpopResult>%i</cantpopResult>\n",dispatcherfiltersTraped.cantpopResult);
		printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",dispatcherfiltersTraped.cmc_pathaccess);
		printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",dispatcherfiltersTraped.filterSameCrc32_2);
	    }
	    printf("\t</filters>\n");
	    printf("</dispatcher_info>\n");


	    if ((!hascashe) && (!hasprequery)) {

		//viser info om serverne som svarte
		//printf("<SEARCHNODES_INFO NROFSEARCHNODES=\"%i\" />\n",nrRespondedServers);

		for (i=0;i< num_servers ;i++) {
			if (sockfd[i] != 0) {
				printf("<searchnode>\n");
				printf("\t<nodename>%s</nodename>\n",SiderHeder[i].servername);
				printf("\t<totaltime>%f</totaltime>\n",SiderHeder[i].total_usecs);
				printf("\t<hits>%i</hits>\n",SiderHeder[i].TotaltTreff);
				printf("\t<filtered>%i</filtered>\n",SiderHeder[i].filtered);
				printf("\t<shown>%i</shown>\n",SiderHeder[i].showabal);

#ifndef DEBUG
				printf("\t<time_profile>\n");
				{
					printf("\t\t<AuthorSearch>%f</AuthorSearch>\n",SiderHeder[i].queryTime.AthorSearch);
					//printf("\t\t<AthorRank>%f</AthorRank>\n",SiderHeder[i].queryTime.AthorRank);
					printf("\t\t<UrlSearch>%f</UrlSearch>\n",SiderHeder[i].queryTime.UrlSearch);
					printf("\t\t<MainSearch>%f</MainSearch>\n",SiderHeder[i].queryTime.MainSearch);
					//printf("\t\t<MainRank>%f</MainRank>\n",SiderHeder[i].queryTime.MainRank);
					printf("\t\t<MainAuthorMerge>%f</MainAuthorMerge>\n",SiderHeder[i].queryTime.MainAthorMerge);
					printf("\t\t<popRank>%f</popRank>\n",SiderHeder[i].queryTime.popRank);
					printf("\t\t<responseShortening>%f</responseShortening>\n",SiderHeder[i].queryTime.responseShortning);

					printf("\t\t<allrankCalc>%f</allrankCalc>\n",SiderHeder[i].queryTime.allrankCalc);
					printf("\t\t<indexSort>%f</indexSort>\n",SiderHeder[i].queryTime.indexSort);
					printf("\t\t<searchSimple>%f</searchSimple>\n",SiderHeder[i].queryTime.searchSimple);

					printf("\t\t<popResult>%f</popResult>\n",SiderHeder[i].queryTime.popResult);
					printf("\t\t<adultcalc>%f</adultcalc>\n",SiderHeder[i].queryTime.adultcalk);

#ifdef BLACK_BOKS
					printf("\t\t<filetypes>%f</filetypes>\n",SiderHeder[i].queryTime.filetypes);
					printf("\t\t<iintegerGetValueDate>%f</iintegerGetValueDate>\n",SiderHeder[i].queryTime.iintegerGetValueDate);
					printf("\t\t<dateview>%f</dateview>\n",SiderHeder[i].queryTime.dateview);
					printf("\t\t<FilterCount>%f</FilterCount>\n",SiderHeder[i].queryTime.FilterCount);
					printf("\t\t<pathaccess>%f</pathaccess>\n",SiderHeder[i].queryTime.pathaccess);
					printf("\t\t<urlrewrite>%f</urlrewrite>\n",SiderHeder[i].queryTime.urlrewrite);
					printf("\t\t<getUserObjekt>%f</getUserObjekt>\n",SiderHeder[i].queryTime.getUserObjekt);
					printf("\t\t<cmc_connect>%f</cmc_connect>\n",SiderHeder[i].queryTime.cmc_conect);
#endif
					#ifdef BLACK_BOKS
					printf("\t\t<html_parser_run>%f</html_parser_run>\n",SiderHeder[i].queryTime.html_parser_run);
					printf("\t\t<generate_snippet>%f</generate_snippet>\n",SiderHeder[i].queryTime.generate_snippet);
					#endif
				}
				printf("\t</time_profile>\n");

				printf("\t<filters>\n");
				{
					printf("\t\t<filterAdultWeight_bool>%i</filterAdultWeight_bool>\n",SiderHeder[i].filtersTraped.filterAdultWeight_bool);
					printf("\t\t<filterAdultWeight_value>%i</filterAdultWeight_value>\n",SiderHeder[i].filtersTraped.filterAdultWeight_value);
					printf("\t\t<filterSameCrc32_1>%i</filterSameCrc32_1>\n",SiderHeder[i].filtersTraped.filterSameCrc32_1);
					printf("\t\t<filterSameUrl>%i</filterSameUrl>\n",SiderHeder[i].filtersTraped.filterSameUrl);
					printf("\t\t<filterNoUrl>%i</filterNoUrl>\n",SiderHeder[i].filtersTraped.filterNoUrl);
					printf("\t\t<find_domain_no_subname>%i</find_domain_no_subname>\n",SiderHeder[i].filtersTraped.find_domain_no_subname);
					printf("\t\t<filterSameDomain>%i</filterSameDomain>\n",SiderHeder[i].filtersTraped.filterSameDomain);
					printf("\t\t<filterTLDs>%i</filterTLDs>\n",SiderHeder[i].filtersTraped.filterTLDs);
					printf("\t\t<filterResponse>%i</filterResponse>\n",SiderHeder[i].filtersTraped.filterResponse);
					printf("\t\t<cantpopResult>%i</cantpopResult>\n",SiderHeder[i].filtersTraped.cantpopResult);
					printf("\t\t<cmc_pathaccess>%i</cmc_pathaccess>\n",SiderHeder[i].filtersTraped.cmc_pathaccess);
					printf("\t\t<filterSameCrc32_2>%i</filterSameCrc32_2>\n",SiderHeder[i].filtersTraped.filterSameCrc32_2);
				}
				printf("\t</filters>\n");
#endif

				printf("</searchnode>\n");
			}	
		}
	    }
	    else {
		printf("<searchnode>\n");
		printf("\t<nodename>cache.exactseek.com</nodename>\n");
		printf("\t<totaltime>%f</totaltime>\n",FinalSiderHeder.total_usecs);
		printf("\t<hits>%i</hits>\n",FinalSiderHeder.TotaltTreff);
		printf("\t<filtered>0</filtered>\n");
		printf("</searchnode>\n");
	    }

	    //cashe eller ingen cashe. Adserverene skal vises
	    for (i=0;i<nrOfAddServers;i++) {
       	        if (addsockfd[i] != 0) {
			printf("<searchnode>\n");
				printf("\t<nodename>%s</nodename>\n",AddSiderHeder[i].servername);
       	         		printf("\t<totaltime>%f</totaltime>\n",AddSiderHeder[i].total_usecs);
				printf("\t<hits>%i</hits>\n",AddSiderHeder[i].TotaltTreff);
				printf("\t<filtered>%i</filtered>\n",AddSiderHeder[i].filtered);
			printf("</searchnode>\n");
		}	
	    }


	    //hvis vi har noen feil viser vi dem
	    for (i=0;i<errorha.nr;i++) {
		printf("<error>\n");
		printf("  <errorcode>%i</errorcode>\n",errorha.errorcode[i]);
        	printf("  <errormessage>%s</errormessage>\n",errorha.errormessage[i]);
        	printf("</error>\n");
	    }

	    #ifdef BLACK_BOKS

	    for(i=0;i<SiderHeder[0].filters.collections.nrof;i++) {
		if (SiderHeder[0].filters.collections.elements[i].nrof == -1)
			continue;

		if (SiderHeder[0].filters.collections.elements[i].checked) {
			strscpy(colchecked," selected=\"true\"",sizeof(colchecked));
		}
		else {
			strscpy(colchecked,"",sizeof(colchecked));
		}

		printf("<collection%s>\n",colchecked);
		printf("\t<name>%s</name>\n",SiderHeder[0].filters.collections.elements[i].name);
		printf("\t<totalresultscount>%i</totalresultscount>\n",SiderHeder[0].filters.collections.elements[i].nrof);

		printf("</collection>\n");

		
	    }


	    for (i=0;i<SiderHeder[0].filters.filtypes.nrof;i++) {
		printf("<filetype>\n");

		printf("<filename>%s</filename>\n<filelongname>%s</filelongname>\n<filenr>%i</filenr>",
				SiderHeder[0].filters.filtypes.elements[i].name,
				SiderHeder[0].filters.filtypes.elements[i].longname,
				SiderHeder[0].filters.filtypes.elements[i].nrof);
		
		printf("</filetype>\n");

	    }		

	    #ifdef ATTRIBUTES
	    printf("%s\n", SiderHeder[0].navigation_xml);
	    #endif

	    {

	    char *dateview_type_names[] = {
					"Today",
					"Yesterday",
					"Last 7 days",
					"Last 30 days",
					"This year",
					"Last year",
					"Older than two years"};

	    char *dateview_type_query[] = {
					" date:\"today\"",
					" date:\"yesterday\"",
					" date:\"this week\"",
					" date:\"this month\"",
					" date:\"this year\"",
					" date:\"last year\"",
					" date:\"two years plus\""};

	    char *dateview_type_query_short[] = {
					"today",
					"yesterday",
					"this week",
					"this month",
					"this year",
					"last year",
					"two years plus"};

	    // Bør gjøres sammen med attributter?
	    query_array	qa;
	    get_query(QueryData.query, strlen(QueryData.query), &qa);
	    container	*remove = set_container( int_container() );
	    buffer	*B = buffer_init(-1);
	    int		highlight_date = -1;

	    for (i=0; i<qa.n; i++)
		if (qa.query[i].operand == QUERY_DATE)
		    {
			set_insert(remove, i);
			for (y=0; y<7; y++)
			    {
				char	full_string[64];
				int qs_i;
				int pos = 0;
				for (qs_i=0; qs_i<qa.query[i].n; qs_i++)
				    pos+= sprintf(&(full_string[pos]), "%s%s", qs_i>0?" ":"", qa.query[i].s[qs_i]);

				if (!strcmp(dateview_type_query_short[y], full_string))
				    highlight_date = y;
			    }
		    }

	    bsprint_query_with_remove(B, remove, &qa, 0);
	    char	*basedatequery = buffer_exit(B);
	    char	xmlescapebuf1[2048];
	    char	xmlescapebuf2[2048];

	    printf("<group name=\"Date\" query=\"%s\" expanded=\"true\">\n", 
			xml_escape_uri(basedatequery, xmlescapebuf1, sizeof(xmlescapebuf1)));
		for (y=0;y<7;y++) {
		    printf("\t<item name=\"%s\" query=\"%s%s\" hits=\"%i\"%s />\n",
			dateview_type_names[y],
			xml_escape_uri(basedatequery, xmlescapebuf1, sizeof(xmlescapebuf1)),
			xml_escape_uri(dateview_type_query[y], xmlescapebuf2, sizeof(xmlescapebuf2)),
			SiderHeder[0].dates[y],
			highlight_date==y ? " selected=\"true\"" : "");
		}
	    printf("</group>\n");
	    printf("</navigation>\n");

	    destroy(remove);
	    destroy_query(&qa);

	    }

	    #else

		#ifdef DEBUG
        	printf("|%-10s|%-10s|%-10s||%-10s|%-10s|%-10s|%-18s|%-10s|%-10s|\n",
                	"AllRank",
                	"TermRank",
                	"PopRank",
                	"Body",
                	"Headline",
                	"Tittel",
                	"Athor (nr)",
                	"UrlM",
                	"UrlDom",
                	"UrlSub"
                );
        	printf("|----------|----------|----------||----------|----------|----------|------------------|----------|----------|----------|\n");

                for(i=0;i<FinalSiderHeder.showabal;i++) {
                        printf("|%10i|%10i|%10i||%10i|%10i|%10i|%10i (%5i)|%10i|%10i|%10i| %s\n",

				Sider[i].iindex.allrank,
                                Sider[i].iindex.TermRank,
                                Sider[i].iindex.PopRank,

                                Sider[i].iindex.rank_explaind.rankBody,
                                Sider[i].iindex.rank_explaind.rankHeadline,
                                Sider[i].iindex.rank_explaind.rankTittel,
                                Sider[i].iindex.rank_explaind.rankAthor,
                                Sider[i].iindex.rank_explaind.nrAthor,
                                Sider[i].iindex.rank_explaind.rankUrl_mainbody,
                                Sider[i].iindex.rank_explaind.rankUrlDomain,
                                Sider[i].iindex.rank_explaind.rankUrlSub,

                                Sider[i].DocumentIndex.Url
                                );
                }

		#endif
	#endif

	    //skal printe ut FinalSiderHeder.showabal sider, men noen av sidene kan være slettet

	    //x=0;
	    //i=0;
	    //regner ut hvor vi skal begynne og vise treff. Eks side 2 er fra 11-20
	    //i er hvor vi skal begynne
	    i = QueryData.MaxsHits * (QueryData.start -1);
	    x = i;
	    #ifdef DEBUG	
	    printf("x: %i, MaxsHits %i, start %i, showabal %i\n",x,QueryData.MaxsHits,QueryData.start,
		FinalSiderHeder.showabal);
	    #endif

	    while ((x<(QueryData.MaxsHits*QueryData.start)) && (x<FinalSiderHeder.showabal) && (i < (queryNodeHeder.MaxsHits * num_servers))) {
		
		if (!Sider[i].deletet) {


			#ifdef DEBUG
				printf("i %i, r %i, a: %i, bid : %f, u: %s. DocID: %u\n",i,Sider[i].iindex.allrank,Sider[i].DocumentIndex.AdultWeight,Sider[i].bid,Sider[i].url,Sider[i].iindex.DocID);
			#else


				if (Sider[i].type == siderType_ppctop) {
					printf("<result_ppc>\n");
					printf("\t<bid>%f</bid>\n",Sider[i].bid);
					//++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt
				}
				else if (Sider[i].type == siderType_ppcside) {
					printf("<result_ppcpage>\n");
					printf("\t<bid>%f</bid>\n",Sider[i].bid);
					//++totlaAds; //ikke helt bra denne. Vi teller antall anonser vist, ikke totalt 
				}
				else {
                			printf("<result>\n");
				}

	                	printf("\t<docid>%i-%i</docid>\n",Sider[i].iindex.DocID,rLotForDOCid(Sider[i].iindex.DocID));


        	        	printf("\t<title><![CDATA[%s]]></title>\n",Sider[i].title);

                		//DocumentIndex
                		printf("\t<url><![CDATA[%s]]></url>\n",Sider[i].url);
                		printf("\t<uri><![CDATA[%s]]></uri>\n",Sider[i].uri);
#ifdef BLACK_BOKS
                		printf("\t<fulluri><![CDATA[%s]]></fulluri>\n",Sider[i].fulluri);
#endif

				{
					int j;

					for (j = 0; j < Sider[i].n_urls; j++) {
						printf("\t<duplicateurl>\n");
							printf("\t\t<url><![CDATA[%s]]></url>\n",Sider[i].urls[j].url);
							printf("\t\t<uri><![CDATA[%s]]></uri>\n",Sider[i].urls[j].uri);
							printf("\t\t<fulluri><![CDATA[%s]]></fulluri>\n",Sider[i].urls[j].fulluri);
						printf("\t</duplicateurl>\n");
					}
				}

				#ifdef ATTRIBUTES
				{
					char *o = NULL;
					char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN], keyvalue[MAX_ATTRIB_LEN];

					qrewrite qrewrite;
					qrewrite_init(&qrewrite, QueryData.query);
					char attbuff[MaxQueryLen], attrq_esc[MaxQueryLen * 4], attrq2_esc[MaxQueryLen * 4];
					char ekey[1024], evalue[1024];

					printf("\t<attributes>\n");
					while (next_attribute(Sider[i].attributes, &o, key, value, keyvalue)) {
						query_attr_set_filter(attbuff, sizeof attbuff, &qrewrite, key, value, 0);
						xml_escape_uri(attbuff, attrq_esc, sizeof attrq_esc);
						
						query_attr_set_filter(attbuff, sizeof attbuff, &qrewrite, key, value, 1);
						xml_escape_uri(attbuff, attrq2_esc, sizeof attrq2_esc);
						escapeHTML(ekey, sizeof ekey, key);
						escapeHTML(evalue, sizeof evalue, value);


						printf("\t<attribute key=\"%s\" value=\"%s\" query=\"%s\" attribute_query=\"%s\" />\n", 
							ekey, evalue, 
							attrq_esc, attrq2_esc);
					}
					printf("\t</attributes>\n");
					qrewrite_destroy(&qrewrite);
				}
				#endif

				//gjør om språk fra tall til code
				char documentlangcode[4];
				getLangCode(documentlangcode,atoi(Sider[i].DocumentIndex.Sprok));

				//finner vid
				char vidbuf[64];
        			vid_u(vidbuf,sizeof(vidbuf),salt,Sider[i].iindex.DocID,etime,QueryData.userip);
				printf("\t<vid>%s</vid>\n",vidbuf);


                		printf("\t<documentlanguage>%s</documentlanguage>\n", documentlangcode);
				printf("\t<documenttype>%s</documenttype>\n", Sider[i].DocumentIndex.Dokumenttype);
                		printf("\t<position>%i</position>\n",x);
                		printf("\t<repositorysize>%u</repositorysize>\n",Sider[i].DocumentIndex.htmlSize);
				printf("\t<filetype>%s</filetype>\n", Sider[i].iindex.filetype);
				printf("\t<icon>%s</icon>\n", get_filetype_icon(Sider[i].iindex.filetype));


				//if (!getRank) {
					if (Sider[i].thumbnale[0] != '\0') {
						printf("\t<thumbnail width=\"%i\" height=\"%i\">%s</thumbnail>\n",
						    Sider[i].thumbnailwidth, Sider[i].thumbnailheight, Sider[i].thumbnale);
					}
					else {
						printf("\t<thumbnail></thumbnail>\n");
					}

					printf("\t<description>%s</description>\n",
					    Sider[i].description);
				//}



				printf("\t<crc32>%u</crc32>\n",Sider[i].DocumentIndex.crc32);
	
				//ser ikke ut til at vi teller den
				//printf("\t<PAGEGENERATETIME>%f</PAGEGENERATETIME>\n",Sider[i].pageGenerateTime);

               			printf("\t<termrank>%i</termrank>\n",Sider[i].iindex.TermRank);

               			printf("\t<poprank>%i</poprank>\n",Sider[i].iindex.PopRank);
       	        		printf("\t<allrank>%i</allrank>\n",Sider[i].iindex.allrank);

                		printf("\t<nrofhits>%i</nrofhits>\n",Sider[i].iindex.TermAntall);
                		//printer ut hits (hvor i dokumenetet orde befinner seg ).
				/*
                		printf("\t<HITS>");
                		for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
                	        	printf("%hu ",Sider[i].iindex.hits[y]);
                		}
                		printf("</HITS>\n");
				*/

				printf("\t<result_collection>%s</result_collection>\n",Sider[i].subname.subname);


				#ifdef BLACK_BOKS
					char timebuf[64];
					if (Sider[i].DocumentIndex.CrawleDato != 0) {
						printf("\t<time_unix>%u</time_unix>\n",Sider[i].DocumentIndex.CrawleDato);
						// Magnus: Konverterer til locale:
				        	setlocale(LC_TIME, "no_NO.utf8");
						strftime(timebuf, 63, "%A %e. %b %Y %k:%M", localtime((time_t *)&Sider[i].DocumentIndex.CrawleDato));
						timebuf[64] = '\0';
						printf("\t<time_iso>%s</time_iso>\n",timebuf);
					}
					// Sender med cache link hvis 
					// collection er konfigurert til aa vise cache.
					if ((int) Sider[i].subname.config.cache_link)
	                			printf("\t<cache document=\"%u\" time=\"%u\" signature=\"%u\" collection=\"%s\" host=\"%s\" />\n", 
							Sider[i].cache_params.doc_id, Sider[i].cache_params.time, 
							Sider[i].cache_params.signature, Sider[i].cache_params.subname, 
							Sider[i].cache_params.cache_host);
					else 
						printf("\t<cache></cache>\n");
					
					printf("\t<paid_inclusion>%i</paid_inclusion>\n",(int)Sider[i].subname.config.isPaidInclusion);

				#else
				
	                		printf("\t<domain>%s</domain>\n",Sider[i].domain);
	                		printf("\t<domain_id>%hu</domain_id>\n",Sider[i].DomainID);

					//finer om forige treff hadde samme domene
					if (i>0 && (lastdomain != NULL) && (strcmp(Sider[i].domain,lastdomain) == 0)) {			
		                		printf("\t<domain_grouped>true</domain_grouped>\n");
					}
					else {
		                		printf("\t<domain_grouped>false</domain_grouped>\n");

					}
					// ikke 100% riktig dette, da vi vil få problemer med at ppc reklame får samme side kan 
					// være siste, og da blir treff 1 rykket inn
					lastdomain = Sider[i].domain;

					printf("\t<servername>%s</servername>\n",Sider[i].servername);

	                		printf("\t<adultweight>%hu</adultweight>\n",Sider[i].DocumentIndex.AdultWeight);
	                		printf("\t<metadescription><![CDATA[]]></metadescription>\n");
	                		printf("\t<category></category>\n");
	                		printf("\t<offensice_code>false</offensive_code>\n");


					ipaddr.s_addr = Sider[i].DocumentIndex.IPAddress;

                			printf("\t<ipaddress>%s</ipaddress>\n",inet_ntoa(ipaddr));

                			printf("\t<response>%hu</response>\n",Sider[i].DocumentIndex.response);
	
					printf("\t<crawlerversion>%f</crawlerversion>\n",Sider[i].DocumentIndex.clientVersion);
					printf("\t<htmlpreparsed>%i</htmlpreparsed>\n",Sider[i].HtmlPreparsed);

					printf("\t<cache document=\"%u\" time=\"%u\" signature=\"%u\" collection=\"%s\" host=\"%s\" />\n", 
							Sider[i].cache_params.doc_id, Sider[i].cache_params.time, 
							Sider[i].cache_params.signature, Sider[i].cache_params.subname, 
							Sider[i].cache_params.cache_host);
	


	                		printf("\t<paid_inclusion>%i</paid_inclusion>\n",(int)Sider[i].subname.config.isPaidInclusion);

			#endif

			#ifdef EXPLAIN_RANK
				printf("\t<explain_rank>");
				print_explain_rank(&Sider[i],QueryData.queryhtml);
				printf("</explain_rank>\n");
			#endif
		
			if (Sider[i].type == siderType_ppctop ) {
				printf("</result_ppc>\n");
			}
			else if (Sider[i].type == siderType_ppcside ) {
				printf("</result_ppcpage>\n");
			}
			else {
                		printf("</result>\n");
			}
		
                
			#endif

			//teller bare normale sider
			if (Sider[i].type == siderType_normal) {
				++x;
			}
		}
		else{ 
			warnx("nr %i er deletet. Rank %i\n",i,Sider[i].iindex.allrank);
		}
		
		++i;
	    }

	    printf("</search>\n");
}


