#define _GNU_SOURCE

#include <locale.h>
#include <stdio.h>
#include <err.h>

#include "out_sdjson.h"
#include "out_common.h"

#ifdef ATTRIBUTES
	#include "../attributes/attr_makexml.h"
	#include "qrewrite.h"
#endif

#include "../common/boithohome.h"
#include "../common/langToNr.h" // getLangCode
#include "../common/attributes.h" // next_attribute
#include "../common/bstr.h"
#include "../common/lot.h"
#include "../common/xml.h"
#include "../common/cgi.h"
#include "../common/vid.h"
#include "../ds/dcontainer.h"
#include "../ds/dset.h"
#include "../ccan/json/json.h"

void disp_out_sd_json(

	struct SiderHederFormat FinalSiderHeder,
        struct QueryDataForamt QueryData,
	int noDoctype,
        struct SiderHederFormat *SiderHeder,
	int hascashe,
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
	char *tmps;

	JsonNode *results_info = json_mkobject();
	JsonNode *root = json_mkobject();

	//får rare svar fra hilite. Dropper å bruke den får nå
	FinalSiderHeder.hiliteQuery[0] = '\0';
	#ifdef WITH_SPELLING
	strsandr(SiderHeder->spellcheckedQuery, "\"","&quot;");
	#endif


	json_append_member(results_info, "query", json_mkstring(QueryData.queryhtml) );
	#ifdef WITH_SPELLING
	json_append_member(results_info, "spellcheckedquery", json_mkstring(	SiderHeder->spellcheckedQuery) );
	#endif
	json_append_member(results_info, "filtered", json_mknumber(FinalSiderHeder.filtered) );
	json_append_member(results_info, "shown", json_mknumber(FinalSiderHeder.showabal) );
	json_append_member(results_info, "total", json_mknumber(FinalSiderHeder.TotaltTreff) );
	json_append_member(results_info, "cache", json_mkbool( hascashe ) );
	json_append_member(results_info, "time", json_mknumber(FinalSiderHeder.total_usecs) );
	json_append_member(results_info, "jsonversion", json_mkstring("2.2") );

	json_append_member(root,"results_info", results_info);


	// If we are asked to not generate any hits we don't have to generate navigation menu other.
	if (QueryData.MaxsHits == 0) {
	        tmps = json_stringify(root, "\t");
	        puts(tmps);
	        free(tmps);

		json_delete(results_info);
		json_delete(root);

		return;
	}


/*

	    //hvis vi har noen feil viser vi dem
	    for (i=0;i<errorha.nr;i++) {
		printf("<error>\n");
		printf("  <errorcode>%i</errorcode>\n",errorha.errorcode[i]);
        	printf("  <errormessage>%s</errormessage>\n",errorha.errormessage[i]);
        	printf("</error>\n");
	    }
*/

	// collections
	JsonNode *collections = json_mkarray();
	for(i=0;i<SiderHeder[0].filters.collections.nrof;i++) {

		JsonNode *collection = json_mkobject();

		if (SiderHeder[0].filters.collections.elements[i].nrof == -1)
			continue;

		if (SiderHeder[0].filters.collections.elements[i].checked) {
			strscpy(colchecked," selected=\"true\"",sizeof(colchecked));
		}
		else {
			strscpy(colchecked,"",sizeof(colchecked));
		}


		json_append_member(collection, "name", json_mkstring(SiderHeder[0].filters.collections.elements[i].name) );
		json_append_member(collection, "totalresultscount", json_mknumber(SiderHeder[0].filters.collections.elements[i].nrof) );


		json_append_element(collections, collection );
	}
	json_append_member(root, "collections", collections );



	// filtypes (currently not in use. Integrated in normal navconfig insted)
	if (SiderHeder[0].filters.filtypes.nrof!=0) {
		JsonNode *filetypes = json_mkarray();
		for (i=0;i<SiderHeder[0].filters.filtypes.nrof;i++) {
			JsonNode *filetype = json_mkobject();

			json_append_member(filetype, "filename", json_mkstring(SiderHeder[0].filters.filtypes.elements[i].name) );
			json_append_member(filetype, "filelongname", json_mkstring(SiderHeder[0].filters.filtypes.elements[i].longname) );
			json_append_member(filetype, "filenr", json_mknumber(SiderHeder[0].filters.filtypes.elements[i].nrof) );

			json_append_element(filetypes, filetype );
		}		
		json_append_member(root, "filetypes", filetypes );
	}

	JsonNode *navigations = json_mkarray();


	#ifdef ATTRIBUTES
		// Vi definerer <navigation> i searchdbb da vi vil ha med query. Hvis noe gikk feil må vi derfor definere den her også 
		if (SiderHeder[0].navigation_xml[0] != '\0') {
		    	//printf("%s\n", SiderHeder[0].navigation_xml);
			JsonNode *nav = json_decode( SiderHeder[0].navigation_xml );
			if (nav==NULL) {
				fprintf(stderr,"Can't decode json!\n-\n-%s-\n", SiderHeder[0].navigation_xml);
			}
			else {	
				JsonNode *triple;
				json_foreach(triple, nav)
					json_append_element(navigations, triple);

				//json_append_element(navigations, nav);
			}
			//json_append_member(navigation, "attribs", json_decode( SiderHeder[0].navigation_xml ) );
		}
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

	    JsonNode *dates = json_mkarray();

		for (y=0;y<7;y++) {
		    	JsonNode *date = json_mkobject();

			json_append_member(date, "name", json_mkstring(dateview_type_names[y]) );
			if (asprintf(&tmps, "%s%s", basedatequery,dateview_type_query[y]) > 0) {
				json_append_member(date, "query", json_mkstring(tmps) );
			}
			free(tmps);
			json_append_member(date, "hits", json_mknumber(SiderHeder[0].dates[y]) );
			if (highlight_date==y) {
				json_append_member(date, "selected", json_mkbool(1) );
			}
		    	json_append_element(dates, date );
		}

                JsonNode *dategroup = json_mkobject();
                json_append_member(dategroup, "name", json_mkstring("Date") );
	        json_append_member(dategroup, "items", dates );

	        json_append_element(navigations, dategroup);

	        json_append_member(root, "navigation", navigations );


	        destroy(remove);
	        destroy_query(&qa);

	    }

	    i = QueryData.MaxsHits * (QueryData.start -1);
	    x = i;
	    #ifdef DEBUG	
		    printf("x: %i, MaxsHits %i, start %i, showabal %i\n",x,QueryData.MaxsHits,QueryData.start,
			FinalSiderHeder.showabal);
	    #endif

	    JsonNode *results = json_mkarray();

	    while ((x<(QueryData.MaxsHits*QueryData.start)) && (x<FinalSiderHeder.showabal) && (i < (queryNodeHeder.MaxsHits * num_servers))) {

		
		if (!Sider[i].deletet) {

			JsonNode *result = json_mkobject();


			json_append_member(result, "docid", json_mknumber(Sider[i].iindex.DocID) );
			json_append_member(result, "title", json_mkstring(Sider[i].title) );
			json_append_member(result, "url", json_mkstring(Sider[i].url) );
			json_append_member(result, "uri", json_mkstring(Sider[i].uri) );
			#ifdef BLACK_BOX
			json_append_member(result, "fulluri", json_mkstring(Sider[i].fulluri) );
			#endif

			{
				int j;
				JsonNode *duplicateurls = json_mkarray();

				for (j = 0; j < Sider[i].n_urls; j++) {
					JsonNode *duplicateurl = json_mkobject();

					json_append_member(duplicateurl, "url", json_mkstring(Sider[i].urls[j].url) );
					json_append_member(duplicateurl, "uri", json_mkstring(Sider[i].urls[j].uri) );
					json_append_member(duplicateurl, "fulluri", json_mkstring(Sider[i].urls[j].fulluri) );

					json_append_element(duplicateurls, duplicateurl );

				}

				json_append_member(result, "duplicateurls", duplicateurls );
			}




			#ifdef ATTRIBUTES
			{

				JsonNode *attributes = json_mkarray();


				char *o = NULL;
				char key[MAX_ATTRIB_LEN], value[MAX_ATTRIB_LEN], keyvalue[MAX_ATTRIB_LEN];

				qrewrite qrewrite;
				qrewrite_init(&qrewrite, QueryData.query);
				char att_query[MaxQueryLen], att_attribute_query[MaxQueryLen], attbuff[MaxQueryLen];

				while (next_attribute(Sider[i].attributes, &o, key, value, keyvalue)) {

					JsonNode *attribute = json_mkobject();

					query_attr_set_filter(att_query, sizeof attbuff, &qrewrite, key, value, 0);
						
					query_attr_set_filter(att_attribute_query, sizeof attbuff, &qrewrite, key, value, 1);

					// Runarb: 13.04.2012: Temporery fix for geting bad utf data from http://datasets.opentestset.com/datasets/Enron_files/full/arora-h/McKinsey%20Enterprise%20Report%2011-00.ppt						
					int len;
					for(len=0;len<strlen(value);len++) {
						if ((unsigned int)value[len] < 31) {
							value[len] = 'X';
						}
					}

						
					json_append_member(attribute, "key", json_mkstring(key) );
					json_append_member(attribute, "value", json_mkstring(value) );
					json_append_member(attribute, "query", json_mkstring(att_query) );
					json_append_member(attribute, "attribute_query", json_mkstring(att_attribute_query) );

					json_append_element(attributes, attribute );
				}
				qrewrite_destroy(&qrewrite);

				json_append_member(result, "attributes", attributes );

			}
			#endif

				//gjør om språk fra tall til code
				char documentlangcode[4];
				getLangCode(documentlangcode,atoi(Sider[i].DocumentIndex.Sprok));

				//finner vid
				char vidbuf[64];
				vid_u(vidbuf,sizeof(vidbuf),salt,Sider[i].iindex.DocID,etime,QueryData.userip);

				json_append_member(result, "vid", json_mkstring(vidbuf) );
				json_append_member(result, "documentlanguage", json_mkstring(documentlangcode) );
				json_append_member(result, "documenttype", json_mkstring(Sider[i].DocumentIndex.Dokumenttype) );
				json_append_member(result, "position", json_mknumber(x) );
				json_append_member(result, "repositorysize", json_mknumber(Sider[i].DocumentIndex.htmlSize) );
				json_append_member(result, "filetype", json_mkstring(Sider[i].iindex.filetype) );
				json_append_member(result, "icon", json_mkstring(get_filetype_icon(Sider[i].iindex.filetype)) );


				if (Sider[i].thumbnale[0] != '\0') {
					JsonNode *thumbnales = json_mkarray();

						JsonNode *thumbnale = json_mkobject();

						json_append_member(thumbnale, "width", json_mknumber(Sider[i].thumbnailwidth) );
						json_append_member(thumbnale, "height", json_mknumber(Sider[i].thumbnailheight) );
						json_append_member(thumbnale, "url", json_mkstring(Sider[i].thumbnale) );

						json_append_element(thumbnales, thumbnale );

					json_append_member(result, "thumbnale", thumbnales );
					
				}
				else {
					json_append_member(result, "thumbnail", json_mknull() );
				}

				json_append_member(result, "description", json_mkstring(Sider[i].description) );
				json_append_member(result, "crc32", json_mknumber(Sider[i].DocumentIndex.crc32) );
				json_append_member(result, "termrank", json_mknumber(Sider[i].iindex.TermRank) );
				json_append_member(result, "poprank", json_mknumber(Sider[i].iindex.PopRank) );
				json_append_member(result, "allrank", json_mknumber(Sider[i].iindex.allrank) );
				json_append_member(result, "nrofhits", json_mknumber(Sider[i].iindex.TermAntall) );

				// printer ut hits (hvor i dokumenetet orde befinner seg ).
				/*
				printf("\t<HITS>");
				for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
					printf("%hu ",Sider[i].iindex.hits[y]);
				}
				printf("</HITS>\n");
				*/

				json_append_member(result, "result_collection", json_mkstring(Sider[i].subname.subname) );

				#ifdef BLACK_BOX
					char timebuf[64];
					if (Sider[i].DocumentIndex.CrawleDato != 0) {

						json_append_member(result, "time_unix", json_mknumber(Sider[i].DocumentIndex.CrawleDato) );

						// Konverterer til locale:
						if (strcmp(queryNodeHeder.HTTP_ACCEPT_LANGUAGE,"no") == 0) {
							setlocale(LC_TIME, "no_NO.utf8");
						}
						else if (strcmp(queryNodeHeder.HTTP_ACCEPT_LANGUAGE,"fr") == 0) {
							setlocale(LC_TIME, "fr_FR.utf8");
						}
						else {
							setlocale(LC_TIME, "en_US.utf8");
						}
						strftime(timebuf, 63, "%A %e. %b %Y %k:%M", localtime((time_t *)&Sider[i].DocumentIndex.CrawleDato));
						timebuf[64] = '\0';
						json_append_member(result, "time_iso", json_mkstring(timebuf) );

					}
					// Sender med cache link hvis 
					// collection er konfigurert til aa vise cache.
					if ((int) Sider[i].subname.config.cache_link) {

						JsonNode *caches = json_mkarray();

							JsonNode *cache = json_mkobject();

							json_append_member(cache, "document", json_mknumber(Sider[i].cache_params.doc_id) );
							json_append_member(cache, "time", json_mknumber(Sider[i].cache_params.time) );
							json_append_member(cache, "signature", json_mknumber(Sider[i].cache_params.signature) );
							json_append_member(cache, "collection", json_mkstring(Sider[i].cache_params.subname) );
							json_append_member(cache, "host", json_mkstring(Sider[i].cache_params.cache_host) );

							json_append_element(caches, cache );

						json_append_member(result, "cache", caches );

					}
					else {
						json_append_member(result, "cache", json_mknull() );
					}

					json_append_member(result, "paid_inclusion", json_mkbool(Sider[i].subname.config.isPaidInclusion) );


			#endif

			#ifdef EXPLAIN_RANK
				printf("\t<explain_rank>");
				print_explain_rank(&Sider[i],QueryData.queryhtml);
				printf("</explain_rank>\n");
			#endif
			

			json_append_element(results, result );


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

	json_append_member(root, "results", results );


        tmps = json_stringify(root, "\t");
        puts(tmps);
        free(tmps);


	json_delete(results_info);
	json_delete(root);

}


