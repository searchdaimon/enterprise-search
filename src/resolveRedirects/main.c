#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resolveRedirects.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"
#include "../common/reposetoryNET.h"

#include "../UrlToDocID/search_index.h"

//#define URLTODOCIDDB "/mnt/node1/hda4/UrlToDocID/"
//#define URLTODOCIDDB "/home/boitho/UrlToDocID/"
#define NEWURLS "/tmp/new.urls.txt"

int resolveabsoluteURLRedirect(char url[], char redirectTo[], int redirectToBufSize) {

	char domain[512];
	char newurl[512];

	if (!find_domain (url,domain,sizeof(domain))) {
		return 0;
	}

	sprintf(newurl,"http://%s%s",domain,redirectTo);

	#ifdef DEBUG
	printf("resolveabsoluteURLRedirect: url: \"%s\", redirectTo: \"%s\" -> \"%s\"\n",url,redirectTo,newurl);
	#endif

	strscpy(redirectTo,newurl,redirectToBufSize);
}

int
main (int argc, char **argv)
{
	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	char HtmlBuffer[50000];
	char *subname;
	unsigned int HtmlBufferLen;
	struct ReposetoryHeaderFormat ReposetoryHeader;
	FILE *new, *mapping;
	char filename[1024];
	//int ldsock;
	int stat_badurl = 0;
	int stat_unknownDocID = 0;
	int stat_haveDocID = 0;
	int stat_cantUrlnormaliz = 0;

	char *acl_allowbuffer, *acl_deniedbuffer;
	unsigned int DocID, redirDocID;

	if (argc < 5) {
		printf("Dette programet identifiserer 301 og 302 redirects i DocumentIndex. Gi det et lot nr. \n\n"
		       "\tUsage: ./readDocumentIndex 1 www newurlsfile UrlToDocID\n");
		exit(0);
	}

	LotNr = atoi(argv[1]);
	subname = argv[2];
	char *newurlsfile = argv[3];
	char *UrlToDocIDdb = argv[4];

	DocID = 0;

	GetFilPathForLot(filename, LotNr, "www");
	strcat(filename, "redirmap");
	if ((mapping = fopen(filename, "w")) == NULL) {
		//err(1, "fopen(newurls)");
		err(1, filename);
	}

	if ((new = fopen(newurlsfile, "a")) == NULL)
		err(1, "fopen(newurls)");

        char            db_index[strlen(UrlToDocIDdb)+7];
        sprintf(db_index, "%s.index", UrlToDocIDdb);
        urldocid_data   *data = urldocid_search_init(db_index, UrlToDocIDdb);

	while (DIGetNext(&DocumentIndexPost, LotNr, &DocID, subname)) {
		if ((DocumentIndexPost.response == 301) || (DocumentIndexPost.response == 302)) {
			HtmlBufferLen = sizeof(HtmlBuffer);
			if (rReadHtml(HtmlBuffer, &HtmlBufferLen, DocumentIndexPost.RepositoryPointer, DocumentIndexPost.htmlSize,
			              DocID, subname, &ReposetoryHeader, &acl_allowbuffer, &acl_deniedbuffer, DocumentIndexPost.imageSize) == 0)
				continue;

			if (HtmlBufferLen < 200) {

				if (HtmlBuffer[0] == '/') {
					//printf("non http url \"%s\" -> \"%s\"\n",DocumentIndexPost.Url ,HtmlBuffer);
					resolveabsoluteURLRedirect(DocumentIndexPost.Url ,HtmlBuffer,sizeof(HtmlBuffer));
					//exit(1);
				}

				if (strncmp("http://", HtmlBuffer, 7) != 0) {
					//fprintf(stderr, "Invalid url: %s\n", HtmlBuffer);
					++stat_badurl;

				} else if (!url_normalization(HtmlBuffer,sizeof(HtmlBuffer))) {
					++stat_cantUrlnormaliz;

				//} else if (getUrlTODOcIDNET(ldsock, HtmlBuffer, &redirDocID)) {
				} else if (getDocIDFromUrl(data, HtmlBuffer, &redirDocID)) {
					struct redirects redir;
					#ifdef DEBUG
					printf("%u => %u (reason: %hu)\n", DocID, redirDocID, DocumentIndexPost.response);
					#endif
					redir.DocID = DocID;
					redir.redirectTo = redirDocID;
					redir.response = DocumentIndexPost.response;

					fwrite(&redir, sizeof(redir), 1, mapping);
					//printf("DocID: %u, url: \"%s\", redirects of type %hu to \"%s\"\n",
					//       DocID, DocumentIndexPost.Url, DocumentIndexPost.response, HtmlBuffer);
					++stat_haveDocID;
				} else {
					fprintf(new, "%s\n", HtmlBuffer);
					++stat_unknownDocID;
				}
			}
		}
	}

	fclose(new);
	fclose(mapping);
	//closeUrlTODocIDNET(ldsock);
	urldocid_search_exit(data);
	//DIClose();


	printf("badurl\t\t\t%-10i\nunknown DocID\t\t%-10i\nhave DocID\t\t%-10i\ncan't Urlnormaliz\t%-10i\n",
		stat_badurl,
		stat_unknownDocID,
		stat_haveDocID,
		stat_cantUrlnormaliz
	);

	return 0;
}

