
#ifndef _GENERATE_THUMBNAIL_H_
#define _GENERATE_THUMBNAIL_H_


unsigned char* generate_thumbnail( const void *image, const size_t size, size_t *new_size );
unsigned char* free_thumbnail_memory( unsigned char *blob );
unsigned char* generate_pdf_thumbnail( const void *document, const size_t size, size_t *new_size );

#endif	// _GENERATE_THUMBNAIL_H_
