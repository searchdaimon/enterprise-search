/*
    ** client.c -- a stream socket client demo
    */
    #include "../common/define.h"
    #include <arpa/inet.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include "../common/cgi-util.h"

    #define PORT 6500 // the port client will be connecting to 

    #define MAXDATASIZE 100 // max number of bytes we can get at once 

    int main(int argc, char *argv[])
    {
        int sockfd, n;
	int i,y;
	char *strpointer;  
	int res;
        char buf[MAXDATASIZE];
        struct hostent *he;
        struct sockaddr_in their_addr; // connector's address information 
	FILE *LOGFILE;
	//char hostName[] = "localhost";
        //char hostName[] = "127.0.0.1";
	char hostName[] = "bbs-001.boitho.com";
	struct SiderFormat Sider[MaxsHits];
        struct SiderHederFormat SiderHeder;
	char buff[64]; //generell buffer
	struct in_addr ipaddr;
        struct QueryDataForamt QueryData;


        //send out an HTTP header:
        printf("Content-type: text/xml\n\n");



        //hvis vi har argumeneter er det første et query
        if (getenv("QUERY_STRING") == NULL) {
                if (argc < 2 ) {
                        printf("Error ingen query spesifisert.\n\nEksempel på bruk for å søke på boitho:\n\tsearchkernel boitho\n\n\n");
                }
                else {
                        QueryData.query[0] = '\0';
                        for(i=1;i<argc ;i++) {
                                sprintf(QueryData.query,"%s %s",QueryData.query,argv[i]);
                        }
                        //strcpy(QueryData.query,argv[1]);
                        //printf("argc :%i %s %s\n",argc,argv[1],argv[2]);
                        printf("query %s\n",QueryData.query);
                }
        }
        else {
		// Initialize the CGI lib
        	res = cgi_init();

		// Was there an error initializing the CGI???
	        if (res != CGIERR_NONE) {
        	        printf("Error # %d: %s<p>\n", res, cgi_strerror(res));
        	        exit(0);
        	}
		
		if (cgi_getentrystr("query") == NULL) {
                	perror("Did'n receive any query.");
        	}
		else {
        	        strncat(QueryData.query,cgi_getentrystr("query"),sizeof(QueryData.query));
	        }

        }

        if (strlen(QueryData.query) > MaxQueryLen -1) {
                printf("query to long\n");
                exit(1);
        }

        //gjør om til liten case
        for(i=0;i<strlen(QueryData.query);i++) {
                QueryData.query[i] = tolower(QueryData.query[i]);
        }


        if ((he=gethostbyname(hostName)) == NULL) {  // get the host info 
            perror("gethostbyname");
            exit(1);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        their_addr.sin_family = AF_INET;    // host byte order 
        their_addr.sin_port = htons(PORT);  // short, network byte order 
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

        if (connect(sockfd, (struct sockaddr *)&their_addr,
                                              sizeof(struct sockaddr)) == -1) {
            perror("connect");
            exit(1);
        }
	struct queryNodeHederFormat queryNodeHeder;

	//kopierer inn query	
	strncpy(queryNodeHeder.query,QueryData.query,sizeof(queryNodeHeder.query) -1);
	
	//sender forespørsel
	sendall(sockfd,queryNodeHeder.query,sizeof(queryNodeHeder));

	//motter hedderen for svaret
	if ((i=recv(sockfd, &SiderHeder, sizeof(SiderHeder),MSG_WAITALL)) == -1) {
                perror("recv");
        }

	//printf("TotaltTreff %i,showabal %i,filtered %i,total_usecs %f\n",SiderHeder.TotaltTreff,SiderHeder.showabal,SiderHeder.filtered,SiderHeder.total_usecs);

	for(i=0;i<SiderHeder.showabal;i++) {

		if ((n=recv(sockfd, &Sider[i], sizeof(struct SiderFormat),MSG_WAITALL)) == -1) {
	               	perror("recv");
        	}
		//printf("url: %s\n",Sider[i].DocumentIndex.Url);
	}


        close(sockfd);


        y=0;
        //fjerner tegn som er eskapet med \, eks \" blir til &quot;
        for(i=0;i<strlen(QueryData.query);i++) {
                if (QueryData.query[i] == '\\') {

                        switch(QueryData.query[++i]) {
                                case '"':
                                        //&quot;
                                        buff[y++] = '&'; buff[y++] = 'q'; buff[y++] = 'u'; buff[y++] = 'o'; buff[y++] = 't'; buff[y++] = ';';
                                break;


                        }
                        //else {
                        //      printf("error: found \\ but no case\n");
                        //}
                }
                else {
                        //printf("%c\n",QueryData.query[i]);
                        buff[y++] = QueryData.query[i];
                }
        }
        strncpy(QueryData.query,buff,sizeof(QueryData.query) -1);

        printf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?> \n");
        printf("<!DOCTYPE family SYSTEM \"http://www.boitho.com/xml/search.dtd\"> \n");

        printf("<search>\n");   


        printf("<treff-info totalt=\"%i\" query=\"%s\" hilite=\"%s\" tid=\"%f\" filtered=\"%i\" showabal=\"%i\"/>\n",SiderHeder.TotaltTreff,QueryData.query,SiderHeder.hiliteQuery,SiderHeder.total_usecs,SiderHeder.filtered,SiderHeder.showabal);


	for(i=0;i<SiderHeder.showabal;i++) {

		if (!Sider[i].deletet) {

                //filtrerer ut tegn som ikke er lov i xml
                while ((strpointer = strchr(Sider[i].DocumentIndex.Url,'&')) != NULL) {
                        (*strpointer) = 'a';
                }
                //while ((strpointer = strchr(Sider[i].title,'&')) != NULL) {
                //        (*strpointer) = 'a';
                //}
                //while ((strpointer = strchr(Sider[i].description,'&')) != NULL) {
                //        (*strpointer) = 'a';
                //}


                printf("<treff>\n");

                printf("\t<DocID>%i-%i</DocID>\n",Sider[i].iindex.DocID,rLotForDOCid(Sider[i].iindex.DocID));
                printf("\t<POSISJON>%i</POSISJON>\n",i +1);

                //DocumentIndex
                printf("\t<Url>%s</Url>\n",Sider[i].DocumentIndex.Url);
                printf("\t<Title>%s</Title>\n",Sider[i].title);
                printf("\t<AdultWeight>%hu</AdultWeight>\n",Sider[i].DocumentIndex.AdultWeight);
                printf("\t<Sprok>%s</Sprok>\n",Sider[i].DocumentIndex.Sprok);
                //temp: blir rare tegn her              
		printf("\t<Dokumenttype>%s</Dokumenttype>\n",Sider[i].DocumentIndex.Dokumenttype);

                printf("\t<RepositorySize>%u</RepositorySize>\n",Sider[i].DocumentIndex.htmlSize);

                printf("\t<THUMBNALE>%s</THUMBNALE>\n",Sider[i].thumbnale);

                printf("\t<CACHE>%s</CACHE>\n",Sider[i].cacheLink);
                printf("\t<IMAGEWIDTH>100</IMAGEWIDTH>\n");
                printf("\t<IMAGEHEIGHT>100</IMAGEHEIGHT>\n");

                printf("\t<METADESCRIPTION></METADESCRIPTION>\n");
                printf("\t<CATEGORY></CATEGORY>\n");
                printf("\t<OFFENSIVE_CODE>FALSE</OFFENSIVE_CODE>\n");


                printf("\t<beskrivelse>%s</beskrivelse>\n",Sider[i].description);
                printf("\t<TermRank>%i</TermRank>\n",Sider[i].iindex.TermRank);
                printf("\t<PopRank>%i</PopRank>\n",Sider[i].iindex.PopRank);
                printf("\t<allrank>%i</allrank>\n",Sider[i].iindex.allrank);

		ipaddr.s_addr = Sider[i].DocumentIndex.IPAddress;

                printf("\t<IPAddress>%s</IPAddress>\n",inet_ntoa(ipaddr));

                printf("\t<RESPONSE>%hu</RESPONSE>\n",Sider[i].DocumentIndex.response);

                printf("\t<NrOfHits>%i</NrOfHits>\n",Sider[i].iindex.TermAntall);

                //printer ut hits (hvor i dokumenetet orde befinner seg ).
                printf("\t<hits>");
                for (y=0; (y < Sider[i].iindex.TermAntall) && (y < MaxTermHit); y++) {
                        printf("%hu ",Sider[i].iindex.hits[y]);
                }
                printf("</hits>\n");

                printf("</treff>\n");

		}
	}

	printf("</search>\n");

	//ToDo: må ha låsing her
	if ((LOGFILE = fopen("/home/boitho/config/query.log","a")) == NULL) {
		perror("logfile");
	}
        fprintf(LOGFILE,"%s %i\n",queryNodeHeder.query,SiderHeder.TotaltTreff);
        fclose(LOGFILE);


        return 0;
    } 

