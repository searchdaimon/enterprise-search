#include <stdlib.h> 
#include <stdio.h> 

int main (int argc, char *argv[]) {

	FILE *Wordfile;
	char buff [512];
	int fcMin;
	int fcMac;
	int DataSize;

        if (argc < 2) {
                printf("Dette programet konverterer en Microsoft Word file til text. Gi den en wordfile\n");
                exit(0);
        }


	//opner filen
	if ((Wordfile = fopen(argv[1],"rb")) == NULL) {
		perror(argv[1]);
		exit(1);
	}

	//static char buff [] = {'2','3'};
	//word filer består av blokker på 512 bytes
	//en Word fil begynner med en som block 1.
	//denne skal begynne med hex verdien "d0 cf 11 e0  a1 b1 1a e1"
	//vi tester derfor om dette er en word fil vi kan lese ved å teste for "d0 cf 11 e0 a1 b1 1a e1"
	
	//fseek(Wordfile,0,0);
	//fread(buff, sizeof(char),8,Wordfile);
	//block nr 2 er den såkalte  FIB (File Information Block) som ineholder informasjon og pekere til data og blokker.
	//verdien med offset 24 er en peker til selve teksten. Dette er datatype "long"
	//Blokk 2 begynner på adresse 512 (hver blokk er på 512 bytes og vi begynner på 1, ikke 0)
	//altså er ofset 24 i adresse 24 + 512 = 536.

	//søker til adresse 536
	fseek(Wordfile,536,0);
	//funner start verdien. Dette er ofsettet der dataene begynner
	fread(&fcMin,sizeof(int),1,Wordfile);
	//Verdien på block 2 offsett 28 ineholder hvor dataen stopper
	//ofset 28 i adresse 28 + 512 = 540.
	fseek(Wordfile,540,0);
	fread(&fcMac,sizeof(int),1,Wordfile);
	//$fcMin og $fcMac er ofsett verdier fra blok 3. Altså må vi legge til 512 * 2 = 1024 bytes
	fcMin += 1024;
	fcMac += 1024;
	//data størelse er $fcMac - $fcMin
	DataSize = fcMac - fcMin;
	printf("%i -> %i as %i b in size\n",fcMin,fcMac,DataSize);
	//Søker til $fcMin og leser $DataSize
	fseek(Wordfile,fcMin,0);
	fread(&fcMac,sizeof(char),DataSize,Wordfile);
	printf("%s",buff); }
