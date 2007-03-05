
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include <wand/MagickWand.h>

#include "generate_thumbnail.h"

#define gspath "/usr/bin/gs"

#define converttemptemplate "/tmp/generateThumbnail"

#define DoMagick(wand,X) \
    { \
	if ((X) == MagickFalse) \
	    { \
	        char		*description; \
	        ExceptionType	severity; \
		\
	        description = MagickGetException(wand, &severity); \
	        fprintf(stderr, "generateThumbnail: %s %s %ld %s\n", GetMagickModule(), description); \
	        description = (char *) MagickRelinquishMemory(description); \
		\
	        pixel_wand = DestroyPixelWand( pixel_wand ); \
		magick_wand_canvas = DestroyMagickWand( magick_wand_canvas ); \
		magick_wand = DestroyMagickWand( magick_wand ); \
		MagickWandTerminus(); \
	        return NULL; \
	    } \
    }


unsigned char* generate_pdf_thumbnail( const void *document, const size_t size, size_t *new_size ) {

	void *image;
	void *thumbnail;
	FILE *fp;
	char command[512];
	char documentfile[PATH_MAX];
	char imagefile[PATH_MAX];
	struct stat inode;      // lager en struktur for fstat å returnere.

	//tmpfilename = mktemp("/tmp/generateThumbnail_XXXXXX"); //make a unique temporary file name


	
	snprintf(documentfile,sizeof(documentfile),"%s.pdf",converttemptemplate);
	snprintf(imagefile,sizeof(imagefile),"%s.png",converttemptemplate);
	
	if ((fp = fopen(documentfile,"wb")) == NULL) {
		printf(documentfile);
		return NULL;
	}
	fwrite(document,1,size,fp);
	fclose(fp);

	snprintf(command,sizeof(command),"%s -dBATCH -dFirstPage=1 -dLastPage=1 -sDEVICE=png256 -dNOPAUSE -dSAFER -sOutputFile=%s %s",gspath,imagefile,documentfile);

	printf("runing %s\n",command);
	system(command);


	if ((fp = fopen(imagefile,"rb")) == NULL) {
                printf(imagefile);
                return NULL;
        }
	fstat(fileno(fp),&inode);
	image = malloc(inode.st_size);
	fread(image,1,inode.st_size,fp);
	fclose(fp);
    
	thumbnail = generate_thumbnail(image,inode.st_size,new_size);

	free(image);
	unlink(documentfile);
	unlink(imagefile);

	return thumbnail;
}

unsigned char* generate_thumbnail( const void *image, const size_t size, size_t *new_size )
{
    MagickWand		*magick_wand, *magick_wand_canvas;
    PixelWand		*pixel_wand;
    unsigned long	cols, rows, ncols, nrows;
    unsigned char	*new_blob;

    // Initialize MagickWand:
    MagickWandGenesis();

    magick_wand = NewMagickWand();
    magick_wand_canvas = NewMagickWand();
    pixel_wand = NewPixelWand();



    // Read image from memory:
    DoMagick( magick_wand, MagickReadImageBlob( magick_wand, image, size ) );

    // If input consists of several images, we'll only take the first one:
    MagickResetIterator( magick_wand );
    MagickSetFirstIterator( magick_wand );

    // Get image size:
    cols = MagickGetImageWidth( magick_wand );
    rows = MagickGetImageHeight( magick_wand );

    printf("Image size: %ix%i\n", (int)cols, (int)rows);

    if (cols==0 || rows==0)
	{
	    fprintf(stderr, "generateThumbnail: Warning, imagesize==0, skipping image...\n");

            pixel_wand = DestroyPixelWand( pixel_wand );
	    magick_wand_canvas = DestroyMagickWand( magick_wand_canvas );
    	    magick_wand = DestroyMagickWand( magick_wand );
    	    MagickWandTerminus();
	    return NULL;
	}

    if (cols >= rows)
	{
	    ncols = 98;
	    nrows = (rows*98)/cols;
	}
    else
	{
	    nrows = 98;
	    ncols = (cols*98)/rows;
	}

    // Convert the image into a thumbnail:
    DoMagick( magick_wand, MagickThumbnailImage( magick_wand, ncols, nrows ) );


    PixelSetColor( pixel_wand, "#8f7f6f" );

    DoMagick( magick_wand, MagickBorderImage( magick_wand, pixel_wand, 1, 1 ) );

    DoMagick( magick_wand, MagickSetImageBackgroundColor( magick_wand, pixel_wand ) );


    PixelSetColor( pixel_wand, "white" );
    DoMagick( magick_wand_canvas, MagickNewImage( magick_wand_canvas, 100, 100, pixel_wand ) );

    DoMagick( magick_wand_canvas, MagickCompositeImage( magick_wand_canvas, magick_wand, OverCompositeOp, 49-(ncols/2), 49-(nrows/2) ) );

//    DoMagick( magick_wand_canvas, MagickWriteImage( magick_wand_canvas, "thumbnail.png" ) );
    DoMagick( magick_wand_canvas, MagickSetImageFormat( magick_wand_canvas, "PNG" ) );
    new_blob = MagickGetImageBlob( magick_wand_canvas, new_size );
// unsigned char *MagickGetImageBlob(MagickWand *wand,size_t *length)

    pixel_wand = DestroyPixelWand( pixel_wand );
    magick_wand = DestroyMagickWand( magick_wand );
    magick_wand_canvas = DestroyMagickWand( magick_wand_canvas );
    MagickWandTerminus();

    return new_blob;
}

unsigned char* free_thumbnail_memory( unsigned char *blob )
{
    return (unsigned char*)MagickRelinquishMemory( (void*)blob );
}

