#include <stdio.h>

#include "out_opensearch.h"


void disp_out_opensearch(int total_res, struct SiderFormat *results, struct queryNodeHederFormat *queryNodeHeder, int num_servers, int start, int res_per_page, char *query_escaped) {
	int i, x;
        
	printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
	printf("<rss version=\"2.0\" xmlns:opensearch=\"http://a9.com/-/spec/opensearch/1.1/\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
	printf("  <channel>\n");
	printf("    <title>%s - Searchdaimon results</title>\n", query_escaped);
	printf("    <description>Results for \"%s\".</description>\n", query_escaped);
	printf("    <opensearch:totalResults>%i</opensearch:totalResults>\n", total_res);
	printf("    <opensearch:startIndex>%i</opensearch:startIndex>\n", start);
	printf("    <opensearch:itemsPerPage>%i</opensearch:itemsPerPage>\n", res_per_page);
	printf("    <atom:link rel=\"search\" type=\"application/opensearchdescription+xml\" href=\"http://%s/webclient/opensearchdescription.xml\"/>\n",
			getenv("HTTP_HOST"));

	i = res_per_page * (start -1);
	x = i;

	while ((x<(res_per_page * start)) && ( x < total_res) && (i < (queryNodeHeder->MaxsHits * num_servers))) {

		if (!results[i].deletet) {

			printf("<item>\n");
			printf("\t<docid>%i-%i</docid>\n",results[i].iindex.DocID, rLotForDOCid(results[i].iindex.DocID));
			printf("\t<title><![CDATA[%s]]></title>\n", results[i].title);
			printf("\t<link><![CDATA[%s]]></link>\n", results[i].url);
			printf("\t<description>%s</description>\n", results[i].description);
			printf("</item>\n");

			//teller bare normale sider
			if (results[i].type == siderType_normal) {
				++x;
			}
		}
		++i;
	}

	printf("  </channel>\n</rss>\n");
}

