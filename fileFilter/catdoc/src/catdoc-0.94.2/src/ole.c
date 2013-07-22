/**
 * @file   ole.c
 * @author Alex Ott, Victor B Wagner
 * @date   Wed Jun 11 12:33:01 2003
 * Version: $Id: ole.c,v 1.1.1.1 2007/12/04 18:43:09 boitho Exp $
 * Copyright: Victor B Wagner, 1996-2003 Alex Ott, 2003
 * 
 * @brief  Parsing structure of MS Office compound document
 * 
 * This file is part of catdoc project
 * and distributed under GNU Public License
 * 
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "catdoc.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

long int sectorSize, shortSectorSize;
/* BBD Info */
long int  bbdNumBlocks;
unsigned char *BBD=NULL;
/* SBD Info */
long int sbdNumber, sbdStart, sbdLen;
unsigned char *SBD=NULL;
oleEntry *rootEntry=NULL;
/* Properties Info */
long propCurNumber, propLen, propNumber, propStart;
unsigned char *properties=NULL;
long int fileLength=0;

static unsigned char ole_sign[]={0xD0,0xCF,0x11,0xE0,0xA1,0xB1,0x1A,0xE1,0};


/** 
 * Initializes ole structure
 * 
 * @param f (FILE *) compound document file, positioned at bufSize
 *           byte. Might be pipe or socket 
 * @param buffer (void *) bytes already read from f
 * @param bufSize number of bytes already read from f should be less
 *                than 512 
 * 
 * @return 
 */
FILE* ole_init(FILE *f, void *buffer, size_t bufSize)  {
	unsigned char oleBuf[BBD_BLOCK_SIZE];
	unsigned char *tmpBuf;
	FILE *newfile;
	int ret=0, i;
	long int sbdMaxLen, sbdCurrent, propMaxLen, propCurrent, mblock, msat_size;
	oleEntry *tEntry;

	/* deleting old data (if it was allocated) */
	ole_finish();
	
	if (fseek(f,0,SEEK_SET) == -1) {
		if ( errno == ESPIPE ) {
			/* We got non-seekable file, create temp file */
			if((newfile=tmpfile()) == NULL) {
				perror("Can't create tmp file");
				return NULL;
			}
			if (bufSize > 0) {
				ret=fwrite(buffer, 1, bufSize, newfile);
				if(ret != bufSize) {
					perror("Can't write to tmp file");
					return NULL;
				}
			}
			
			while(!feof(f)){
				ret=fread(oleBuf,1,BBD_BLOCK_SIZE,f);
				fwrite(oleBuf, 1, ret, newfile);
			}
			fseek(newfile,0,SEEK_SET);
		} else {
			perror("Can't seek in file");
			return NULL;
		}
	} else {
		newfile=f;
	}	
	fseek(newfile,0,SEEK_END);
	fileLength=ftell(newfile);
/* 	fprintf(stderr, "fileLength=%ld\n", fileLength); */
	fseek(newfile,0,SEEK_SET);
	ret=fread(oleBuf,1,BBD_BLOCK_SIZE,newfile);
	if ( ret != BBD_BLOCK_SIZE ) {
		return NULL;
	}
	if (strncmp(oleBuf,ole_sign,8) != 0) {
		return NULL;
	}
 	sectorSize = 1<<getshort(oleBuf,0x1e);
	shortSectorSize=1<<getshort(oleBuf,0x20);
	
/* Read BBD into memory */
	bbdNumBlocks = getulong(oleBuf,0x2c);
	if((BBD=malloc(bbdNumBlocks*sectorSize)) == NULL ) {
		return NULL;
	}
	
	if((tmpBuf=malloc(MSAT_ORIG_SIZE)) == NULL ) {
		return NULL;
	}
	memcpy(tmpBuf,oleBuf+0x4c,MSAT_ORIG_SIZE);
	mblock=getlong(oleBuf,0x44);
	msat_size=getlong(oleBuf,0x48);

/* 	fprintf(stderr, "msat_size=%ld\n", msat_size); */

	i=0;
	while((mblock >= 0) && (i < msat_size)) {
		unsigned char *newbuf;
/* 		fprintf(stderr, "i=%d mblock=%ld\n", i, mblock); */
		if ((newbuf=realloc(tmpBuf, sectorSize*(i+1)+MSAT_ORIG_SIZE)) != NULL) {
			tmpBuf=newbuf;
		} else {
			perror("MSAT realloc error");
			free(tmpBuf);
			ole_finish();
			return NULL;
		}
		
		fseek(newfile, 512+mblock*sectorSize, SEEK_SET);
		if(fread(tmpBuf+MSAT_ORIG_SIZE+(sectorSize-4)*i,
						 1, sectorSize, newfile) != sectorSize) {
			fprintf(stderr, "Error read MSAT!\n");
			ole_finish();
			return NULL;
		}

		i++;
		mblock=getlong(tmpBuf, MSAT_ORIG_SIZE+(sectorSize-4)*i);
	}
	
/* 	fprintf(stderr, "bbdNumBlocks=%ld\n", bbdNumBlocks); */
	for(i=0; i< bbdNumBlocks; i++) {
		long int bbdSector=getlong(tmpBuf,4*i);
		
		if (bbdSector >= fileLength/sectorSize || bbdSector < 0) {
			fprintf(stderr, "Bad BBD entry!\n");
			ole_finish();
			return NULL;
		}
		fseek(newfile, 512+bbdSector*sectorSize, SEEK_SET);
		if ( fread(BBD+i*sectorSize, 1, sectorSize, newfile) != sectorSize ) {
			fprintf(stderr, "Can't read BBD!\n");
			free(tmpBuf);
			ole_finish();
			return NULL;
		}
	}
	free(tmpBuf);
	
/* Read SBD into memory */
	sbdLen=0;
	sbdMaxLen=10;
	sbdCurrent = sbdStart = getlong(oleBuf,0x3c);
	if (sbdStart > 0) {
		if((SBD=malloc(sectorSize*sbdMaxLen)) == NULL ) {
			ole_finish();
			return NULL;
		}
		while(1) {
			fseek(newfile, 512+sbdCurrent*sectorSize, SEEK_SET);
			fread(SBD+sbdLen*sectorSize, 1, sectorSize, newfile);
			sbdLen++;
			if (sbdLen >= sbdMaxLen) {
				unsigned char *newSBD;
				
				sbdMaxLen+=5;
				if ((newSBD=realloc(SBD, sectorSize*sbdMaxLen)) != NULL) {
					SBD=newSBD;
				} else {
					perror("SBD realloc error");
					ole_finish();
					return NULL;
				}
			}
			sbdCurrent = getlong(BBD, sbdCurrent*4);
			if(sbdCurrent < 0 ||
				sbdCurrent >= fileLength/sectorSize)
				break;
		}
		sbdNumber = (sbdLen*sectorSize)/shortSectorSize;
/*   		fprintf(stderr, "sbdLen=%ld sbdNumber=%ld\n",sbdLen, sbdNumber); */
	} else {
		SBD=NULL;
	}
/* Read property catalog into memory */
	propLen = 0;
	propMaxLen = 5;
	propCurrent = propStart = getlong(oleBuf,0x30);
	if (propStart >= 0) {
		if((properties=malloc(propMaxLen*sectorSize)) == NULL ) {
			ole_finish();
			return NULL;
		}
		while(1) {
/*  			fprintf(stderr, "propCurrent=%ld\n",propCurrent); */
			fseek(newfile, 512+propCurrent*sectorSize, SEEK_SET);
			fread(properties+propLen*sectorSize,
				  1, sectorSize, newfile);
			propLen++;
			if (propLen >= propMaxLen) {
				unsigned char *newProp;
				
				propMaxLen+=5;
				if ((newProp=realloc(properties, propMaxLen*sectorSize)) != NULL)
					properties=newProp;
				else {
					perror("Properties realloc error");
					ole_finish();
					return NULL;
				}
			}
			
			propCurrent = getlong(BBD, propCurrent*4);
			if(propCurrent < 0 ||
			   propCurrent >= fileLength/sectorSize ) {
				break;
			}
		}
/*  		fprintf(stderr, "propLen=%ld\n",propLen); */
		propNumber = (propLen*sectorSize)/PROP_BLOCK_SIZE;
		propCurNumber = 0;
	} else {
		ole_finish();
		properties = NULL;
		return NULL;
	}
	
	
/* Find Root Entry */
	while((tEntry=(oleEntry*)ole_readdir(newfile)) != NULL) {
		if (tEntry->type == oleRootDir ) {
			rootEntry=tEntry;
			break;
		}
		ole_close((FILE*)tEntry);
	}
	propCurNumber = 0;
	fseek(newfile, 0, SEEK_SET);
	if (!rootEntry) {
		fprintf(stderr,"Broken OLE structure. Cannot find root entry in this file!\n");		ole_finish();
		return NULL;
	}	
	return newfile;
}

/** 
 * 
 * 
 * @param oleBuf 
 * 
 * @return 
 */
int rightOleType(unsigned char *oleBuf) {
	return (oleBuf[0x42] == 1 || oleBuf[0x42] == 2 ||
			oleBuf[0x42] == 3 || oleBuf[0x42] == 5 );
}

/** 
 * 
 * 
 * @param oleBuf 
 * 
 * @return 
 */
oleType getOleType(unsigned char *oleBuf) {
	return (oleType)((unsigned char)oleBuf[0x42]);
}

/** 
 * Reads next directory entry from file
 * 
 * @param name buffer for name converted to us-ascii should be at least 33 chars long
 * @param size size of file 
 * 
 * @return 0 if everything is ok -1 on error
 */
FILE *ole_readdir(FILE *f) {
	int i, nLen;
	unsigned char *oleBuf;
	oleEntry *e=NULL;
	long int chainMaxLen, chainCurrent;
	
	if ( properties == NULL || propCurNumber >= propNumber || f == NULL )
		return NULL;
	oleBuf=properties + propCurNumber*PROP_BLOCK_SIZE;
	if( !rightOleType(oleBuf))
		return NULL;
	if ((e = (oleEntry*)malloc(sizeof(oleEntry))) == NULL) {
		perror("Can\'t allocate memory");
		return NULL;
	}
	e->dirPos=oleBuf;
	e->type=getOleType(oleBuf);
	e->file=f;
	e->startBlock=getlong(oleBuf,0x74);
	e->blocks=NULL;
	
	nLen=getshort(oleBuf,0x40);
	for (i=0 ; i < nLen /2; i++)
		e->name[i]=(char)oleBuf[i*2];
	e->name[i]='\0';
	propCurNumber++;
	e->length=getulong(oleBuf,0x78);
/* Read sector chain for object */
	chainMaxLen = 25;
	e->numOfBlocks = 0;
	chainCurrent = e->startBlock;
	e->isBigBlock = (e->length >= 0x1000) || !strcmp(e->name, "Root Entry");
/* 	fprintf(stderr, "e->name=%s e->length=%ld\n", e->name, e->length); */
/* 	fprintf(stderr, "e->startBlock=%ld BBD=%p\n", e->startBlock, BBD); */
	if (e->startBlock >= 0 &&
		e->length >= 0 &&
		(e->startBlock <=
		 fileLength/(e->isBigBlock ? sectorSize : shortSectorSize))) {
		if((e->blocks=malloc(chainMaxLen*sizeof(long int))) == NULL ) {
			return NULL;
		}
		while(1) {
/* 			fprintf(stderr, "chainCurrent=%ld\n", chainCurrent); */
			e->blocks[e->numOfBlocks++] = chainCurrent;
			if (e->numOfBlocks >= chainMaxLen) {
				long int *newChain;
				chainMaxLen+=25;
				if ((newChain=realloc(e->blocks,
									  chainMaxLen*sizeof(long int))) != NULL)
					e->blocks=newChain;
				else {
					perror("Properties realloc error");
					free(e->blocks);
					e->blocks=NULL;
					return NULL;
				}
			}
			if ( e->isBigBlock ) {
				chainCurrent = getlong(BBD, chainCurrent*4);
			} else if ( SBD != NULL ) {
				chainCurrent = getlong(SBD, chainCurrent*4);
			} else {
				chainCurrent=-1;
			}
			if(chainCurrent <= 0 ||
			   chainCurrent >= ( e->isBigBlock ?
								 ((bbdNumBlocks*sectorSize)/4)
								 : ((sbdNumber*shortSectorSize)/4) ) ||
			   (e->numOfBlocks >
				e->length/(e->isBigBlock ? sectorSize : shortSectorSize))) {
/*   				fprintf(stderr, "chain End=%ld\n", chainCurrent);   */
				break;
			}
		}
	}
	
	if(e->length > (e->isBigBlock ? sectorSize : shortSectorSize)*e->numOfBlocks)
		e->length = (e->isBigBlock ? sectorSize : shortSectorSize)*e->numOfBlocks;
/* 	fprintf(stderr, "READDIR: e->name=%s e->numOfBlocks=%ld length=%ld\n", */
/* 					e->name, e->numOfBlocks, e->length); */
	
	return (FILE*)e;
}

/** 
 * Open stream, which correspond to directory entry last read by
 * ole_readdir 
 * 
 * 
 * @return opaque pointer to pass to ole_read, casted to (FILE *)
 */
int ole_open(FILE *stream) {
	oleEntry *e=(oleEntry *)stream;
	if ( e->type != oleStream)
		return -2;
	
	e->ole_offset=0;
	e->file_offset= ftell(e->file);
	return 0;
}

/** 
 * 
 * 
 * @param e 
 * @param blk 
 * 
 * @return 
 */
long int calcFileBlockOffset(oleEntry *e, long int blk) {
	long int res;
	if ( e->isBigBlock ) {
		res=512+e->blocks[blk]*sectorSize;
	} else {
		long int sbdPerSector=sectorSize/shortSectorSize;
		long int sbdSecNum=e->blocks[blk]/sbdPerSector;
		long int sbdSecMod=e->blocks[blk]%sbdPerSector;
/* 		fprintf(stderr, "calcoffset: e->name=%s e->numOfBlocks=%ld length=%ld sbdSecNum=%ld rootEntry->blocks=%p\n", 
 						e->name, e->numOfBlocks, e->length, sbdSecNum, rootEntry->blocks);*/
		res=512 + rootEntry->blocks[sbdSecNum]*sectorSize + sbdSecMod*shortSectorSize;
	}
	return res;
}


/** 
 * Reads block from open ole stream interface-compatible with fread
 * 
 * @param ptr pointer to buffer for read to
 * @param size size of block
 * @param nmemb size in blocks 
 * @param stream pointer to FILE* structure
 * 
 * @return number of readed blocks
 */
size_t ole_read(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	oleEntry *e = (oleEntry*)stream;
	long int llen = size*nmemb, rread=0, i;
	long int blockNumber, modBlock, toReadBlocks, toReadBytes, bytesInBlock;
	long int ssize;				/**< Size of block */
	long int newoffset;
	unsigned char *cptr = ptr;	
	if( e->ole_offset+llen > e->length )
		llen= e->length - e->ole_offset;
	
	ssize = (e->isBigBlock ? sectorSize : shortSectorSize);
	blockNumber=e->ole_offset/ssize;
/* 	fprintf(stderr, "blockNumber=%ld e->numOfBlocks=%ld llen=%ld\n", */
/* 			blockNumber, e->numOfBlocks, llen); */
	if ( blockNumber >= e->numOfBlocks || llen <=0 )
		return 0;
	
	modBlock=e->ole_offset%ssize;
	bytesInBlock = ssize - modBlock;
	if(bytesInBlock < llen) {
		toReadBlocks = (llen-bytesInBlock)/ssize;
		toReadBytes = (llen-bytesInBlock)%ssize; 
	} else {
		toReadBlocks = toReadBytes = 0;
	}
/* 	fprintf(stderr, "llen=%ld toReadBlocks=%ld toReadBytes=%ld bytesInBlock=%ld blockNumber=%ld modBlock=%ld\n", */
/* 			llen, toReadBlocks, toReadBytes, bytesInBlock, blockNumber, modBlock); */
	newoffset = calcFileBlockOffset(e,blockNumber)+modBlock;
	if (e->file_offset != newoffset) {
		fseek(e->file, e->file_offset=newoffset, SEEK_SET);
	}
	rread=fread(ptr, 1, min(llen,bytesInBlock), e->file);
	e->file_offset += rread;
	for(i=0; i<toReadBlocks; i++) {
		int readbytes;
		blockNumber++;
		newoffset = calcFileBlockOffset(e,blockNumber);
		if (newoffset != e->file_offset);
		fseek(e->file, e->file_offset=newoffset , SEEK_SET);
		readbytes=fread(cptr+rread, 1, min(llen-rread, ssize), e->file);
		rread +=readbytes;
		e->file_offset +=readbytes;
	}
	if(toReadBytes > 0) {
		int readbytes;
		blockNumber++;
		newoffset = calcFileBlockOffset(e,blockNumber);
		fseek(e->file, e->file_offset=newoffset, SEEK_SET);
        readbytes=fread(cptr+rread, 1, toReadBytes,e ->file);
		rread +=readbytes;
		e->file_offset +=readbytes;
	}
/*	fprintf(stderr, "ole_offset=%ld rread=%ld llen=%ld\n",
	e->ole_offset, rread, llen);*/
	e->ole_offset+=rread;
	return rread;
}	

/** 
 * 
 * 
 * @param stream 
 * 
 * @return 
 */
int ole_eof(FILE *stream) {
	oleEntry *e=(oleEntry*)stream;
/*	fprintf(stderr, "EOF: e->ole_offset=%ld  e->length=%ld\n",
	e->ole_offset,  e->length);*/
	return (e->ole_offset >=  e->length);
}

/** 
 * 
 * 
 */
void ole_finish(void) {
	if ( BBD != NULL ) free(BBD);
	if ( SBD != NULL ) free(SBD);
	if ( properties != NULL ) free(properties);
	if ( rootEntry != NULL ) ole_close((FILE*)rootEntry);
	properties = SBD = BBD = NULL;
	rootEntry = NULL;
}

/** 
 * 
 * 
 * @param stream 
 * 
 * @return 
 */
int ole_close(FILE *stream) {
	oleEntry *e=(oleEntry*)stream;
	if(e == NULL)
		return -1;
	if (e->blocks != NULL)
		free(e->blocks);
	free(e);
	return 0;
}

/**
 * 
 * 
 * @param stream pointer to OLE stream structure
 * @param offset 
 * @param whence 
 * 
 * @return 
 */
int ole_seek(FILE *stream, long offset, int whence) {
	oleEntry *e=(oleEntry*)stream;
	long int new_ole_offset=0, new_file_offset;
	int ssize, modBlock, blockNumber;
	
	switch(whence) {
	case SEEK_SET:
		new_ole_offset=offset;
		break;
		
	case SEEK_CUR:
		new_ole_offset=e->ole_offset+offset;
		break;
		
	case SEEK_END:
		new_ole_offset=e->length+offset;
		break;
		
	default:
		errno=EINVAL;
		return -1;
	}
	if(new_ole_offset<0)
		new_ole_offset=0;
	if(new_ole_offset >= e->length)
		new_ole_offset=e->length;

	ssize = (e->isBigBlock ? sectorSize : shortSectorSize);
	blockNumber=new_ole_offset/ssize;
	if ( blockNumber >= e->numOfBlocks )
		return -1;
	
	modBlock=new_ole_offset%ssize;
	new_file_offset = calcFileBlockOffset(e,blockNumber)+modBlock;
	fseek(e->file, e->file_offset=new_file_offset, SEEK_SET);
	e->ole_offset=new_ole_offset;
	
	return 0;
}

/** 
 * Tell position inside OLE stream
 * 
 * @param stream pointer to OLE stream
 * 
 * @return current position inside OLE stream
 */
long ole_tell(FILE *stream) {
	oleEntry *e=(oleEntry*)stream;
	return e->ole_offset;
}


/**
 * 
 * 
 */
size_t (*catdoc_read)(void *ptr, size_t size, size_t nmemb, FILE *stream);
int (*catdoc_eof)(FILE *stream);
int (*catdoc_seek)(FILE *stream, long offset, int whence);
long (*catdoc_tell)(FILE *stream);

void set_ole_func(void) {
	catdoc_read=ole_read;
	catdoc_eof=ole_eof;
	catdoc_seek=ole_seek;
	catdoc_tell=ole_tell;
}

#ifdef feof
/* feof is macro in Turbo C, so we need a real function to assign to
 * pointer
 */ 
int my_feof(FILE *f) {
    return feof(f);
}    
#define FEOF my_feof
#else
#define FEOF feof
#endif

void set_std_func(void) {
	catdoc_read=fread;
	catdoc_eof=FEOF;
	catdoc_seek=fseek;
	catdoc_tell=ftell;
}

