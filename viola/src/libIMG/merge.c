/* merge.c:
 *
 * merge two images
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "image.h"

/* if merging bitmaps they don't have to be converted to
 * 24-bit.  this saves a lot of space.
 */

static Image *bitmapToBitmap(src, dest, atx, aty, clipw, cliph, verbose)
     Image        *src, *dest;
     unsigned int  atx, aty, clipw, cliph;
     unsigned int  verbose;
{ unsigned int  destlinelen, srclinelen;
  unsigned int  deststart;
  unsigned int  flip;
  unsigned int  x, y;
  byte         *destline, *srcline;
  byte          deststartmask;
  byte          destmask, srcmask;
  byte         *destpixel, *srcpixel;

  destlinelen= (dest->width / 8) + (dest->width % 8 ? 1 : 0);
  srclinelen= (src->width / 8) + (src->width % 8 ? 1 : 0);
  destline= dest->data + (aty * destlinelen);
  srcline= src->data;
  deststart= atx / 8;
  deststartmask= 0x80 >> (atx % 8);
  flip= ((*dest->rgb.red == *(src->rgb.red + 1)) &&
	 (*dest->rgb.green == *(src->rgb.green + 1)) &&
	 (*dest->rgb.blue == *(src->rgb.blue + 1)));
  for (y= 0; y < cliph; y++) {
    destpixel= destline + deststart;
    srcpixel= srcline;
    destmask= deststartmask;
    srcmask= 0x80;
    for (x= 0; x < clipw; x++) {
      if (flip)
	if (*srcpixel & srcmask)
	  *destpixel &= ~destmask;
	else
	  *destpixel |= destmask;
      else
	if (*srcpixel & srcmask)
	  *destpixel |= destmask;
	else
	  *destpixel &= ~destmask;
      destmask >>= 1;
      srcmask >>= 1;
      if (destmask == 0) {
	destmask= 0x80;
	destpixel++;
      }
      if (srcmask == 0) {
	srcmask= 0x80;
	srcpixel++;
      }
    }
    destline += destlinelen;
    srcline += srclinelen;
  }
  if (verbose)
    printf("done\n");
  return(dest);
}

static Image *anyToTrue(src, dest, atx, aty, clipw, cliph, verbose)
     Image        *src, *dest;
     unsigned int  atx, aty, clipw, cliph;
     unsigned int  verbose;
{ Pixel         fg, bg;
  unsigned int  destlinelen, srclinelen;
  unsigned int  deststart;
  unsigned int  x, y;
  byte         *destline, *srcline;
  byte         *destpixel, *srcpixel;
  byte          srcmask;
  Pixel         pixval;

  if (!TRUEP(dest))
    dest= expand(dest);

  switch (src->type) {
  case IBITMAP:
    fg= RGB_TO_TRUE(src->rgb.red[1], src->rgb.green[1], src->rgb.blue[1]);
    bg= RGB_TO_TRUE(src->rgb.red[0], src->rgb.green[0], src->rgb.blue[0]);
    destlinelen= dest->width * dest->pixlen;
    srclinelen= (src->width / 8) + (src->width % 8 ? 1 : 0);
    destline= dest->data + (aty * destlinelen);
    srcline= src->data;
    deststart= atx * dest->pixlen;
    for (y= 0; y < cliph; y++) {
      destpixel= destline + deststart;
      srcpixel= srcline;
      srcmask= 0x80;
      for (x= 0; x < clipw; x++) {
	valToMem((*srcpixel & srcmask ? fg : bg), destpixel, 3);
	destpixel += 3;
	srcmask >>= 1;
	if (srcmask == 0) {
	  srcpixel++;
	  srcmask= 0x80;
	}
      }
      destline += destlinelen;
      srcline += srclinelen;
    }
    break;
  
  case IRGB:
    destlinelen= dest->width * dest->pixlen;
    srclinelen= src->width * src->pixlen;
    deststart= atx * dest->pixlen;
    destline= dest->data + (aty * destlinelen);
    srcline= src->data;

    for (y= 0; y < cliph; y++) {
      destpixel= destline + deststart;
      srcpixel= srcline;
      for (x= 0; x < clipw; x++) {
	pixval= memToVal(srcpixel, src->pixlen);
	*(destpixel++)= src->rgb.red[pixval] >> 8;
	*(destpixel++)= src->rgb.green[pixval] >> 8;
	*(destpixel++)= src->rgb.blue[pixval] >> 8;
	srcpixel += src->pixlen;
      }
      destline += destlinelen;
      srcline += srclinelen;
    }
    break;

  case ITRUE:
    destlinelen= dest->width * dest->pixlen;
    srclinelen= src->width * src->pixlen;
    deststart= atx * dest->pixlen;
    destline= dest->data + (aty * destlinelen);
    srcline= src->data;

    for (y= 0; y < cliph; y++) {
      destpixel= destline + deststart;
      srcpixel= srcline;
      for (x= 0; x < clipw; x++) {
	*(destpixel++)= *(srcpixel++);
	*(destpixel++)= *(srcpixel++);
	*(destpixel++)= *(srcpixel++);
      }
      destline += destlinelen;
      srcline += srclinelen;
    }
    break;
  }
  if (verbose)
    printf("done\n");
  return(dest);
}

/* put src image on dest image
 */

Image *merge(dest, src, atx, aty, verbose)
     Image        *dest;
     Image        *src;
     int           atx, aty;
     unsigned int  verbose;
{ int clipw, cliph;
  int clipped = 0;
  Image *newimage;

  goodImage(dest, "merge");
  goodImage(src, "merge");

  if (verbose) {
    printf("  Merging...");
    fflush(stdout);
  }

  /* adjust clipping of src to fit within dest
   */

  clipw= src->width;
  cliph= src->height;
  if ((atx + clipw < 0) || (aty + cliph < 0) ||
      (atx >= (int)dest->width) ||
      (aty >= (int)dest->height)) /* not on dest, ignore */
    return;

  if (atx + clipw > dest->width)
    clipw = dest->width - atx;
  if (aty + cliph > dest->height)
    cliph = dest->height - aty;

  /* extra clipping required for negative offsets
   */

  if ( atx < 0 || aty < 0 ) {
    int clipx, clipy;
 
    if ( atx < 0 ) {
      clipx = -atx;
      clipw += atx;
      atx = 0;
    }
    else
      clipx = 0;
    
    if ( aty < 0 ) {
      clipy = -aty;
      cliph += aty;
      aty = 0;
    }
    else
      clipy = 0;
    
    clipped = 1;
    src = clip(src, clipx, clipy, clipw, cliph, verbose);
  }
 
  if (BITMAPP(dest) && BITMAPP(src))
    newimage= bitmapToBitmap(src, dest, (unsigned int)atx, (unsigned int)aty,
			     clipw, cliph, verbose);
  else
    newimage= anyToTrue(src, dest, (unsigned int)atx, (unsigned int)aty,
			clipw, cliph, verbose);
  if (clipped)
    freeImage(src);
  return(newimage);
}
