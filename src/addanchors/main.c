#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "../common/define.h"
#include "../common/lot.h"
#include "../common/reposetoryNET.h"


#define maxOpenFiles 200

struct filesFormat {
	int open;
	int havebeenopen;
	FILE *fh;
	int usetime;
	char name[512];
};


int main (int argc, char *argv[]) {

	//FILE *UPDATEFILE;
	struct anchorfileFormat anchorfileData;
	int lotNr,i,count;
	struct filesFormat files[maxLots];
	int openfilescount,usetimeForclose;
	
	int usetime;
	//tester for at vi har fåt hvilken fil vi skal bruke
	if (argc < 2) {
		printf("Usage: ./addanchors subname < anchorfile \n\n\tanchorfile, fil med tekster på linker\n\n");
		exit(1);
	}

	//char *anchorfile = argv[1];
	char *subname = argv[1];
	/*
	if ((UPDATEFILE = fopen(anchorfile,"rb")) == NULL) {
                printf("Cant read anchorfile ");
                perror(anchorfile);
                exit(1);
        }
	*/
	for (i=0;i<maxLots;i++) {
		files[i].open = 0;	
		files[i].havebeenopen = 0;	
		sprintf(files[i].name,"/tmp/addanchors_%i",i);
	}

	usetime = 1;
	openfilescount = 0;
	count = 0;
	while (fread(&anchorfileData,sizeof(struct anchorfileFormat),1,stdin) > 0) {
	//while(!feof(UPDATEFILE)) {
	//	fread(&anchorfileData,sizeof(struct anchorfileFormat),1,UPDATEFILE);
		
		lotNr = rLotForDOCid(anchorfileData.DocID);

		//hvis filen ikke er open opner vi den
		if (!files[lotNr].open) {
			//printf("opening file %i\n",lotNr);
			if ((files[lotNr].fh = fopen(files[lotNr].name,"ab")) == NULL) {
				perror(files[lotNr].name);
				exit(1);
			}
			files[lotNr].open = 1;
			files[lotNr].havebeenopen = 1;
			openfilescount++;
		}
		else {
			//printf("fil allerede open: %i\n",lotNr);
		}
		//setter når den ble brukt sist
		files[lotNr].usetime = usetime++;

		for(i=sizeof(anchorfileData.text); ((i>0) && ((anchorfileData.text[i] == ' ') || (anchorfileData.text[i] == '\0'))); i--) {
                	//printf("i: %i, c: %c cn: %i\n",i,text[i],(int)text[i]);
        	}
        	anchorfileData.text[i +1] = '\0';

		//printf("anchorfileData.text %s\n",anchorfileData.text);

		//skriver DocID
        	fwrite(&anchorfileData.DocID,sizeof(unsigned int),1,files[lotNr].fh);
        	//skriver teksten
        	fputs(anchorfileData.text,files[lotNr].fh);
        	//skriver line seperator
        	fwrite("***",sizeof(char),3,files[lotNr].fh);

		//printf("%i (%i): %s\n",anchorfileData.DocID,lotNr,anchorfileData.text);
		//fwrite(&anchorfileData,sizeof(struct anchorfileFormat),1,files[lotNr].fh);

		//anchoradd(anchorfileData.DocID,anchorfileData.text,sizeof(anchorfileData.text));

		if (openfilescount > maxOpenFiles) {
			usetimeForclose = (usetime - maxOpenFiles);
			printf("will close files whid smaler usetim then %i\n",usetimeForclose);
			for (i=0;i<maxLots;i++) {
				if ((files[i].open) && (files[i].usetime < usetimeForclose)) {
					//printf("closing file %i, usetime %i\n",i,usetime);
					fclose(files[i].fh);
					files[i].open = 0;
					--openfilescount;
				}
			}
			printf("open files is now %i\n",openfilescount);
		}

		




		
		//printf("%i\n",count);
		//if (count > 200) {
		//	break;
		//}

		++count;
	}
	//fclose(UPDATEFILE);


	for (i=0;i<maxLots;i++) {
		if(files[i].havebeenopen) {
			printf("file %s hav been open\n",files[i].name);
                	if (files[i].open) {
				fclose(files[i].fh);
			}
			//sender den over nettverket

			rSendFile(files[i].name, "anchors", i, "a", subname);
			
			
			//sletter filen
			unlink(files[i].name);
		}	
	}
}



