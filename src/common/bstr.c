#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bstr.h"
#include "debug.h"
#include "strlcat.h"



char *strdupnul(char *in) {
	if (in == NULL) {
		return NULL;
	}
	else {
		return strdup(in);
	}
}

int btolower(int c) {

        c = (unsigned char)c;

        if ((c >= 65) && (c <= 90)) {
                //A-Z
                c = c+32;
        }
        else if (c >= 192 && (c <=221)) {
                //øæå og andre utviede chars over 127
                c = c+32;
        }

        return c;
}



void chomp(char string[]) {
        int i;


	i = strlen(string);
	--i;

        //ToDo: må teste for opperativsystem her:
        /*
        //Windows:
        if (string[i] == '\n') {
                string[i -1] = '\0';
        }
        */
        //Unix:
        if (string[i] == '\n') {
                string[i] = '\0';
        }

}

void strsandr (char text[], char find[], char replace[]) {

	char *cptr;
	int rlen = strlen(replace);
	int flen = strlen(find);
	//blir problemer her for korte strenger med slange replasments. Slenger på +512 som er midlertidig fix
	char *buff = malloc((strlen(text) * 2) +512);

	char *bptr;
	char *cprtold = text;

	buff[0] = '\0';

	cptr = text;
	bptr = buff;

	strcpy(buff,text);	

	while ((cptr = strstr(cptr,find))) {

		bptr = (bptr + (cptr - cprtold));

		//går over treffet
		cptr = (cptr + flen);		

		//kopierer inn replacmnt
		strcpy(bptr,replace);
		bptr = (bptr + rlen);
	

		//kopierer inn resten
		strcat(bptr,cptr);
		
		cprtold = cptr;

	}

	strcpy(text,buff);	

	free(buff);
}

void ntobr (char textbuff[], int texbufftsize) {

	char replace[] = {"<br>\n"};
	char find[] = {"\n"};
	char *cptr;
	int rlen = strlen(replace);
	int flen = strlen(find);
	char *buff = malloc((texbufftsize * 2));
	char *bptr;
	char *cprtold = textbuff;

	buff[0] = '\0';

	cptr = textbuff;
	bptr = buff;

	strcpy(buff,textbuff);	

	while ((cptr = (char *)strstr(cptr,find))) {

		if ((cptr == textbuff) || (cptr[-1] == '>')) {

			bptr = (bptr + (cptr - cprtold));

			//går over treffet
			cptr = (cptr + flen);		

			//kopierer inn replacmnt
			strcpy(bptr,find);

			bptr = (bptr + flen);
		
			//kopierer inn resten
			strcat(bptr,cptr);
		} 
		else {
			bptr = (bptr + (cptr - cprtold));

			//går over treffet
			cptr = (cptr + flen);		

			strcpy(bptr,replace);

			bptr = (bptr + rlen);
		
			//kopierer inn resten
			strcat(bptr,cptr);
		}

		cprtold = cptr;
	}

	strscpy(textbuff,buff,texbufftsize);	

	free(buff);
}

void strcasesandr (char textbuff[], int texbufftsize,char find[], char replace[]) {

	char *cptr;
	int rlen = strlen(replace);
	int flen = strlen(find);
	char *buff = malloc((texbufftsize * 2));
	char *bptr;
	char *cprtold = textbuff;

	buff[0] = '\0';

	cptr = textbuff;
	bptr = buff;

	strcpy(buff,textbuff);	

	while ((cptr = (char *)strcasestr(cptr,find))) {

		bptr = (bptr + (cptr - cprtold));

		//går over treffet
		cptr = (cptr + flen);		

		//kopierer inn replacmnt
		strcpy(bptr,replace);
		bptr = (bptr + rlen);
	
		//kopierer inn resten
		strcat(bptr,cptr);

		cprtold = cptr;

	}

	strscpy(textbuff,buff,texbufftsize);	

	free(buff);
}

void saafree(char **List) {

	int Count = 0;

	while( (List[Count] != NULL) ) {
		free(List[Count++]);
	}

	free(List);

	return;

}

//string kopieringsrutine som sørger for at man aldri kopierer over størelsen til dest, og altid slutter på \0
//dropp in replacsement for strncpy()
void strscpy(char *dest, const char *src, size_t n) {

	int i,srcstrlen;

 	srcstrlen = strlen(src);

	if ((unsigned int)srcstrlen >= n) {
		srcstrlen = (n -1);
	}

	for(i=0;i<srcstrlen;i++) {
		dest[i] = src[i];
	}
	dest[srcstrlen] = '\0';


}

/* Split.c - C-language equivilant of Perl's split function
   - Written Dec, 1999 by Jeff Hay, jrhay@lanl.gov
   - Modified to standalone module, Sept. 2000, Jeff Hay

   -from http://home.earthlink.net/~jrhay/src/split/split.c
   -runarb: ingen lisens på siden.

   Emulate Perl's "split" function - returns an array of strings and the
   number of elements in the array (ie, number of columns in original string).
   Last element is NULL. Leaves original string intact. Always returns at
   least 1 element.  Returns "" in element if Delim appears twice in a row.
   Returns < 0 on any error.
*/


/* If we're making a test/demo program, make sure DEBUG is defined
   for our debugging mallocs.... */
#ifdef __TEST_SPLIT__
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif



int split(const char *Input, char *Delim, char ***List) {

	int Found;
	int MaxFound;
	int Length;
	int DelimLen;
	const char* Remain;
	char* Position;

	DelimLen = strlen(Delim);
	Found = 0;
	Remain = Input;
	MaxFound = 10;

	if ((List == NULL) || (Input == NULL) || (Delim == NULL)) {
		return -1;
	}


	if ((*List = (char **)malloc(MaxFound * sizeof(char *))) == NULL) {
		perror("malloc List");
		return -1;
	}

	while ((Position = strstr(Remain, Delim)) != NULL) {
		Length = Position - Remain;
		
		if((((*List)[Found]) = (char *)malloc(sizeof(char)*(Length+1))) == NULL) {
			perror("split: malloc list element");
			return -1;
		}
		strncpy(((*List)[Found]), Remain, Length);
		((*List)[Found])[Length] = 0;

		Found++;
		Remain = Position + DelimLen;
		
		if (Found == MaxFound) {
			MaxFound += 10;
			*List = (char **)realloc(*List, MaxFound * sizeof(char *));
		}
	}

	if (Found+1 == MaxFound) {
		MaxFound += 2;
		*List = (char **)realloc(*List, MaxFound * sizeof(char *));
	}
	
	Length = strlen(Remain);
	((*List)[Found]) = (char *)malloc(sizeof(char)*(Length+1));
	strncpy(((*List)[Found]), Remain, Length);
	((*List)[Found])[Length] = 0;

	Found++;
	((*List)[Found]) = NULL;

	return Found;
} /* split() */

/* Destroys the array of strings structrue returned by split() */
void FreeSplitList(char **List) {
	int Count;

	if (List == NULL) {
		return;
	}

	Count = 0;
	while( (List[Count] != NULL) ) {
	free(List[Count++]);
	}

	free(List);

	return;
} /* FreeSplitList() */

// like strlcat, but test self for if the src is to long to add to dst, and givs a warning
size_t strlwcat(char *dst, const char *src, size_t siz) {

	size_t len;

	len = strlcat(dst,src,siz);

	if (len >= siz) {
		printf("strlwcat: ENAMETOOLONG\n");
	}

	return len;
}

#ifdef __TEST_SPLIT__
void dosplit(char *string, char *delim) {

	char **Data;
	int Count, TokCount;

	printf("Splitting: \"%s\" on \"%s\"\n", string, delim);

	TokCount = split(string, delim, &Data);
	printf("\tfound %d token(s):\n", TokCount);

	Count = 0;
	while( (Data[Count] != NULL) ) {
		printf("\t\t%d\t\"%s\"\n", Count, Data[Count++]);
	}
	
	printf("\n");

	FreeSplitList(Data);

	return;
} /* dosplit() */

int main() {
	char *splitstring1 = "this;is;a;test;of;split";
	char *splitstring2 = "this<skip>is<skip>a<skip>test<skip>of<skip>split";
	char *splitstring3 = "this is a test  of skip";

	printf("C Version Of Split testing program\n\n");

	dosplit(splitstring1, ";");
	dosplit(splitstring2, "<skip>");
	dosplit(splitstring3, "*");
	dosplit(splitstring3, " ");

	printf("C-Split Done\n");

	return 0;
} /* main() */
#endif //__TEST_SPLIT__

