#ifndef _xgifload_h
#define _xgifload_h

typedef unsigned char byte;

typedef struct _GIFImage
  {
    /* The color map, read from the GIF header */

    byte Red[256], Green[256], Blue[256];
    Boolean Allocd[256];
    Pixel cols[256];

    int  BitsPerPixel;			/* Bits per pixel, read from GIF header */
    int  CodeSize;			/* Code size, read from GIF header */
    int  Width, Height;
    Boolean Interlace, HasColormap, fastAlloc;

    byte *Raster;			/* The Raster data stream, unblocked */
    XImage *Image;                      /* The corresponding X-image */
    Pixmap Pixmap;                      /* the corresponding pixmap */
    int Refcount;                       /* Number of references to image */ 

  } GIFImage;

/* public routines */

GIFImage *GIFLoadFile();
XImage   *GIFToXImage();
void GIFFreeImage();
void GIFFreeFile();

#endif
