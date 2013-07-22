/**
 * @file   pptparse.c
 * @author Alex Ott <alexott@gmail.com>
 * @date   23 ‰≈À 2004
 * Version: $Id: pptparse.c,v 1.1.1.1 2007/12/04 18:43:09 boitho Exp $
 * Copyright: Alex Ott
 * 
 * @brief  .ppt parsing routines
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "ppt.h"
#include "catdoc.h"
#include "ppttypes.h"

static void process_item (int rectype, long reclen, FILE* input);

#if !defined(min)
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif


/** 
 * 
 * 
 * @param input 
 * @param filename 
 */
void do_ppt(FILE *input,char *filename) {
	int itemsread=1;
	int rectype;
	long reclen;
	unsigned char recbuf[8];

	while(itemsread) {
		itemsread = catdoc_read(recbuf, 1, 8, input);
/* 		fprintf(stderr,"itemsread=%d: ",itemsread); */
/* 		for(i=0; i<8; i++) */
/* 			fprintf(stderr,"%02x ",recbuf[i]); */
/* 		fprintf(stderr,"\n"); */
		
		if (catdoc_eof(input)) {
			process_item(DOCUMENT_END,0,input);
			return;
		}
		if(itemsread < 8)
			break;
		rectype=getshort(recbuf,2);
		reclen=getulong(recbuf,4);
		if (reclen < 0) {
			return;
		}	
		process_item(rectype,reclen,input);
	}
}


/** 
 * 
 * 
 * @param rectype 
 * @param reclen 
 * @param input 
 */
static void process_item (int rectype, long reclen, FILE* input) {
	int i=0, u;
	static char buf[2];

	switch(rectype) {
	case DOCUMENT_END:
/* 		fprintf(stderr,"End of document, ended at %ld\n",catdoc_tell(input)); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case DOCUMENT:
/* 		fprintf(stderr,"Start of document, reclen=%ld, started at %ld\n", reclen, */
/* 						catdoc_tell(input)); */
		break;

	case DOCUMENT_ATOM:
/* 		fprintf(stderr,"DocumentAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case SLIDE:
/* 		fprintf(stderr,"Slide, reclen=%ld\n", reclen); */
/*  		fputs("---------------------------------------\n",stderr); */
		break;

	case SLIDE_ATOM:
/* 		fprintf(stderr,"SlideAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;
		
	case SLIDE_BASE:
/* 		fprintf(stderr,"SlideBase, reclen=%ld\n", reclen); */
		break;

	case SLIDE_BASE_ATOM:
/* 		fprintf(stderr,"SlideBaseAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;
		
	case NOTES:
/* 		fprintf(stderr,"Notes, reclen=%ld\n", reclen); */
		break;

	case NOTES_ATOM:
/* 		fprintf(stderr,"NotesAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;
		
	case HEADERS_FOOTERS:
/* 		fprintf(stderr,"HeadersFooters, reclen=%ld\n", reclen); */
		break;

	case HEADERS_FOOTERS_ATOM:
/* 		fprintf(stderr,"HeadersFootersAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;
		
	case MAIN_MASTER:
/* 		fprintf(stderr,"MainMaster, reclen=%ld\n", reclen); */
		break;
		
	case TEXT_BYTES_ATOM: {
/* 			fprintf(stderr,"TextBytes, reclen=%ld\n", reclen); */
			for(i=0; i < reclen; i++) {
				catdoc_read(buf,1,1,input);
				if((unsigned char)*buf!=0x0d)
					fputs(convert_char((unsigned char)*buf),stdout);
				else
					fputc('\n',stdout);
			}
			fputc('\n',stdout);
		}
		break;
		
	case TEXT_CHARS_ATOM: 
	case CSTRING: {
			long text_len;
			
/* 			fprintf(stderr,"CString, reclen=%ld\n", reclen); */
			text_len=reclen/2;
			for(i=0; i < text_len; i++) {
				catdoc_read(buf,2,1,input);
				u=(unsigned short)getshort(buf,0);
				if(u!=0x0d)
					fputs(convert_char(u),stdout);
				else
					fputc('\n',stdout);
			}
			fputc('\n',stdout);
		}
		break;
		
	case USER_EDIT_ATOM:
/* 		fprintf(stderr,"UserEditAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case COLOR_SCHEME_ATOM:
/* 		fprintf(stderr,"ColorSchemeAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case PPDRAWING:
/* 		fprintf(stderr,"PPDrawing, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case ENVIRONMENT:
/* 		fprintf(stderr,"Environment, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case SSDOC_INFO_ATOM:
/* 		fprintf(stderr,"SSDocInfoAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case SSSLIDE_INFO_ATOM:
/* 		fprintf(stderr,"SSSlideInfoAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case PROG_TAGS:
/* 		fprintf(stderr,"ProgTags, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case PROG_STRING_TAG:
/* 		fprintf(stderr,"ProgStringTag, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case PROG_BINARY_TAG:
/* 		fprintf(stderr,"ProgBinaryTag, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case LIST:
/* 		fprintf(stderr,"List, reclen=%ld\n", reclen); */
		break;

	case SLIDE_LIST_WITH_TEXT:
/* 		fprintf(stderr,"SlideListWithText, reclen=%ld\n", reclen); */
/*  		fputs("---------------------------------------\n",stderr); */
		break;

	case PERSIST_PTR_INCREMENTAL_BLOCK:
/* 		fprintf(stderr,"PersistPtrIncrementalBlock, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case EX_OLE_OBJ_STG:
/* 		fprintf(stderr,"ExOleObjStg, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case PPDRAWING_GROUP:
/* 		fprintf(stderr,"PpdrawingGroup, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case EX_OBJ_LIST:
/* 		fprintf(stderr,"ExObjList, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case TX_MASTER_STYLE_ATOM:
/* 		fprintf(stderr,"TxMasterStyleAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case HANDOUT:
/* 		fprintf(stderr,"Handout, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case SLIDE_PERSIST_ATOM:
/* 		fprintf(stderr,"SlidePersistAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case TEXT_HEADER_ATOM:
/* 		fprintf(stderr,"TextHeaderAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case TEXT_SPEC_INFO:
/* 		fprintf(stderr,"TextSpecInfo, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

	case STYLE_TEXT_PROP_ATOM:
/* 		fprintf(stderr,"StyleTextPropAtom, reclen=%ld\n", reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);
		break;

		/*	case :
		fprintf(stderr,", reclen=%ld\n", reclen);
		catdoc_seek(input, reclen, SEEK_CUR);
		break;*/

		/*	case :
		fprintf(stderr,", reclen=%ld\n", reclen);
		catdoc_seek(input, reclen, SEEK_CUR);
		break;*/

	default:
/* 		fprintf(stderr,"Default action for rectype=%d reclen=%ld\n", */
/* 						rectype, reclen); */
		catdoc_seek(input, reclen, SEEK_CUR);

	}
	
}
