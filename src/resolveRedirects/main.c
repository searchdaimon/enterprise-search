#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resolveRedirects.h"

#include "../common/define.h"
#include "../common/DocumentIndex.h"
#include "../common/reposetory.h"
#include "../common/reposetoryNET.h"

#include "../getDocIDFromUrl/getDocIDFromUrl.h"


//#define URLTODOCIDDB "/mnt/node1/hda4/UrlToDocID/"
#define URLTODOCIDDB "/home/boitho/UrlToDocID/"
#define NEWURLS "/tmp/new.urls.txt"

char *urltodociddb;

int
resolveDocIDfromUrl(char *url, unsigned int *docid)
{
	int ret;
	unsigned int id;

	if ((ret = getDocIDFromUrl(urltodociddb, url, &id)))
		*docid = id;

	return ret;
}


int
main (int argc, char **argv)
{
	struct DocumentIndexFormat DocumentIndexPost;
	int LotNr;
	char HtmlBuffer[5000];
	char *subname;
	unsigned int HtmlBufferLen;
	struct ReposetoryHeaderFormat ReposetoryHeader;
	FILE *new, *mapping;
	char filename[1024];
	int ldsock;

	char *acl_allowbuffer, *acl_deniedbuffer;
	unsigned int DocID, redirDocID;

	if (argc < 6) {
		printf("Dette programet identifiserer 301 og 302 redirects i DocumentIndex. Gi det et lot nr. \n\n"
		       "\tUsage: ./readDocumentIndex 1 www newurlsfile urltodociddb boitholdhostname\n");
		exit(0);
	}

	LotNr = atoi(argv[1]);
	subname = argv[2];
	DocID = 0;
	urltodociddb = argv[4];

	ldsock = openUrlTODocIDNET(argv[5]);
	if (ldsock == -1)
		err(1, "openUrlTODocIDNET()");

	GetFilPathForLot(filename, LotNr, "www");
	strcat(filename, "redirmap");
	if ((mapping = fopen(filename, "w")) == NULL)
		err(1, "fopen(newurls)");

	if ((new = fopen(argv[3], "w")) == NULL)
		err(1, "fopen(newurls)");

	while (DIGetNext(&DocumentIndexPost, LotNr, &DocID, subname)) {
		if ((DocumentIndexPost.response == 301) || (DocumentIndexPost.response == 302)) {
			HtmlBufferLen = sizeof(HtmlBuffer);
			if (rReadHtml(HtmlBuffer, &HtmlBufferLen, DocumentIndexPost.RepositoryPointer, DocumentIndexPost.htmlSize,
			              DocID, subname, &ReposetoryHeader, &acl_allowbuffer, &acl_deniedbuffer) == 0)
				continue;

			if (HtmlBufferLen < 200) {
				if (strncmp("http://", HtmlBuffer, 7) != 0) {
					//fprintf(stderr, "Invalid url: %s\n", HtmlBuffer);
				} else if (getUrlTODOcIDNET(ldsock, HtmlBuffer, &redirDocID)) {
					struct redirects redir;
					printf("%u => %u (reason: %hu)\n", DocID, redirDocID, DocumentIndexPost.response);
					redir.DocID = DocID;
					redir.redirectTo = redirDocID;
					redir.response = DocumentIndexPost.response;

					fwrite(&redir, sizeof(redir), 1, mapping);
					//printf("DocID: %u, url: \"%s\", redirects of type %hu to \"%s\"\n",
					//       DocID, DocumentIndexPost.Url, DocumentIndexPost.response, HtmlBuffer);
				} else {
					fprintf(new, "%s\n", HtmlBuffer);
				}
			}
		}
	}

	fclose(new);
	fclose(mapping);
	closeUrlTODocIDNET(ldsock);

	//DIClose();

	return 0;
}

