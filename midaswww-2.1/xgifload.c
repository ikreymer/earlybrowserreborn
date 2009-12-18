/*
 *
 * Modified for use with MidasWWW .... December 1992
 * Tony Johnson (Tony_Johnson@Slac.Stanford.Edu)
 *
 * This file now contains multiple routines:
 *    GIFLoadFile   ... loads a gif file and returns a GIF structure
 *    GIFFreeFile   ... frees a GIF structure
 *    GIFToXImage   ... converts a GIF structure to an XImage
 *    GIFFreeImage  ... free colors associated with a GIF image
 *
 * xgifload.c  -  based strongly on...
 *
 * gif2ras.c - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
 *
 * Copyright (c) 1988, 1989 by Patrick J. Naughton
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <Mrm/MrmAppl.h>                        /* Motif Toolkit and MRM */
#include "xgifload.h"  
#include "midaswww.h"

#define NEXTBYTE (GetChar(data))
#define READCODE (ReadCode(&context,gifImage)) 
#define IMAGESEP 0x2c
#define INTERLACEMASK 0x40
#define COLORMAPMASK 0x80

char *id87 = "GIF87a";
char *id89 = "GIF89a";

typedef struct _Context
  {

    int XC, YC;			/* Output X and Y coords of current pixel */
    int Pass;			/* Used by output routine if interlaced pic */
    byte *Image;		/* The result array */
    int BytesPerImageCell;	/* Bytes per cell in output Image */
    int BytesPerScanline;	/* bytes per scanline in output Raster */
    struct _used {
      int count;
      int index;
    } used[256];
    int CodeSize;
    int numused;
    int BitOffset;
    int ReadMask;		/* Code AND mask for current code size */

  } Context; 

/*
 * Function used to sort _used structure
 */
static int sortfun(a,b)
struct _used *a,*b;
{
  return b->count - a->count;
}
/*
 * The fastAlloc routines provide fast color allocation for pseudoColor
 * visuals 
 */

static unsigned long pixels[256];
static XColor colors[256];
static int npixels = 0;
static int useCount[256];
static Display *fastdpy;
static Colormap fastcmap;
static XColor ToBeStored[256];
static int nToBeStored;
static int maxpixels; 

#define MINIMUM(a,b) ((a)<(b)?(a):(b))

static Boolean fastAllocInit(dpy,theVisual,cmap,dispcells)
Display *dpy;
Colormap cmap;
Visual *theVisual;
int dispcells;
{
  int i;
  Status rc;

  if (npixels > 0) return TRUE;
 
  if (theVisual->class != PseudoColor &&
      theVisual->class != GrayScale ) return FALSE; 

  maxpixels  = MINIMUM(dispcells/2,256);
  rc = XAllocColorCells(dpy,cmap,FALSE,NULL,0,pixels,maxpixels);

  if (rc == 0) return FALSE;

  for (i=0; i < maxpixels; i++) useCount[i] = 0;

  fastdpy = dpy;
  fastcmap = cmap;
  npixels = maxpixels;
  nToBeStored = 0;

  return TRUE;
}
static void fastAllocColor(x)
XColor *x;
{
  int j, d, mdist, close, free;
  unsigned long freej;
 
  /* run through the entire X colormap and pick the closest color */
         	
  mdist = 1000000;   close = -1; free = -1;

  for (j=0; j<npixels; j++) {
    int i = pixels[j];
    if (useCount[i]) {
      d = abs(x->red   - colors[i].red  ) +
          abs(x->green - colors[i].green) +
          abs(x->blue  - colors[i].blue );
      if (d<mdist) { mdist=d; close=i; }
    }
    else free = i;
  } 
  if (mdist > 20 && free < 0 && npixels < maxpixels) {
    int rc = XAllocColorCells(fastdpy,fastcmap,FALSE,NULL,0,&pixels[npixels],maxpixels-npixels);
    if (rc != 0) { 
      free = pixels[npixels];
      npixels = maxpixels;
    }
  }
  if (mdist <= 20 || free < 0) { /* close enough?? */
    useCount[close]++;
    x->pixel = close;
  }  
  else {
    useCount[free]++;
    x->pixel = free;
    colors[free] = *x;
    ToBeStored[nToBeStored++] = *x; 
  }
  return;
}
static void fastAllocStore()
{
  XStoreColors(fastdpy,fastcmap,ToBeStored,nToBeStored);
  nToBeStored = 0;
}
static void fastAllocFree(free)
Pixel free;
{
  useCount[free]--;
}    
void fastAllocFreeUnusedColors()
{
  Pixel free[256];
  int nFree = 0, j;

  for (j=0; j<npixels; )
    { 
      if (useCount[pixels[j]] == 0) 
        { 
          free[nFree++] = pixels[j]; 
          pixels[j] = pixels[--npixels]; 
        }
      else j++;
    }

  if (nFree) XFreeColors(fastdpy,fastcmap,free,nFree,0);
}
/* Fetch the next code from the Raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location in the Raster array as a BIT Offset.  We compute
 * the byte Offset into the Raster array by dividing this by 8, pick up
 * three bytes, compute the bit Offset into our 24-bit chunk, shift to
 * bring the desired code to the bottom, then mask it off and return it. 
 */


static int ReadCode(context,gifImage)
Context *context;
GIFImage *gifImage;
{
    int RawCode, ByteOffset;

    ByteOffset = context->BitOffset / 8; 
    RawCode = gifImage->Raster[ByteOffset] + (0x100 * gifImage->Raster[ByteOffset + 1]); 
    if (context->CodeSize >= 8) RawCode += (0x10000 * gifImage->Raster[ByteOffset + 2]);
    RawCode >>= (context->BitOffset % 8); 
    context->BitOffset += context->CodeSize; 
    return(RawCode & context->ReadMask);
}


static void AddToPixel(context,gifImage,Index)
Context *context;
GIFImage *gifImage;
int Index;
{
    if (context->YC < gifImage->Height)
      *(context->Image +  context->YC * context->BytesPerScanline + context->XC) = Index;

    if (context->used[Index].count++ == 0) context->numused++; 

/* Update the X-coordinate, and if it overflows, update the Y-coordinate */

    if (++context->XC == gifImage->Width) {

/* If a non-interlaced picture, just increment YC to the next scan line. 
 * If it's interlaced, deal with the interlace as described in the GIF
 * spec.  Put the decoded scan line out to the screen if we haven't gone
 * past the bottom of it
 */

	context->XC = 0;
	if (!gifImage->Interlace) context->YC++;
	else {
	    switch (context->Pass) {
		case 0:
		    context->YC += 8;
		    if (context->YC >= gifImage->Height) {
			context->Pass++;
			context->YC = 4;
		    }
		break;
		case 1:
		    context->YC += 8;
		    if (context->YC >= gifImage->Height) {
			context->Pass++;
			context->YC = 2;
		    }
		break;
		case 2:
		    context->YC += 4;
		    if (context->YC >= gifImage->Height) {
			context->Pass++;
			context->YC = 1;
		    }
		break;
		case 3:
		    context->YC += 2;
		break;
		default:
		break;
	    }
	}
    }
}

/*****************************/
GIFImage *GIFLoadFile(parent,message,GetChar,data)
Widget parent;
char *message;
int (*GetChar)();
DataSource *data;
/*****************************/
{
    register byte  ch, ch1;
    register byte  *ptr1;
    int filesize;
    register int   i;
    char test[6];
    GIFImage *gifImage;
    int Rwidth, Rheight;	/* screen dimensions */
    Boolean HasColormap;
    int LeftOfs, TopOfs;	/* image offset */
    int BitsPerPixel;		/* Bits per pixel, read from GIF header */
    int Background;		/* background color */
    int ColorMapSize;
    int gif89; 
    int Aspect;
    int block;
    byte *Raster;
	
    for (i=0; i<6; i++) test[i] = NEXTBYTE; 

    if      (!strncmp(test, id87, 6)) gif89 = 0;
    else if (!strncmp(test, id89, 6)) gif89 = 1;
    else 
      {
	strcpy(message,"File is not in GIF format.");
        return NULL;
      }

/* Get variables from the GIF screen descriptor */

    ch = NEXTBYTE;
    Rwidth = ch + 0x100 * NEXTBYTE;	/* screen dimensions... not used. */
    ch = NEXTBYTE;
    Rheight = ch + 0x100 * NEXTBYTE;

    ch = NEXTBYTE;
    HasColormap = ((ch & COLORMAPMASK) ? True : False);

    BitsPerPixel = (ch & 7) + 1;

    Background = NEXTBYTE;	/* background color... not used. */
    Aspect = NEXTBYTE;

    if (Aspect)		/* supposed to be NULL for gif87 */
      {
        if (!gif89) 
          {
   	    strcpy(message,"Corrupt GIF file (bad screen descriptor).");
            return NULL;
          }
      }

/* Create the GIFImage */  

    gifImage = XtNew(GIFImage);   
    gifImage->HasColormap  = HasColormap;
    gifImage->BitsPerPixel = BitsPerPixel;
    gifImage->Image = NULL;
    gifImage->Pixmap = NULL;
    gifImage->Refcount = 0;

    ColorMapSize = 1 << BitsPerPixel; 

/* Read in global colormap. */

    if (HasColormap) {

	for (i = 0; i < ColorMapSize; i++) {
	    gifImage->Red[i]   = NEXTBYTE;
	    gifImage->Green[i] = NEXTBYTE;
	    gifImage->Blue[i]  = NEXTBYTE;
	    }
        }

    else {  /* no colormap in GIF file */

        fprintf(stderr,"GIF warning!  no colortable in this file.  Winging it.\n");
        }

/* Check for image seperator */

    
    for (block = NEXTBYTE; block != IMAGESEP; block = NEXTBYTE)
      {  
        int sbsize;
        NEXTBYTE; 
	/* read (and ignore) data sub-blocks */
	do 
          {
	    int j = 0;
            sbsize = NEXTBYTE; 
            if (data->eof)
              {
   	         strcpy(message,"Corrupt GIF file (image separator).");
                 goto  cleanup;
              }
	    while (j<sbsize) { NEXTBYTE;  j++; }
	  } while (sbsize);
      } 

/* Now read in values from the image descriptor */

    ch = NEXTBYTE;
    LeftOfs = ch + 0x100 * NEXTBYTE;
    ch = NEXTBYTE;
    TopOfs = ch + 0x100 * NEXTBYTE;
    ch = NEXTBYTE;
    gifImage->Width = ch + 0x100 * NEXTBYTE;
    ch = NEXTBYTE;
    gifImage->Height = ch + 0x100 * NEXTBYTE;
    gifImage->Interlace = ((NEXTBYTE & INTERLACEMASK) ? True : False);
    gifImage->CodeSize = NEXTBYTE;

/* Note that I ignore the possible existence of a local color map.
 * I'm told there aren't many files around that use them, and the spec
 * says it's defined for future use.  This could lead to an error
 * reading some files. 
 */

/* Read the Raster data.  Here we just transpose it from the GIF array
 * to the Raster array, turning it from a series of blocks into one long
 * data stream, which makes life much easier for ReadCode().
 */

    
    filesize = gifImage->Width * gifImage->Height * (gifImage->CodeSize+1)/8;
    Raster = (byte *) XtMalloc(filesize);

    ptr1 = Raster;
    do {
        ch = ch1 = NEXTBYTE;

        if (data->eof)
          { 
            XtFree((char *) Raster);
	    strcpy(message,"Premature end-of-file found in GIF file.");
            goto  cleanup;
	  }
	if (ptr1 - Raster + ch > filesize)
	  {
            int nused = ptr1 - Raster; 
            Raster = (byte *) XtRealloc((char *) Raster, filesize += filesize/4);
            ptr1 = Raster + nused;
          }
	while (ch--) *ptr1++ = NEXTBYTE;
    } while(ch1);

    if (filesize - (ptr1 - Raster) > 1000) Raster = (byte *) XtRealloc((char *) Raster,(ptr1 - Raster)); 
    gifImage->Raster = Raster;    
    return gifImage;

cleanup:
  
    XtFree((char *)gifImage);
    return NULL;
}
XImage *GIFToXImage(w,gifImage,message)
Widget w;
GIFImage *gifImage;
char *message;
{
    register byte  ch, ch1;
    register int   i;
    int numcols;
    int *thiserr = NULL , *nexterr = NULL, *temperr;
    int fs_direction;
    int threshval;

#define FS_SCALE 1024
#define HALF_FS_SCALE 512

    int BitsPerPixel = gifImage->BitsPerPixel;
    int Width  = gifImage->Width; 
    int Height = gifImage->Height;
    unsigned long *cols = gifImage->cols;
    byte *Raster = gifImage->Raster;	/* The Raster data stream, unblocked */
    char *data = NULL;

    Widget   s;
    Display  *theDisp   = XtDisplay(w); 
    int      theScreen  = XScreenNumberOfScreen(XtScreen(w));
    int      dispcells  = DisplayCells(theDisp, theScreen);
    Colormap theCmap;
    Visual   *theVisual = DefaultVisual(theDisp,theScreen);
    int      theDepth;
    XImage   *theImage;

    int 
    ColorMapSize,		/* number of colors */
    InitCodeSize,		/* Starting code size, used during Clear */
    Code,			/* Value returned by ReadCode */
    MaxCode,			/* limiting value for current code size */
    ClearCode,			/* GIF clear code */
    EOFCode,			/* GIF end-of-information code */
    CurCode, OldCode, InCode,	/* Decompressor variables */
    FirstFree,			/* First free code, generated per GIF spec */
    FreeCode,			/* Decompressor, next free slot in hash table */
    FinChar,			/* Decompressor variable */
    BitMask;			/* AND mask for data size */

    int OutCount = 0;		/* Decompressor output 'stack count' */
    int OutCode[1025];          /* An output array used by the decompressor */
    int Prefix[4096];           /* The hash table used by the decompressor */
    int Suffix[4096];
    
    Context context;

    Arg arglist[10];
    int n=0;

    if (gifImage->Image)
      {
        gifImage->Refcount++;
        return gifImage->Image;
      }

    XtSetArg(arglist[n], XtNdepth,    &theDepth);  n++;
    XtSetArg(arglist[n], XtNcolormap, &theCmap);   n++;
    XtGetValues(w,arglist,n); 

    if (theDepth <= 4) 
      {
        int col;
        /* prepare for Floyd-Steinberg halftone conversion */
 
        thiserr = (int *) XtMalloc((gifImage->Width + 2) * sizeof(int));
        nexterr = (int *) XtMalloc((gifImage->Width + 2) * sizeof(int));

        srand((int) (time(0) ^ getpid ()));
        for (col = 0 ; col < gifImage->Width + 2 ; col++)
          thiserr[col] = (rand() % FS_SCALE - HALF_FS_SCALE ) / 4; 

        fs_direction = 1;
        threshval = HALF_FS_SCALE; 
      }
    


    context.XC = 0;
    context.YC = 0;
    context.Pass = 0;  
    context.CodeSize = gifImage->CodeSize; 
    
    context.BitOffset = 0; 

/* Start reading the Raster data. First we get the intial code size
 * and compute decompressor constant values, based on this code size.
 */
    numcols = ColorMapSize = 1 << BitsPerPixel;
    BitMask = ColorMapSize - 1;

    ClearCode = (1 << context.CodeSize);
    EOFCode = ClearCode + 1;
    FreeCode = FirstFree = ClearCode + 2;

    for (i=0 ; i<numcols ; i++) 
      { 
        context.used[i].count = 0;
        context.used[i].index = i;   
        gifImage->Allocd[i] = FALSE;
      }
/* The GIF spec has it that the code size is the code size used to
 * compute the above values is the code size given in the file, but the
 * code size used in compression/decompression is the code size given in
 * the file plus one. (thus the ++).
 */

    context.CodeSize++;
    InitCodeSize = context.CodeSize;
    MaxCode = (1 << context.CodeSize);
    context.ReadMask = MaxCode - 1;

/* Allocate the X Image */

/* WARNING: I've only tested this on 8 and 16 depth DISPLAY, who knows
   if it works elsewhere???....let me know if you improve it please.
   
   TonyJ
*/   
   

    if      (theDepth<=8 ) context.BytesPerImageCell = 1;
    else if (theDepth<=16) context.BytesPerImageCell = 2;
    else                   context.BytesPerImageCell = 4;

    context.Image = (byte *) XtMalloc(context.BytesPerImageCell*Width*Height); 
    data = XtMalloc(context.BytesPerImageCell*Width*Height); 

    theImage = XCreateImage(theDisp,theVisual,theDepth,ZPixmap,0,data,
                            Width,Height,context.BytesPerImageCell*8,Width * context.BytesPerImageCell);
    if (!theImage)
      {
        XtFree((char *) context.Image);
	strcpy(message,"Unable to create image for GIF file.");
        goto cleanup; 
      }

    context.BytesPerScanline = Width;

/* Decompress the file, continuing until you see the GIF EOF code.
 * One obvious enhancement is to add checking for corrupt files here.
 */

    Code = READCODE;
    while (Code != EOFCode) {

/* Clear code sets everything back to its initial value, then reads the
 * immediately subsequent code as uncompressed data.
 */

	if (Code == ClearCode) {
	    context.CodeSize = InitCodeSize;
	    MaxCode = (1 << context.CodeSize);
	    context.ReadMask = MaxCode - 1;
	    FreeCode = FirstFree;
	    CurCode = OldCode = Code = READCODE;
	    FinChar = CurCode & BitMask;
	    AddToPixel(&context,gifImage,FinChar);
	}
	else {

/* If not a clear code, then must be data: save same as CurCode and InCode */

	    CurCode = InCode = Code;

/* If greater or equal to FreeCode, not in the hash table yet;
 * repeat the last character decoded
 */

	    if (CurCode >= FreeCode) {
		CurCode = OldCode;
		OutCode[OutCount++] = FinChar;
	    }

/* Unless this code is raw data, pursue the chain pointed to by CurCode
 * through the hash table to its end; each code in the chain puts its
 * associated output code on the output queue.
 */

	    while (CurCode > BitMask) {
		if (OutCount > 1024) {
		    strcpy(message,"Corrupt GIF file (OutCount)!.");
        	    XDestroyImage(theImage);
                    theImage = NULL;
        	    goto cleanup;
                    }
		OutCode[OutCount++] = Suffix[CurCode];
		CurCode = Prefix[CurCode];
	    }

/* The last code in the chain is treated as raw data. */

	    FinChar = CurCode & BitMask;
	    OutCode[OutCount++] = FinChar;

/* Now we put the data out to the Output routine.
 * It's been stacked LIFO, so deal with it that way...
 */

	    for (i = OutCount - 1; i >= 0; i--)
		AddToPixel(&context,gifImage,OutCode[i]);
	    OutCount = 0;

/* Build the hash table on-the-fly. No table is stored in the file. */

	    Prefix[FreeCode] = OldCode;
	    Suffix[FreeCode] = FinChar;
	    OldCode = InCode;

/* Point to the next slot in the table.  If we exceed the current
 * MaxCode value, increment the code size unless it's already 12.  If it
 * is, do nothing: the next code decompressed better be CLEAR
 */

	    FreeCode++;
	    if (FreeCode >= MaxCode) {
		if (context.CodeSize < 12) {
		    context.CodeSize++;
		    MaxCode *= 2;
		    context.ReadMask = (1 << context.CodeSize) - 1;
		}
	    }
	}
	Code = READCODE;
    }

    if (theDepth <= 4)
      {
        byte *bP = context.Image - gifImage->Width - 1;
        int row;
        for (row = 0; row < gifImage->Height; row++)
          {
            int col, limitcol;
            for (col = 0 ; col < gifImage->Width + 2 ; col++) nexterr[col] = 0;
            
            if (fs_direction)
              {
                col = 0;
                limitcol = gifImage->Width;
                bP += gifImage->Width + 1; 
              }
            else
              {
                col = gifImage->Width - 1;
                limitcol = -1;
                bP += gifImage->Width - 1; 
              } 
            do 
              {
                char c = *bP;
                int gray = .299 * gifImage->Red[c] + .587 * gifImage->Green[c] + .114 * gifImage->Blue[c];  
                int sum = (gray * FS_SCALE) / 256 + thiserr[col+1];
                if (sum >= threshval)
                  {
                     XPutPixel(theImage,col,row,WhitePixel(theDisp,theScreen));
                     sum -= threshval + HALF_FS_SCALE;
                  }
                else XPutPixel(theImage,col,row,BlackPixel(theDisp,theScreen));

                if (fs_direction)
                  {
                    thiserr[col + 2] += (sum * 7) / 16;
                    nexterr[col    ] += (sum * 3) / 16;
                    nexterr[col + 1] += (sum * 5) / 16;
                    nexterr[col + 2] += (sum    ) / 16;
                    ++col;
                    ++bP; 
                  }
                else
                  {
                    thiserr[col    ] += (sum * 7) / 16;
                    nexterr[col + 2] += (sum * 3) / 16;
                    nexterr[col + 1] += (sum * 5) / 16;
                    nexterr[col    ] += (sum    ) / 16; 
                    --col;
                    --bP;
                  }  
              } while (col != limitcol);
            temperr = thiserr;
            thiserr = nexterr;
            nexterr = temperr;
            fs_direction = ! fs_direction;   
          }  
      }

    /* we've got the picture loaded, we know what colors are needed. get 'em */
    /* no need to allocate any colors if no colormap in GIF file */

    else if (gifImage->HasColormap) 
      {
        Boolean no_more_colors = FALSE;
	register int   i,j,k;
        XColor  defs[256];
     	XColor  ctab[256];
       	int    dc = (dispcells<256) ? dispcells : 256;       
        byte *ptr;
        Boolean fastAlloc = gifImage->fastAlloc = fastAllocInit(theDisp,theVisual,theCmap,dispcells); 

        /*
         * The first colors that we try to allocate are more likely to be allocated 
         * exactly, so sort by frequency of use 
         */

        qsort(context.used,numcols,sizeof(struct _used),sortfun);       

	/* Allocate the X colors for this picture */

        for (k=j=0; k<numcols; k++) {
          i = context.used[k].index;
          if (context.used[k].count) {
            defs[i].red   = gifImage->Red[i]<<8;
            defs[i].green = gifImage->Green[i]<<8;
            defs[i].blue  = gifImage->Blue[i]<<8;
            defs[i].flags = DoRed | DoGreen | DoBlue;
            if (!no_more_colors) {

              if (fastAlloc) { 
                 fastAllocColor(&defs[i]);
                 gifImage->Allocd[i] = TRUE;
                 cols[i] = defs[i].pixel;
              }
              else if (XAllocColor(theDisp,theCmap,&defs[i])) {  
                gifImage->Allocd[i] = TRUE; 
                cols[i] = defs[i].pixel;
              }
              else {
                /*
                 * In the interest of speed we assume that if
                 * one allocation fails all subsequent ones will
                 * fail too.
                 */  
                no_more_colors = TRUE;
                /* read in the color table */
                for (j=0; j<dc; j++) ctab[j].pixel = j;
                XQueryColors(theDisp,theCmap,ctab,dc);
              }
            }

            if (no_more_colors) { 
              Boolean skip; 
              int k, d, mdist, close;
 
              /* run through the entire X colormap and pick the closest color */
         	
              mdist = 1000000;   close = -1;

              for (j=0; j<dc; j++) {
                d = abs(defs[i].red   - ctab[j].red  ) +
                    abs(defs[i].green - ctab[j].green) +
                    abs(defs[i].blue  - ctab[j].blue );
                if (d<mdist) { mdist=d; close=j; }
              }
              if (close<0)
                {
 	          strcpy(message,"Can't allocate colors for GIF file.");
         	  XDestroyImage(theImage);
                  theImage = NULL;
                  goto cleanup;
                }           
              defs[i] = ctab[close];
 
              /*  
               * We attempt to alloocate colors here to simplify the task of freeing
               * them ,and to make sure no one else changes the ones we are using.    
               * This might fail if the color is allocated read/write by some-other
               * application, in fact we shouldn't really be using such pixels anyway
               * but it probably doesn't REALLY matter. 
               */

              skip = FALSE;  
              cols[i] = defs[i].pixel;
              for (j = 0 ; j < i ; j++)
                if (cols[j] == cols[i]) skip = TRUE;

              if (!skip && XAllocColor(theDisp,theCmap,&defs[i])) {
                cols[i] = defs[i].pixel; /* just in case it changed */
                gifImage->Allocd[i] = TRUE;
              }
            } 
          }
        }
 
        ptr = context.Image;
        for (i=0; i<Height; i++)
          for (j=0; j<Width; j++,ptr++) 
            XPutPixel(theImage,j,i,cols[*ptr]);

       if (fastAlloc) fastAllocStore();    
     } 
   gifImage->Refcount = 1;
   gifImage->Image = theImage;

cleanup: 
   XtFree((char *) thiserr);
   XtFree((char *) nexterr);
   XtFree((char *) context.Image);
   return theImage;
}
/* Catch the alloc error when there is not enough resources for the
 * pixmap.  
 */
static Boolean alloc_error;
static XErrorHandler oldhandler;

static int catch_alloc (dpy, err)
Display *dpy;
XErrorEvent *err;
{
    if (err->error_code == BadAlloc) {
  	alloc_error = True;
    }
    if (alloc_error) return 0;
    oldhandler(dpy, err);
}
Pixmap GIFToPixmap(w,gifImage,message)
Widget w;
GIFImage *gifImage;
char *message;
{
    Pixmap pixmap = gifImage->Pixmap;
    XImage *image = gifImage->Image;
    int Width  = gifImage->Width; 
    int Height = gifImage->Height;
    Display  *theDisp   = XtDisplay(w); 
    Window   theWind    = XtWindow(w);
    GC gc = DefaultGCOfScreen(XtScreen(w));
    int	theDepth;
 
    Arg arglist[10];
    int n=0;

    if (pixmap) return pixmap;
    if (!image) image = GIFToXImage(w,gifImage,message);
    if (!image) return NULL;

    XtSetArg(arglist[n], XtNdepth,    &theDepth);  n++;
    XtGetValues(w,arglist,n);

    XSync(XtDisplay(w), False);  /* Get to known state */
    oldhandler = XSetErrorHandler(catch_alloc);
    alloc_error = False;
    pixmap = XCreatePixmap(theDisp,theWind,Width,Height,theDepth);
    XSync(XtDisplay(w), False);  /* Force the error */
    if (alloc_error && pixmap) 
      {
	XFreePixmap(theDisp,pixmap);
  	XSync(XtDisplay(w), False);  /* Force the error */
	pixmap = NULL;
      }
    oldhandler = XSetErrorHandler(oldhandler);
    if (pixmap)
      {
        gifImage->Pixmap = pixmap;
        XPutImage(theDisp, pixmap, gc, image, 0,0,0,0,Width,Height);
      }
    return pixmap;
}
void GIFFreeImage(w,gifImage)
Widget w;
GIFImage *gifImage; 
{
  int i,j;
  Pixel cols[256];
  int numcols = 1 << gifImage->BitsPerPixel; 
  Colormap theCmap;
  Arg arglist[10];
  int n=0;

  if (!gifImage->Image) return;
  if (--gifImage->Refcount) return;

  XDestroyImage(gifImage->Image);
  if (gifImage->Pixmap) XFreePixmap(XtDisplay(w),gifImage->Pixmap);

  if (gifImage->fastAlloc) 
    {
      for (i=0 ; i < numcols; i++) 
        if (gifImage->Allocd[i]) 
          fastAllocFree(gifImage->cols[i]);
    } 
  else {
    for (j=0 , i=0 ; i < numcols; i++) 
      if (gifImage->Allocd[i]){
        cols[j++] = gifImage->cols[i];
      } 

    XtSetArg(arglist[n], XtNcolormap, &theCmap);   n++;
    XtGetValues(w,arglist,n);

    XFreeColors(XtDisplay(w),theCmap,cols,j,0);
  }
  gifImage->Image = NULL;
  gifImage->Pixmap = NULL;
}
void GIFFreeFile(gifImage)
GIFImage *gifImage; 
{
  XtFree((char *) gifImage->Raster);
  XtFree((char *) gifImage);
}
