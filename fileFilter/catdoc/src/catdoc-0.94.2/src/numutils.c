/*****************************************************************/
/* Utilities to convert various numeric types from the Windows   */
/* (Little endian) format to native types                        */
/*                                                               */
/* This file is part of catdoc project                           */
/* (c) Victor Wagner 1996-2003, (c) Alex Ott 2003	             */
/*****************************************************************/


/********************************************************************/
/* Reads 2-byte LSB  int from buffer at given offset platfom-indepent
 * way
 *********************************************************************/ 
unsigned int getshort(unsigned char *buffer,int offset) {
	return (unsigned short int)buffer[offset]|((unsigned short int)buffer[offset+1]<<8);
}  
/********************************************************************/
/* Reads 4-byte LSB  int from buffer at given offset almost platfom-indepent
 * way
 *********************************************************************/ 
long int getlong(unsigned char *buffer,int offset) {
	return (long)buffer[offset]|((long)buffer[offset+1]<<8L)
		|((long)buffer[offset+2]<<16L)|((long)buffer[offset+3]<<24L);
}  

unsigned long int getulong(unsigned char *buffer,int offset) {
	return (unsigned long)buffer[offset]|((unsigned long)buffer[offset+1]<<8L)
		|((unsigned long)buffer[offset+2]<<16L)|((unsigned long)buffer[offset+3]<<24L);
}  
