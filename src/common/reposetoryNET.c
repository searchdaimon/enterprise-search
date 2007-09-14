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
			//#ifdef DEBUG
                        printf("closing socket to %s\n",lastServer);
			//#endif
                        close(socketha);
               }

                strcpy(lastServer,HostName);

               socketha = cconnect(HostName, BLDPORT);

        }
        else {
		//#ifdef DEBUG
                printf("reusing socket to %s\n",lastServer);
		//#endif
        }

	return socketha;
}

off_t rGetFileSize(char source[], int LotNr,char subname[]) {

	int socketha = conectTo(LotNr);

	#ifdef DEBUG
		printf("rGetFileByOpenHandler\n");
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

int rGetFileByOpenHandler(char source[],FILE *FILEHANDLER,int LotNr,char subname[]) {

	int socketha;

	int n;
	int filnamelen;
	off_t filesize;
	off_t i;
	char c;
	char *filblocbuff;
        off_t fileBloks,filerest;

	//printf("rGetFileByOpenHandler\n");

	//kobler til
	socketha = conectTo(LotNr);

	fseek(FILEHANDLER,SEEK_SET,0);

	//trunkerer filen til 0 bytes
	if (ftruncate( fileno(FILEHANDLER),0) != 0) {
		perror("ftruncate");
	}
	
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
            perror("rGetFileByOpenHandler: Cant recv fileBloks");
            exit(1);
        }

        if ((i=recv(socketha, &filerest, sizeof(filerest),MSG_WAITALL)) == -1) {
            perror("Cant recv filerest");
            exit(1);
        }

	#ifdef DEBUG
        printf("fileBloks: %" PRId64 ", filerest: %" PRId64 "\n",fileBloks,filerest);
	#endif
	
        filblocbuff = (char *)malloc(rNetTrabsferBlok);
	if (fileBloks > 0) {
        	for(i=0;i < fileBloks;i++) {

                	if ((n=recv(socketha, filblocbuff, rNetTrabsferBlok,MSG_WAITALL)) == -1) {
                        	perror("Cant recv dest");
                        	exit(1);
                	}

                	fwrite(filblocbuff,sizeof(c),rNetTrabsferBlok,FILEHANDLER);
        	}

	}

	if (filerest > 0) {
        	if ((n=recv(socketha, filblocbuff, filerest,MSG_WAITALL)) == -1) {
        	                perror("Cant recv filerest");
        	                exit(1);
        	}

        	fwrite(filblocbuff,sizeof(c),filerest,FILEHANDLER);
	}

        free(filblocbuff);
	

	/*
	if ((i=read(socketha, &filesize, sizeof(filesize))) == -1) {
            perror("Cant read filesize");
            exit(1);
        }

	printf("filesize: %" PRId64 "\n",filesize);


	for (i=0;i<filesize;i++) {
		read(socketha, &c, sizeof(char));
		fwrite(&c,sizeof(char),1,FILEHANDLER);

		printf("%i\n",(int)c);
	}
	*/
	#ifdef DEBUG
	printf("file read end\n");
	#endif
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


}


int rSendFile(char source[], char dest[], int LotNr, char opentype[],char subname[]) {

	FILE *FILEHANDLER;

       //opner filen
        if ( (FILEHANDLER = fopen(source,"r")) == NULL ) {
                perror(source);
                return 0;
        }

	
	rSendFileByOpenHandler(FILEHANDLER,dest,LotNr,opentype,subname);

	fclose(FILEHANDLER);
}
int rSendFileByOpenHandler(FILE *FILEHANDLER, char dest[], int LotNr, char opentype[],char subname[]) {


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

	int socketha = conectTo(LotNr);


	//søker til starten
	fseek(FILEHANDLER,0,SEEK_SET);	


	//sender heder
	if (!sendpacked(socketha,C_rSendFile,BLDPROTOCOLVERSION, 0, NULL,subname)) {
		fprintf(stderr,"han't sendpacked\n");
		socketError();
		return 0;
	}

	//sender lotnr
        sendall(socketha,&LotNr, sizeof(LotNr));

	destLen = strlen(dest) +1; // \0
	printf("destlen %i: %s\n",destLen,dest);
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

	printf("fileBloks: %"PRId64", filerest: %"PRId64", size %"PRId64"\n",fileBloks,filerest,inode.st_size);

	filblocbuff = (char *)malloc(rNetTrabsferBlok);
	for (i=0;i<fileBloks;i++) {
                if (n= (fread(filblocbuff,sizeof(char),rNetTrabsferBlok,FILEHANDLER)) != rNetTrabsferBlok) {
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

	//leser filen en byt av gngen, og skriver den til socketen
	//while (!feof(FILEHANDLER)) {

	if (n= (fread(filblocbuff,sizeof(char),filerest,FILEHANDLER)) == -1) {
        	perror("read");
		exit(1);
        }
	if (!sendall(socketha,filblocbuff,filerest)) {
		printf("rSendFileByOpenHandler: can't send filerest\n");
		socketError();
		return 0;
	}

	free(filblocbuff);


}

int rGetNextNET(char *HostName, unsigned int LotNr,struct ReposetoryHeaderFormat *ReposetoryHeader, char htmlbuffer[], char imagebuffebuffer[], unsigned long int *radress, unsigned int FilterTime, unsigned int FileOffset,char subname[]) {


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
                if (i=recv(socketha, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL) == -1) {
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

                        //lseser htmlen
                        if ((i=recv(socketha, htmlbuffer, (*ReposetoryHeader).htmlSize ,MSG_WAITALL))  == -1){
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
                socketha = cconnect(HostName, BLDPORT);
                socketOpen = 1;
        }

	//sender forespørsel
	sendpacked(socketha,C_DIRead,BLDPROTOCOLVERSION, sizeof(DocID), &DocID,subname);


	//leser svar
        if ((i=recv(socketha, DocumentIndexPost, sizeof(struct DocumentIndexFormat),MSG_WAITALL)) == -1) {
              perror("recv");
              exit(1);
        }

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
		exit(1);
	}
	rtext = malloc(textlen+1);
	if ((i = recv(socketha, rtext, textlen, MSG_WAITALL)) == -1) {
		perror("recv(text)");
		exit(1);
	}
	rtext[textlen] = '\0';
	strncpy(text, rtext, len-1);
	text[len-1] = '\0';
	free(rtext);
}


/* XXX: Send and receive compressed data */
void
readHTMLNET(char *subname, unsigned int DocID, char *text, size_t maxlen)
{
	int socketha;
	int i;
	int LotNr;
	size_t len;

	LotNr = rLotForDOCid(DocID);
	socketha = conectTo(LotNr);

	sendpacked(socketha, C_readHTML, BLDPROTOCOLVERSION, sizeof(DocID), &DocID, subname);

	sendall(socketha, &len, sizeof(len));

	if ((i = recv(socketha, &len, sizeof(len), MSG_WAITALL)) == -1) {
		perror("recv(len)");
		exit(1);
	}

	if (len == 0) {
	
	}
	else if (len > maxlen) {
		printf("len (%u)> maxlen (%u)\n",(unsigned int)len,(unsigned int)maxlen);
		exit(1);
	}
	else {
		if ((i = recv(socketha, text, len, MSG_WAITALL)) == -1) {
			printf("readHTMLNET:can't recv. len %i\n",len);
			perror("readHTMLNET:recv(text)");
			exit(1);
		}
	}
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
