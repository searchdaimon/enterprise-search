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
#include "../common/reposetory.h"

#include "../common/getpath.h"

#define PROTOCOLVERSION 1

void connectHandler(int socket);

char *urltodociddb;

int
main (int argc, char **argv)
{
	int ch;

	urltodociddb = NULL;
	while ((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
				urltodociddb = optarg;
				break;
			case '?':
			default:
				err(1, "Unknown argument '%c'", ch);
		}
	}
	argc -= optind;
	argv += optind;

	sconnect(connectHandler, BLDPORT);

	printf("connect done\n");

	return (0);
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
	unsigned int radress;

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
		
			printf("C_rmkdir\n");

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

			printf("~C_rmkdir\n");


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
		else if (packedHedder.command == C_getLotToIndex) {
			printf("fikk C_getLotToIndex\n");

			int dirty;

			if ((i=recv(socket, &dirty, sizeof(dirty),MSG_WAITALL)) == -1) {
				perror("Cant read dirty");
				exit(1);
			}

			printf("dirty: %i\n",dirty);

			LotNr = findLotToIndex(packedHedder.subname,dirty);

			printf("sending respons\n");
			sendall(socket,&LotNr, sizeof(LotNr));

		}
		else if (packedHedder.command == C_getlotHasSufficientSpace) {
			printf("fikk C_getLotToIndex\n");

			int needSpace;
			int response;

			if ((i=read(socket, &LotNr, sizeof(LotNr))) == -1) {
				perror("Cant read lotnr");
				exit(1);
			}

			if ((i=recv(socket, &needSpace, sizeof(needSpace),MSG_WAITALL)) == -1) {
				perror("Cant read dirty");
				exit(1);
			}


			printf("needSpace: %i, LotNr %i\n",needSpace,LotNr);


			response = lotHasSufficientSpace(LotNr, needSpace, packedHedder.subname);


			printf("sending respons\n");
			sendall(socket,&response, sizeof(response));

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
				//sending that the fil is emty
				fileBloks = 0;
				filerest = 0;

				sendall(socket,&fileBloks, sizeof(fileBloks));
				sendall(socket,&filerest, sizeof(filerest));

			}
			else {
				//finner og sender fil størelse
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

					//fread(filblocbuff,sizeof(c),rNetTrabsferBlok,FH);
					//fread_all(const void *buf, size_t size, FILE *stream)
					fread_all(filblocbuff,rNetTrabsferBlok,FH, 4096);

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

	printf("støttes ikke lengere");
	exit(1);
	/*
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
	*/
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

			printf("got commane C_DIRead. sise %i hsize %i ds %i\n",packedHedder.size, sizeof(packedHedder), sizeof(DocID));

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

			IndexTime = GetLastIndexTimeForLot(LotNr,packedHedder.subname);

			sendall(socket,&IndexTime, sizeof(IndexTime));

		}
		else if (packedHedder.command == C_rSetIndexTime) {

			int Lotnr;
			if ((i=recv(socket, &LotNr, sizeof(LotNr),0)) == -1) {
				perror("recv");
				exit(1);
			}

			setLastIndexTimeForLot(LotNr,NULL,packedHedder.subname);

		}
		else if (packedHedder.command == C_rSendFile) {
			//skal mota en fil for lagring i reposetoryet
			//char FilePath[156];
			FILE *FILEHANDLER;
			char c;
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


			//GetFilPathForLot(FilePath,LotNr,packedHedder.subname);

			//legger til filnavnet
			//strncat(FilePath,dest,sizeof(FilePath));

			//leser inn filstørelsen
			if ((i=recv(socket, &fileBloks, sizeof(fileBloks),MSG_WAITALL)) == -1) {
				perror("Cant recv fileBloks");
				exit(1);
			}

			if ((i=recv(socket, &filerest, sizeof(filerest),MSG_WAITALL)) == -1) {
				perror("Cant recv filerest");
				exit(1);
			}

			printf("fileBloks: %" PRId64 ", filerest: %" PRId64 "\n",fileBloks,filerest);

			//åpner filen
			if ((FILEHANDLER = lotOpenFileNoCasheByLotNr(LotNr,dest,opentype,'e',packedHedder.subname)) == NULL) {
				perror(dest);
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
			printf("got DocID %u\n",DocID);
			LotNr = rLotForDOCid(DocID);
			printf("trying to read anchor\n");

			len = anchorRead(LotNr, packedHedder.subname, DocID, NULL, -1);
			printf("got anchor of length %i\n",len);

			sendall(socket, &len, sizeof(len));
			text = malloc(len+1);
			
			printf("readint it again\n");
			anchorRead(LotNr, packedHedder.subname, DocID, text, len+1);
			sendall(socket, text, len);
		}
		else if (packedHedder.command == C_readHTML) {
			/*
			unsigned int DocID;
			unsigned int len;
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
			printf("len %u\n",len);
			text = malloc(len);

			if (text == NULL)
				exit(1);

			DIRead(&DocIndex, DocID, packedHedder.subname);


			if (!rReadHtml(
					text, 
					&len, 
					DocIndex.RepositoryPointer, 
					DocIndex.htmlSize,
					DocID, 
					packedHedder.subname, 
					&ReposetoryHeader,
					&acla, 
					&acld, 
					DocIndex.imageSize)) {
				len = 0;
				sendall(socket, &len, sizeof(len));
			} else {
				++len; // \0
				#ifdef DEBUG
				printf("docID %u\n",DocID);
				printf("Got: (len %i, real %i) ########################\n%s\n#####################\n", len, strlen(text), text);
				#endif
				sendall(socket, &len, sizeof(len));
				sendall(socket, text, len);
				sendall(socket, &ReposetoryHeader,sizeof(ReposetoryHeader));
			}

			free(text);
			*/
		}
		/*
		runarb: 06 des 2007: vi har gåt bort fra denne metoden for nå, og bruker heller index over smb. Men tar vare på den da vi kan trenge den siden

		else if (packedHedder.command == C_urltodocid) {
			char cmd;
			int alloclen;
			char *urlbuf;

			if (urltodociddb == NULL) {
				cmd = C_DOCID_NODB;
				sendall(socket, &cmd, sizeof(cmd));
				exit(1);
			} else {
				cmd = C_DOCID_READY;
				sendall(socket, &cmd, sizeof(cmd));
			}	
			cmd = C_DOCID_NEXT;

			alloclen = 1024;
			urlbuf = malloc(alloclen);

			do {
				unsigned int DocID;
				size_t len;
				if ((i = recv(socket, &cmd, sizeof(cmd), MSG_WAITALL)) == -1) {
					err(1, "recv(cmd)");
				}
				if (cmd == C_DOCID_DONE)
					break;

				if ((i == recv(socket, &len, sizeof(len), MSG_WAITALL)) == -1) {
					err(1, "recv(len)");
				}
				if (alloclen < len+1) {
					free(urlbuf);
					alloclen *= 2;
					urlbuf = malloc(alloclen);
				}
				if ((i == recv(socket, urlbuf, len, MSG_WAITALL)) == -1) {
					err(1, "recv(len)");
				}
				urlbuf[len] = '\0';

				if (!getDocIDFromUrl(urltodociddb, urlbuf, &DocID)) {
					cmd = C_DOCID_NOTFOUND;
					sendall(socket, &cmd, sizeof(cmd));
				} else {
					cmd = C_DOCID_FOUND;
					sendall(socket, &cmd, sizeof(cmd));
					sendall(socket, &DocID, sizeof(DocID));
				}
			} while (1);

			free(urlbuf);
		}
		*/
		else {
			printf("unnown comand. %i\n", packedHedder.command);
		}
		//printf("size is: %i\nversion: %i\ncommand: %i\n",packedHedder.size,packedHedder.version,packedHedder.command);
	} //while

}
