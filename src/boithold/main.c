#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>


#include "../common/define.h"
#include "../common/daemon.h"
#include "../common/DocumentIndex.h"

#include "getpath.h"

#define PROTOCOLVERSION 1

void connectHandler(int socket);



int main (void) {

	sconnect(connectHandler, BLDPORT);

	printf("conek ferdig \n");

	exit(0);
}

void connectHandler(int socket) {
        struct packedHedderFormat packedHedder;

	int i,n;
	int LotNr;
        char lotPath[512];
	char buf[100];
	unsigned int FilterTime;
	int filnamelen;
	FILE *FH;
	struct stat inode;      // lager en struktur for fstat å returnere.
	off_t filesize;
	char c;

	struct DocumentIndexFormat DocumentIndexPost;
	int DocID;

        struct ReposetoryHeaderFormat ReposetoryHeader;
        unsigned long int radress;

        char htmlbuffer[524288];
	int destLeng;
        char dest[512];

	off_t fileBloks,filerest;
	char *filblocbuff;


//while ((i=read(socket, &packedHedder, sizeof(struct packedHedderFormat))) > 0) {
while ((i=recv(socket, &packedHedder, sizeof(struct packedHedderFormat),MSG_WAITALL)) > 0) {

//printf("command: %i\n",packedHedder.command);
//printf("i er %i\n",i);
printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
//printf("subname: %s\n",packedHedder.subname);
//lar size reflektere hva som er igjen av pakken
packedHedder.size = packedHedder.size - sizeof(packedHedder);

if (packedHedder.command == C_rmkdir) {

	//leser data. Det skal væren en int som sier hvilken lot vi vil ha
        if ((i=recv(socket, &LotNr, sizeof(LotNr),MSG_WAITALL)) == -1) {
            perror("Cant read lotnr");
            exit(1);
        }

        //leser destinasjonelengden
        if ((i=recv(socket, &destLeng, sizeof(destLeng),MSG_WAITALL)) == -1) {
            perror("Cant read destLeng");
            exit(1);
        }

        if (destLeng > sizeof(dest)) {
                printf("dest filname is to long at %i\n",destLeng);
                exit(1);
        }

        //leser destinasjonene
        if ((i=recv(socket, &dest, destLeng,MSG_WAITALL)) == -1) {
            perror("Cant read dest");
            exit(1);
        }

	GetFilPathForLot(lotPath,LotNr,packedHedder.subname);

	sprintf(lotPath,"%s%s",lotPath,dest);

	printf("mkdir %s\n",lotPath);

	makePath(lotPath);
        
}
else if (packedHedder.command == C_rComand) {


	//leser data. Det skal væren en int som sier hvilken lot vi vil ha
        if ((i=recv(socket, &LotNr, sizeof(LotNr),MSG_WAITALL)) == -1) {
            perror("Cant read lotnr");
            exit(1);
        }

        //leser destinasjonelengden
        if ((i=recv(socket, &destLeng, sizeof(destLeng),MSG_WAITALL)) == -1) {
            perror("Cant read destLeng");
            exit(1);
        }

        if (destLeng > sizeof(dest)) {
                printf("dest filname is to long at %i\n",destLeng);
                exit(1);
        }

        //leser destinasjonene
        if ((i=recv(socket, &dest, destLeng,MSG_WAITALL)) == -1) {
            perror("Cant read dest");
            exit(1);
        }

	printf("run command %s\n",dest);

	system(dest);

}
else if (packedHedder.command == C_rGetSize) {
	printf("fikk C_rGetSize\n");

      
	//leser data. Det skal væren en int som sier hvilken lot vi vil ha
        if ((i=read(socket, &LotNr, sizeof(LotNr))) == -1) {
            perror("Cant read lotnr");
            exit(1);
        }

	if ((i=read(socket, &filnamelen, sizeof(filnamelen))) == -1) {
            perror("Cant read filnamelen");
            exit(1);
        }

	if (filnamelen > sizeof(buf)) {
		printf("filname to long\n");
	};

	if ((i=read(socket, buf, filnamelen)) == -1) {
            perror("Cant read filnamelen");
            exit(1);
        }	
	
	printf("filname %s\n",buf);
	
	if ((FH = lotOpenFileNoCasheByLotNr(LotNr,buf,"rb",'s',packedHedder.subname)) == NULL) {
		perror(buf);
		//sending that he fil is emty
       		fileBloks = 0;

		sendall(socket,&fileBloks, sizeof(fileBloks));

	}
	else {
		//finner og sender il størelse
		fstat(fileno(FH),&inode);
		//filesize = inode.st_size;
		//sendall(socket,&filesize, sizeof(filesize));

       		fileBloks = inode.st_size;

		printf("size is %" PRId64 "\n",fileBloks);

		sendall(socket,&fileBloks, sizeof(fileBloks));

		fclose(FH);
	}
}
else if (packedHedder.command == C_rGetFile) {
	printf("fikk C_rGetFile\n");

      
	//leser data. Det skal væren en int som sier hvilken lot vi vil ha
        if ((i=read(socket, &LotNr, sizeof(LotNr))) == -1) {
            perror("Cant read lotnr");
            exit(1);
        }

	if ((i=read(socket, &filnamelen, sizeof(filnamelen))) == -1) {
            perror("Cant read filnamelen");
            exit(1);
        }

	if (filnamelen > sizeof(buf)) {
		printf("filname to long\n");
	};

	if ((i=read(socket, buf, filnamelen)) == -1) {
            perror("Cant read filnamelen");
            exit(1);
        }	
	
	printf("filname %s\n",buf);
	
	if ((FH = lotOpenFileNoCasheByLotNr(LotNr,buf,"rb",'s',packedHedder.subname)) == NULL) {
		perror(buf);
		//sending that he fil is emty
       		fileBloks = 0;
	        filerest = 0;

		sendall(socket,&fileBloks, sizeof(fileBloks));
        	sendall(socket,&filerest, sizeof(filerest));

	}
	else {
		//finner og sender il størelse
		fstat(fileno(FH),&inode);
		//filesize = inode.st_size;
		//sendall(socket,&filesize, sizeof(filesize));

       		fileBloks = (int)floor(inode.st_size / rNetTrabsferBlok);
	        filerest = inode.st_size - (fileBloks * rNetTrabsferBlok);

		sendall(socket,&fileBloks, sizeof(fileBloks));
        	sendall(socket,&filerest, sizeof(filerest));

		printf("sending fil. fileBloks %"PRId64", filerest %"PRId64"\n",fileBloks,filerest);


	        filblocbuff = (char *)malloc(rNetTrabsferBlok);
	        for(i=0;i < fileBloks;i++) {

	                fread(filblocbuff,sizeof(c),rNetTrabsferBlok,FH);

	                if ((n=sendall(socket, filblocbuff, rNetTrabsferBlok)) == -1) {
	                        perror("Cant recv dest");
	                        exit(1);
	                }

	        }

	        printf("did recv %i fileBloks\n",i);


	        fread(filblocbuff,sizeof(c),filerest,FH);

	        if ((n=sendall(socket, filblocbuff, filerest)) == -1) {
	                        perror("Cant recv filerest");
	                        exit(1);
	        }

	        free(filblocbuff);


		/*
		for (i=0;i<filesize;i++) {
			fread(&c,sizeof(char),1,FH);
			send(socket, &c, sizeof(char), 0);
			//printf("%i\n",(int)c);
		}
		*/
		printf("send file end\n");		

		fclose(FH);
	}
	
}
else if (packedHedder.command == C_rGetNext) {
	printf("fikk C_rGetNext\n");


	//leser data. Det skal væren en unigned int som sier hvilken lot vi vil ha
	//har deklarert den som int her ???
	if ((i=read(socket, &LotNr, sizeof(LotNr))) == -1) {
            perror("Cant read lotnr");
            exit(1);
	}
	printf("leser FilterTime\n");
	//leser filtertime
	if ((i=read(socket, &FilterTime, sizeof(FilterTime))) == -1) {
            perror("Cant read lotnr");
            exit(1);
        }
	
	printf("lotnr %i FilterTime %u\n",LotNr,FilterTime);

	//henter inn data om den lotten
	if (rGetNext(LotNr,&ReposetoryHeader,htmlbuffer,NULL,&radress,FilterTime,0)) {

		//printf("DocId: %i url: %s\n",ReposetoryHeader.DocID,ReposetoryHeader.url);

		//sender pakke hedder
		sendpacked(socket,C_rLotData,PROTOCOLVERSION, ReposetoryHeader.htmlSize + sizeof(ReposetoryHeader) +sizeof(radress), NULL,packedHedder.subname);	

		//sennder ReposetoryHeader'en
		sendall(socket,&ReposetoryHeader, sizeof(ReposetoryHeader));

		//sender htmlen
		sendall(socket,&htmlbuffer, ReposetoryHeader.htmlSize);

		//sender adressen
		sendall(socket,&radress,sizeof(radress));
		//printf("data sent\n");
	
		//printf("rGetNext: %i\n",ReposetoryHeader.DocID);
		
	}
	else {
		sendpacked(socket,C_rEOF,PROTOCOLVERSION, 0, NULL,packedHedder.subname);
		printf("ferdig\n");
	}
}
else if (packedHedder.command == C_DIWrite) {
	

	if ((i=recv(socket, &DocumentIndexPost, sizeof(struct DocumentIndexFormat),MSG_WAITALL)) == -1) {
                perror("recv");
        	exit(1);
        }
	
	if ((i=recv(socket, &DocID, sizeof(DocID),MSG_WAITALL)) == -1) {
        	perror("recv");
                exit(1);
        }
	
	DIWrite(&DocumentIndexPost,DocID,packedHedder.subname, NULL);
	
	//printf("DIWrite: %i\n",DocID);

}
else if (packedHedder.command == C_DIRead) {

	int DocID;
	struct DocumentIndexFormat DocumentIndexPost;	

	//printf("got commane C_DIRead. sise %i hsize %i ds %i\n",packedHedder.size, sizeof(packedHedder), sizeof(DocID));

	if ((i=recv(socket, &DocID, sizeof(DocID),0)) == -1) {
                perror("recv");
                exit(1);
        }
	//printf("DocID %i\n",DocID);

	//leser inn datan
	//int DIRead (struct DocumentIndexFormat *DocumentIndexPost, int DocID);
	DIRead(&DocumentIndexPost,DocID,packedHedder.subname);

	sendall(socket,&DocumentIndexPost, sizeof(struct DocumentIndexFormat));
}
else if (packedHedder.command == C_rGetIndexTime) {

	int Lotnr;
	unsigned int IndexTime;
	if ((i=recv(socket, &LotNr, sizeof(LotNr),0)) == -1) {
                perror("recv");
                exit(1);
        }

	IndexTime = GetLastIndexTimeForLot(LotNr);

	sendall(socket,&IndexTime, sizeof(IndexTime));

}
else if (packedHedder.command == C_rSetIndexTime) {

        int Lotnr;
        if ((i=recv(socket, &LotNr, sizeof(LotNr),0)) == -1) {
                perror("recv");
                exit(1);
        }

	setLastIndexTimeForLot(LotNr);

}
else if (packedHedder.command == C_rSendFile) {
	//skal mota en fil for lagring i reposetoryet
	char FilePath[156];
	FILE *FILEHANDLER;
	char c;
	path_t *file_path;
	char opentype[2];
	//char *filblocbuff;
	//off_t fileBloks,filerest;

	if ((i=recv(socket, &LotNr, sizeof(LotNr),MSG_WAITALL)) == -1) {
            perror("Cant recv lotnr");
            exit(1);
        }

	printf("lotNr %i\n",LotNr);


	//leser destinasjonelengden
	if ((i=recv(socket, &destLeng, sizeof(destLeng),MSG_WAITALL)) == -1) {
            perror("Cant recv destLeng");
            exit(1);
        }

	if (destLeng > sizeof(dest)) {
		printf("dest filname is to long at %i\n",destLeng);
		exit(1);
	}

	//leser destinasjonene
	if ((i=recv(socket, &dest, destLeng,MSG_WAITALL)) == -1) {
            perror("Cant recv dest");
            exit(1);
        }

	printf("coping %s as length %i in to lot %i\n",dest,destLeng,LotNr);

        if ((i=recv(socket, &opentype, sizeof(char) +1,MSG_WAITALL)) == -1) {
            perror("Cant recv opentype");
            exit(1);
        }
	printf("opentype \"%s\"\n",opentype);


	GetFilPathForLot(FilePath,LotNr,packedHedder.subname);

	//legger til filnavnet
	strncat(FilePath,dest,sizeof(FilePath));

	//leser inn filstørelsen
	if ((i=recv(socket, &fileBloks, sizeof(fileBloks),MSG_WAITALL)) == -1) {
            perror("Cant recv fileBloks");
            exit(1);
        }

	if ((i=recv(socket, &filerest, sizeof(filerest),MSG_WAITALL)) == -1) {
            perror("Cant recv filerest");
            exit(1);
        }

	printf("store in %s, fileBloks: %" PRId64 ", filerest: %" PRId64 "\n",FilePath,fileBloks,filerest);

	//opner filen
	//22.okt.2005: var appand her før. Byttet til at vi kan spesifisere selv
        if ( (FILEHANDLER = fopen(FilePath,opentype)) == NULL ) {
		perror(FilePath);

		//hvis vi ikke fikk opnet filen er det for at pathen kansje ikke fins? Prøver å lage en.
		printf("no path? for %s. Wil try to make\n",FilePath);

		file_path = getpath(FilePath);
		printf("making file path %s\n",file_path->fil_path);
		exit(1);
		makePath(file_path->fil_path);

		if ( (FILEHANDLER = fopen(FilePath,opentype)) == NULL ) {
                	perror(FilePath);
		}
        }

	filblocbuff = (char *)malloc(rNetTrabsferBlok);
	for(i=0;i < fileBloks;i++) {

		if ((n=recv(socket, filblocbuff, rNetTrabsferBlok,MSG_WAITALL)) == -1) {
            		perror("Cant recv dest");
            		exit(1);
        	}

		fwrite(filblocbuff,sizeof(c),rNetTrabsferBlok,FILEHANDLER);
	}

	printf("did recv %i fileBloks\n",i);


	if ((n=recv(socket, filblocbuff, filerest,MSG_WAITALL)) == -1) {
                        perror("Cant recv filerest");
                        exit(1);
        }

        fwrite(filblocbuff,sizeof(c),filerest,FILEHANDLER);


	free(filblocbuff);

	fclose(FILEHANDLER);

	printf("\n");
}
else if (packedHedder.command == C_DIGetIp) {

	
	unsigned int DocID;
        struct DocumentIndexFormat DocumentIndexPost;

	//printf("got command C_DIGetIp\n");

	if ((i=recv(socket, &DocID, sizeof(DocID),MSG_WAITALL)) == -1) {
                perror("recv");
                exit(1);
        }

	//printf("DocID %u\n",DocID);

	DIRead(&DocumentIndexPost,DocID,packedHedder.subname);

	//printf("ipadress: %u\n",DocumentIndexPost.IPAddress);

	sendall(socket,&DocumentIndexPost.IPAddress, sizeof(DocumentIndexPost.IPAddress));

	
}
else if (packedHedder.command == C_anchorAdd) {
	size_t textlen;
	unsigned int DocID;
	char *text;

	printf("Add anchor....\n");
	if ((i = recv(socket, &DocID, sizeof(DocID),MSG_WAITALL)) == -1) {
        	perror("recv");
                exit(1);
        } else if ((i = recv(socket, &textlen, sizeof(textlen), MSG_WAITALL)) == -1) {
        	perror("recv(textlen)");
                exit(1);
        }
	text = malloc(textlen+1);
	text[textlen] = '\0';
	if ((i = recv(socket, text, textlen, MSG_WAITALL)) == -1) {
        	perror("recv(text)");
                exit(1);
        } 

	anchoraddnew(DocID, text, textlen, packedHedder.subname, NULL);
	printf("Text for %d: %s\n", DocID, text);
	
	free(text);
}
else if (packedHedder.command == C_anchorGet) {
	size_t len;
	char *text;
	int LotNr;
	unsigned int DocID;
	printf("Get anchor...\n");

	if ((i = recv(socket, &DocID, sizeof(DocID),MSG_WAITALL)) == -1) {
		perror("recv");
		exit(1);
	}
	LotNr = rLotForDOCid(DocID);
	len = anchorRead(LotNr, packedHedder.subname, DocID, NULL, -1);
	sendall(socket, &len, sizeof(len));
	text = malloc(len+1);
	anchorRead(LotNr, packedHedder.subname, DocID, text, len+1);
	sendall(socket, text, len);
}
else if (packedHedder.command == C_readHTML) {
	unsigned int DocID;
	size_t len;
	size_t rlen;
	char *text;
	char *acla, *acld;
	struct DocumentIndexFormat DocIndex;
	struct ReposetoryHeaderFormat ReposetoryHeader;

	if ((i = recv(socket, &DocID, sizeof(DocID), MSG_WAITALL)) == -1) {
		perror("recv");
		exit(1);
	}

	if ((i = recv(socket, &len, sizeof(len), MSG_WAITALL)) == -1) {
		perror("recv(len)");
		exit(1);
	}
	rlen = len;
	text = malloc(len);

	DIRead(&DocIndex, DocID, packedHedder.subname);

	rReadHtml(text, &len, DocIndex.RepositoryPointer, rlen, DocID, packedHedder.subname, &ReposetoryHeader,
	          &acla, &acld);

	//printf("Got: (%d) %s\n", rlen, text);
	sendall(socket, &len, sizeof(len));
	sendall(socket, text, len);
}
else {
	printf("unnown comand. %i\n", packedHedder.command);
}
//printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
} //while

}
