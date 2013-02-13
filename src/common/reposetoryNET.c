#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>



#include "define.h"
#include "reposetoryNET.h"
#include "daemon.h"
#include "DocumentIndex.h"
#include "lotlist.h"

#include <stdio.h>
#include <sys/stat.h>


//globals
static char lastServer[32] = "noone";

static int socketha;
//static int socketOpen = 0;

//stenger ned sco hvis vi får en error
int socketError () {

	close(socketha);

        strcpy(lastServer,"noone");

	return 1;
}


int conectTo(int LotNr) {


	char HostName[32]; 


        //printf("sendig lot %i to %s\n",LotNr,HostName);

        lotlistLoad();
        lotlistGetServer(HostName,LotNr);


        if (strcmp(HostName,lastServer) != 0) {

		#ifdef DEBUG
                printf("connecting to %s\n",HostName);
		#endif

               if (strcmp(lastServer,"noone") != 0) {
			#ifdef DEBUG
                        printf("closing socket to %s\n",lastServer);
			#endif
                        close(socketha);
               }

                strcpy(lastServer,HostName);

               socketha = cconnect(HostName, BLDPORT);

        }
        else {
		#ifdef DEBUG
                printf("reusing socket to %s\n",lastServer);
		#endif
        }

	return socketha;
}


int lotHasSufficientSpaceNetToHostname(int LotNr, int needSpace, char subname[], char server[]) {


	int response;
	int sock;
	int i;

	if ((sock = cconnect(server, BLDPORT)) == 0) {
		perror(server);
		return 0;
	}

	//sender heder
        sendpacked(sock,C_getlotHasSufficientSpace,BLDPROTOCOLVERSION, 0, NULL,subname);

	//sender needSpace og lotnr
        sendall(sock,&LotNr, sizeof(LotNr));	
        sendall(sock,&needSpace, sizeof(needSpace));	

        //leser inn svar
        if ((i=recv(sock, &response, sizeof(response),MSG_WAITALL)) == -1) {
                perror("lotHasSufficientSpaceNetToHostname: Cant recv response");
		return 0;
        }

	close(sock);

	return response;

}


int getLotToIndex(char subname[],char HostName[], int dirty) {

	int LotNr;
	int sock;
	int i;

	if ((sock = cconnect(HostName, BLDPORT)) == 0) {
		perror(HostName);
		return 0;
	}

	//sender heder
	printf("sending heder\n");
        sendpacked(sock,C_getLotToIndex,BLDPROTOCOLVERSION, 0, NULL,subname);

	//sender dirty
	printf("sending dirty (%i)\n",dirty);
        sendall(sock,&dirty, sizeof(dirty));	

        //leser inn lotnr
        if ((i=recv(sock, &LotNr, sizeof(LotNr),MSG_WAITALL)) == -1) {
                perror("getLotToIndex: Cant recv lot");
		return 0;
        }

	close(sock);

	return LotNr;

}

off_t rGetFileSize(char source[], int LotNr,char subname[]) {

	int socketha = conectTo(LotNr);

	#ifdef DEBUG
		printf("rGetFileSize\n");
	#endif
	int filnamelen;
	off_t i;
        off_t fileBloks;



	
	//sender heder
        sendpacked(socketha,C_rGetSize,BLDPROTOCOLVERSION, 0, NULL,subname);

	//sender lotnr
        sendall(socketha,&LotNr, sizeof(LotNr));	

	//sender filnavn lengde
	filnamelen = strlen(source) +1; // +1 for \0
	sendall(socketha,&filnamelen, sizeof(filnamelen));

	//sender hvilken fil vi vil ha
	sendall(socketha,source, filnamelen);


        //leser inn filstørelsen
        if ((i=recv(socketha, &fileBloks, sizeof(fileBloks),MSG_WAITALL)) == -1) {
            perror("rGetFileSize: Cant recv fileBloks");
            exit(1);
        }

	return fileBloks;
}
int rGetFileByOpenHandlerFromSocket(char source[],FILE *FILEHANDLER,int LotNr,char subname[],int socketha) {

	int n;
	int filnamelen;
	off_t i;
	char c;
	char *filblocbuff;
        off_t fileBloks,filerest;


	fseek(FILEHANDLER,SEEK_SET,0);

	//trunkerer filen til 0 bytes
	//Runarb: 11 juli 2008: dette skaper problemer med atonmisk writes
	/*
	if (ftruncate( fileno(FILEHANDLER),0) != 0) {
		perror("rGetFileByOpenHandlerFromSocket: ftruncate");
		//return 0;
		exit(1);
	}
	*/

	//sender heder
        sendpacked(socketha,C_rGetFile,BLDPROTOCOLVERSION, 0, NULL,subname);

	//sender lotnr
        sendall(socketha,&LotNr, sizeof(LotNr));	

	//sender filnavn lengde
	filnamelen = strlen(source) +1; // +1 for \0
	sendall(socketha,&filnamelen, sizeof(filnamelen));

	//sender hvilken fil vi vil ha
	sendall(socketha,source, filnamelen);


        //leser inn filstørelsen
        if ((i=recv(socketha, &fileBloks, sizeof(fileBloks),MSG_WAITALL)) == -1) {
            perror("rGetFileByOpenHandler: Can't recv fileBloks");
            return 0;
        }

        if ((i=recv(socketha, &filerest, sizeof(filerest),MSG_WAITALL)) == -1) {
            perror("Cant recv filerest");
            return 0;
        }

	#ifdef DEBUG
        printf("fileBloks: %" PRId64 ", filerest: %" PRId64 "\n",fileBloks,filerest);
	#endif
	
        filblocbuff = (char *)malloc(rNetTrabsferBlok);

	if (fileBloks > 0) {
        	for(i=0;i < fileBloks;i++) {

                	if ((n=recv(socketha, filblocbuff, rNetTrabsferBlok,MSG_WAITALL)) == -1) {
                        	perror("Cant recv dest");
                        	return 0;
                	}

                	if (fwrite(filblocbuff,sizeof(c),rNetTrabsferBlok,FILEHANDLER) == 0) {
				perror("fwrite");
				return 0;
			}
        	}

	}

	if (filerest > 0) {
        	if ((n=recv(socketha, filblocbuff, filerest,MSG_WAITALL)) == -1) {
        	                perror("Cant recv filerest");
        	                return 0;
        	}

        	fwrite(filblocbuff,sizeof(c),filerest,FILEHANDLER);
	}

        free(filblocbuff);
	

	#ifdef DEBUG
	printf("file read end\n");
	#endif


	return 1;
}
int rGetFileByOpenHandlerFromHostName(char source[],FILE *FILEHANDLER,int LotNr,char subname[],char HostName[]) {

	int n;

	if ((socketha = cconnect(HostName, BLDPORT)) == 0) {
		printf("can't connect to host \"%s\"\n",HostName);
		perror(HostName);
		return 0;
	}

	n = rGetFileByOpenHandlerFromSocket(source,FILEHANDLER,LotNr,subname,socketha);

	close(socketha);

	return n;
}
int rGetFileByOpenHandler(char source[],FILE *FILEHANDLER,int LotNr,char subname[]) {

	int socketha;


	//printf("rGetFileByOpenHandler\n");

	//kobler til
	if ((socketha = conectTo(LotNr)) == 0) {
		perror("rGetFileByOpenHandler: can't conect to storage server");
		return 0;
	}

	return rGetFileByOpenHandlerFromSocket(source,FILEHANDLER,LotNr,subname,socketha);
}

int rmkdir(char dest[], int LotNr,char subname[]) {

	int destLen;

	int socketha = conectTo(LotNr);

        //sender heder
        sendpacked(socketha,C_rmkdir,BLDPROTOCOLVERSION, 0, NULL,subname);


        //sender lotnr
        sendall(socketha,&LotNr, sizeof(LotNr));


        destLen = strlen(dest) +1; // \0
        //printf("destlen %i: %s\n",destLen,dest);
        //sender destinasjonsstring lengde
        sendall(socketha,&destLen, sizeof(destLen));

        //sender destinasjon
        sendall(socketha,dest, destLen);


	return 0;
}

int rComand(char dest[], int LotNr,char subname[]) {

	int destLen;

	int socketha = conectTo(LotNr);

        //sender heder
        sendpacked(socketha,C_rComand,BLDPROTOCOLVERSION, 0, NULL,subname);


        //sender lotnr
        sendall(socketha,&LotNr, sizeof(LotNr));


        destLen = strlen(dest) +1; // \0
        //printf("destlen %i: %s\n",destLen,dest);
        //sender destinasjonsstring lengde
        sendall(socketha,&destLen, sizeof(destLen));

        //sender destinasjon
        sendall(socketha,dest, destLen);

	return 1;
}


int rSendFile(char source[], char dest[], int LotNr, char opentype[],char subname[]) {

	FILE *FILEHANDLER;

	#ifdef DEBUG
	printf("rSendFile(source=%s, dest=%s, LotNr=%i, opentype=%s, subname=%s)\n",source,dest,LotNr,opentype,subname);
	#endif

       //opner filen
        if ( (FILEHANDLER = fopen(source,"r")) == NULL ) {
                perror(source);
                return 0;
        }

	
	rSendFileByOpenHandler(FILEHANDLER,dest,LotNr,opentype,subname);

	fclose(FILEHANDLER);

	return 1;
}


int rSendFileByOpenHandlerBySocket(FILE *FILEHANDLER, char dest[], int LotNr, char opentype[],char subname[],int socketha);

int rSendFileToHostname(char source[], char dest[], int LotNr, char opentype[],char subname[], char HostName[]) {

	int socketha;

	printf("rSendFileToHostname(source=%s, dest=%s, opentype=%s, subname=%s, HostName=%s)\n",source,dest,opentype,subname,HostName);

	if ((socketha = cconnect(HostName, BLDPORT)) == 0) {
		printf("can't connect to host \"%s\"\n",HostName);
		perror(HostName);
		return 0;
	}

	FILE *FILEHANDLER;

       //opner filen
        if ( (FILEHANDLER = fopen(source,"r")) == NULL ) {
                perror(source);
                return 0;
        }

	
	if (!rSendFileByOpenHandlerBySocket(FILEHANDLER,dest,LotNr,opentype,subname,socketha)) {
		return 0;
	}

	fclose(FILEHANDLER);

	close(socketha);

	printf("~rSendFileToHostname\n");

	return 1;
}

int rSendFileByOpenHandler(FILE *FILEHANDLER, char dest[], int LotNr, char opentype[],char subname[]) {

	int socketha = conectTo(LotNr);

	return rSendFileByOpenHandlerBySocket(FILEHANDLER, dest, LotNr, opentype, subname, socketha);
}


int rSendFileByOpenHandlerBySocket(FILE *FILEHANDLER, char dest[], int LotNr, char opentype[],char subname[],int socketha) {

	int n,i;
	struct stat inode;      // lager en struktur for fstat å returnere.
	int destLen;
	off_t fileBloks,filerest;
	char *filblocbuff;

	//printf("sendig lot %i to %s\n",LotNr,HostName);


	if (strlen(opentype) != 1) {
		printf("Error: rSendFileByOpenHandler: opentype can only be one caracter, and \"w\" or \"a\", not \"%s\"\n.",opentype);
		exit(1);
	}
	else if (opentype[0] != 'w' && opentype[0] != 'a') {
		printf("Error: rSendFileByOpenHandler: opentype can only be \"w\" or \"a\", not \"%s\"\n.",opentype);
		exit(1);
	}



	//søker til starten
	fseek(FILEHANDLER,0,SEEK_SET);	


	//sender heder
	if (!sendpacked(socketha,C_rSendFile,BLDPROTOCOLVERSION, 0, NULL,subname)) {
		fprintf(stderr,"Can't sendpacked() on header\n");
		perror("sendpacked");
		socketError();
		return 0;
	}

	//sender lotnr
        if (!sendall(socketha,&LotNr, sizeof(LotNr))) {
		printf("can't send lotnr\n");
		return 0;
	}

	destLen = strlen(dest) +1; // \0
	//printf("destlen %i: %s\n",destLen,dest);
	//sender destinasjonsstring lengde
	sendall(socketha,&destLen, sizeof(destLen));

	//sender destinasjon
	sendall(socketha,dest, destLen);

        //sender fil opningstype, write, apend. Sender også \0
        sendall(socketha,opentype, sizeof(char) +1);

	//finner hvor stor filen er
	fstat(fileno(FILEHANDLER),&inode);
	

	fileBloks = (int)floor(inode.st_size / rNetTrabsferBlok);

	filerest = inode.st_size - (fileBloks * rNetTrabsferBlok);

	sendall(socketha,&fileBloks, sizeof(fileBloks));
	sendall(socketha,&filerest, sizeof(filerest));

	#ifdef DEBUG
	printf("fileBloks: %"PRId64", filerest: %"PRId64", size %"PRId64"\n",fileBloks,filerest,inode.st_size);
	#endif

	filblocbuff = (char *)malloc(rNetTrabsferBlok);
	for (i=0;i<fileBloks;i++) {
                if ((n = (fread(filblocbuff,sizeof(char),rNetTrabsferBlok,FILEHANDLER))) != rNetTrabsferBlok) {
                        perror("read");
			printf("Is the file open for bout read and write. Not only write?. Did read %i of %i\n",n,rNetTrabsferBlok);

			return 0;
                }
		
                if (!sendall(socketha,filblocbuff,rNetTrabsferBlok)) {
			printf("rSendFileByOpenHandler: can't sendall\n");
			socketError();
			return 0;
		}
                //send(socketha,filblocbuff,rNetTrabsferBlok,0);
        }


	if (filerest != 0) {

		if ((n = (fread(filblocbuff,sizeof(char),filerest,FILEHANDLER))) == -1) {
        		perror("read");
			exit(1);
        	}
		if (!sendall(socketha,filblocbuff,filerest)) {
			printf("rSendFileByOpenHandler: can't send filerest\n");
			socketError();
			return 0;
		}

	}

	free(filblocbuff);


	return 1;
}

int rGetNextNET(char *HostName, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffebuffer[] __bunused, unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset __bunused,char subname[]) {


	static int socketha;
        static int socketOpen = 0;

        int i;
        struct packedHedderFormat packedHedder;
        int DataInLot = 1;


        if (socketOpen != 1) {
                printf("connecting\n");
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }


                //printf("sending kommand\n");
                sendpacked(socketha,C_rGetNext,BLDPROTOCOLVERSION, sizeof(LotNr) + sizeof(FilterTime), NULL,subname);

		//sender Lotnr
		sendall(socketha,&LotNr, sizeof(LotNr));
		//sender filtertid
		sendall(socketha,&FilterTime, sizeof(FilterTime));


		//printf("gting response\n");
                //får svar tilbake
                //leser hedder
                if ((i=recv(socketha, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) == -1) {
                        perror("recv");
                        exit(1);
                }

                //printf("got comand: %i\n",packedHedder.command);

                if (packedHedder.command == C_rLotData) {

                        if ((i=recv(socketha, ReposetoryHeader, sizeof(struct ReposetoryHeaderFormat),MSG_WAITALL)) == -1) {
                            perror("recv");
                            exit(1);
                        }

                        //printf("url %s\n", (*ReposetoryHeader).url);
        		if ((*ReposetoryHeader).htmlSize != 0) {
       			         (*ReposetoryHeader).htmlSize2 = (*ReposetoryHeader).htmlSize;
		        }


                        //lseser htmlen
                        if ((i=recv(socketha, htmlbuffer, (*ReposetoryHeader).htmlSize2 ,MSG_WAITALL))  == -1){
                           perror("recv html");
                            exit(1);
                        }
                        //lsere adressen
                        if ((i=recv(socketha, radress, sizeof(radress),MSG_WAITALL)) == -1) {
                            perror("recv");
                            exit(1);
                        }

                        return 1;
                }
                else if (packedHedder.command == C_rEOF) {
                        printf("eof\n");
                        DataInLot = 0;

			//tror man må stenge ned socketen her slik at serveren gir seg
			//oppstår det problemer prøv å hak fram linjene under
                        //close(socketha);
			//socketOpen := 0;

                        return 0;
                }
                else {
                        printf("command not understod. Did get %i\n",packedHedder.command);
                        return 0;
                }

}

unsigned long int DIGetIp (char *HostName, unsigned int DocID,char subname[]) {

	unsigned long int IPAddress;
	int i;

	static int socketha;
        static int socketOpen = 0;

	
	if (socketOpen != 1) {
                printf("connecting\n");
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }
	
	sendpacked(socketha,C_DIGetIp,BLDPROTOCOLVERSION, sizeof(DocID), &DocID,subname);

	//leser svar
        if ((i=recv(socketha, &IPAddress, sizeof(IPAddress),MSG_WAITALL)) == -1) {
              perror("recv");
              exit(1);
        }
	//printf("ipaa: %u\n",IPAddress);

	return IPAddress;

}

void DIWriteNET (char *HostName, struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]) {

	static int socketha;
        static int socketOpen = 0;


	if (socketOpen != 1) {
                printf("connecting\n");
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }

	sendpacked(socketha,C_DIWrite,BLDPROTOCOLVERSION, sizeof(struct DocumentIndexFormat) + sizeof(DocID), NULL,subname);

	//printf("size of packet: %i\n",sizeof(struct DocumentIndexFormat) + sizeof(DocID));
	
	sendall(socketha,DocumentIndexPost, sizeof(struct DocumentIndexFormat));

	sendall(socketha,&DocID, sizeof(DocID));
}

int DIReadNET (char *HostName, struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]){

	int i;

	static int socketha;
        static int socketOpen = 0;
	

	if (socketOpen != 1) {
                if ((socketha = cconnect(HostName, BLDPORT)) == 0) {
			perror(HostName);
			return 0;
		}
                socketOpen = 1;
        }

	//sender forespørsel
	sendpacked(socketha,C_DIRead,BLDPROTOCOLVERSION, sizeof(DocID), &DocID,subname);


	//leser svar
        if ((i=recv(socketha, DocumentIndexPost, sizeof(struct DocumentIndexFormat),MSG_WAITALL)) == -1) {
              perror("recv");
		return 0;
              //exit(1);
        }

	return 1;

}

int DIReadNET2 ( struct DocumentIndexFormat *DocumentIndexPost, int DocID,char subname[]){

	int i;

	int socketha = conectTo(rLotForDOCid(DocID));

	//sender forespørsel
	sendpacked(socketha,C_DIRead,BLDPROTOCOLVERSION, sizeof(DocID), &DocID,subname);


	//leser svar
        if ((i=recv(socketha, DocumentIndexPost, sizeof(struct DocumentIndexFormat),MSG_WAITALL)) == -1) {
              perror("recv");
              exit(1);
        }

	return 1;
}


void
anchoraddnewNET(char *hostname, unsigned int DocID, char *text, size_t textsize, char *subname)
{
	static int socketha;
        static int socketOpen;

	if (socketOpen != 1) {
                printf("connecting\n");
                socketha = cconnect(hostname, BLDPORT);
                socketOpen = 1;
        }

	sendpacked(socketha,C_anchorAdd, BLDPROTOCOLVERSION, sizeof(DocID), &DocID, subname);
	sendall(socketha, &textsize, sizeof(textsize));
	sendall(socketha, text, textsize);
}


void
anchorReadNET(char *hostname, char *subname, unsigned int DocID, char *text, int len)
{
	static int socketha;
        static int socketOpen;
	size_t textlen;
	char *rtext;
	int i;

	if (socketOpen != 1) {
                printf("connecting\n");
                socketha = cconnect(hostname, BLDPORT);
                socketOpen = 1;
        }

	sendpacked(socketha,C_anchorGet, BLDPROTOCOLVERSION, sizeof(DocID), &DocID, subname);

	if ((i = recv(socketha, &textlen, sizeof(textlen),MSG_WAITALL)) == -1) {
		perror("recv(textlen)");
		return;
	}
	rtext = malloc(textlen+1);
	if (rtext == NULL) {
		perror("malloc(rtext)");
		text[0] = '\0';
		return;
	}
	if ((i = recv(socketha, rtext, textlen, MSG_WAITALL)) == -1) {
		perror("recv(text)");
		free(rtext);
		text[0] = '\0';
		free(rtext);
		return;
	}
	rtext[textlen] = '\0';
	strncpy(text, rtext, len-1);
	text[len-1] = '\0';
	free(rtext);
}


/* XXX: Send and receive compressed data */

int
readHTMLNET_tosoc(char *subname, unsigned int DocID, char *text, unsigned int len,struct ReposetoryHeaderFormat *ReposetoryHeader, int socketha)
{
	int i;
	int LotNr;

	LotNr = rLotForDOCid(DocID);

	sendpacked(socketha, C_readHTML, BLDPROTOCOLVERSION, sizeof(DocID), &DocID, subname);

	sendall(socketha, &len, sizeof(len));

	if ((i = recv(socketha, &len, sizeof(len), MSG_WAITALL)) == -1) {
		perror("recv(len)");
		exit(1);
	}
	if (len == 0) {
		text[0] = '\0';
	} else {
		if ((i = recv(socketha, text, len, MSG_WAITALL)) == -1) {
			printf("%p %d\n", text, len);
			perror("recv(text)");
			text[0] = '\0';
			return 0;
		}
		if ((i = recv(socketha, ReposetoryHeader, sizeof(struct ReposetoryHeaderFormat), MSG_WAITALL)) == -1) {
			printf("%p %d\n", text, len);
			perror("recv(text)");
			text[0] = '\0';
			return 0;
		}
	}

	return len;
}

int
readHTMLNET_toHost(char *subname, unsigned int DocID, char *text, unsigned int len,struct ReposetoryHeaderFormat *ReposetoryHeader, char HostName[])
{

	int ret;

	socketha = cconnect(HostName, BLDPORT);

	if (socketha == 0) {
		text[0] = '\0';
		return 0;
	}

	ret =  readHTMLNET_tosoc(subname,DocID,text,len,ReposetoryHeader,socketha);

	close(socketha);


	return ret;
}
int
readHTMLNET(char *subname, unsigned int DocID, char *text, unsigned int len,struct ReposetoryHeaderFormat *ReposetoryHeader)
{

	int LotNr = rLotForDOCid(DocID);

	socketha = conectTo(LotNr);

	if (socketha == -1) {
		text[0] = '\0';
		return 0;
	}

	return readHTMLNET_tosoc(subname,DocID,text,len,ReposetoryHeader, socketha);
}

unsigned int GetLastIndexTimeForLotNET(char *HostName, int LotNr,char subname[]){

        int i;
	unsigned int IndexTime;

	static int socketha;
        static int socketOpen = 0;


        if (socketOpen != 1) {
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }

        //sender forespørsel
        sendpacked(socketha,C_rGetIndexTime,BLDPROTOCOLVERSION, sizeof(LotNr), &LotNr,subname);


        //leser svar
        if ((i=recv(socketha, &IndexTime, sizeof(IndexTime),MSG_WAITALL)) == -1) {
              perror("recv");
              exit(1);
        }

	return IndexTime;

}

void setLastIndexTimeForLotNET(char *HostName, int LotNr,char subname[]){


	static int socketha;
        static int socketOpen = 0;


        if (socketOpen != 1) {
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }

        //sender forespørsel
        sendpacked(socketha,C_rSetIndexTime,BLDPROTOCOLVERSION, sizeof(LotNr), &LotNr,subname);

}

int
openUrlTODocIDNET(char *hostname)
{
	int sock;
	char cmd;

	sock = cconnect(hostname, BLDPORT);

        //sender forespørsel
        sendpacked(sock, C_urltodocid, BLDPROTOCOLVERSION, 0, NULL, "www");

	recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
	if (cmd == C_DOCID_NODB) {
		fprintf(stderr, "No db is set for that lot daemon\n");
		return -1;
	}

	return sock;
}

int
getUrlTODOcIDNET(int sock, char *url, unsigned int *DocID)
{
	char cmd;
	size_t len;

	cmd = C_DOCID_NEXT;
	sendall(sock, &cmd, sizeof(cmd));
	len = strlen(url);
	sendall(sock, &len, sizeof(len));
	sendall(sock, url, len);
	recv(sock, &cmd, sizeof(cmd), MSG_WAITALL);
	if (cmd == C_DOCID_NOTFOUND) {
		return 0;
	} else {
		recv(sock, DocID, sizeof(*DocID), MSG_WAITALL);
		return 1;
	}
}

void
closeUrlTODocIDNET(int sock)
{
	char cmd;

	cmd = C_DOCID_DONE;
	sendall(sock, &cmd, sizeof(cmd));
	close(sock);
}


void closeNET () {

	static int socketha;
        static int socketOpen = 0;


	close(socketha);
	socketOpen = 0;
}
