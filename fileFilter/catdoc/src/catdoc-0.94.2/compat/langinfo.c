#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <langinfo.h>

static char *badParam="";
char* __get_dos_codepage(void) {
	static char codePageName[10];
	union REGS regs;
	regs.x.ax=0x6601;
	intdos(&regs,&regs);
	sprintf(codePageName,"cp%d",(regs.x.bx & 0xFFFF));
	if (regs.x.cflag) {
		return badParam;
	}
	return codePageName;
}	
char *nl_langinfo(nl_item item) {
   if (item == CODESET) {
   		return __get_dos_codepage();
   } else {
	   return badParam;
   }	   
}	

