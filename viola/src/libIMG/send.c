/* send.c:
 *
 * send an Image to an X pixmap
 *
 * jim frost 10.02.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xloadimage.h"

static int GotError;

static int pixmapErrorTrap(disp, pErrorEvent)
    Display	*disp;
    XErrorEvent * pErrorEvent;
{
#define MAXERRORLEN 100
    char buf[MAXERRORLEN+1];
    GotError = 1;
    XGetErrorText(disp, pErrorEvent->error_code, buf, MAXERRORLEN);
    printf("serial #%d (request code %d) Got Error %s\n",
	pErrorEvent->serial,
	pErrorEvent->request_code,
	buf);
    return(0);
}

Pixmap ximageToPixmap(disp, parent, ximageinfo)
     Display    *disp;
     Window      parent;
     XImageInfo *ximageinfo;
{
  int         (*old_handler)();
  Pixmap        pixmap;

  GotError = 0;
  old_handler = XSetErrorHandler(pixmapErrorTrap);
  XSync(disp, False);
  pixmap= XCreatePixmap(disp, parent,
			ximageinfo->ximage->width, ximageinfo->ximage->height,
			ximageinfo->depth);
  (void)XSetErrorHandler(old_handler);
  if (GotError)
    return(None);
  ximageinfo->drawable= pixmap;
  sendXImage(ximageinfo, 0, 0, 0, 0,
	     ximageinfo->ximage->width, ximageinfo->ximage->height);
  return(pixmap);
}

/* find the best pixmap depth supported by the server for a particular
 * visual and return that depth.
 *
 * this is complicated by R3's lack of XListPixmapFormats so we fake it
 * by looking at the structure ourselves.
 */

static unsigned int bitsPerPixelAtDepth(disp, scrn, depth)
     Display      *disp;
     int           scrn;
     unsigned int  depth;
{
#if 1 /* the way things are */
  unsigned int a;
/*  unsigned int n = disp->nformats;*/
    unsigned int n;
    Screen *screen = ScreenOfDisplay(disp, scrn);
    n = DefaultDepthOfScreen(screen);

  for (a= 0; a < n; a++)
    if (disp->pixmap_format[a].depth == depth)
      return(disp->pixmap_format[a].bits_per_pixel);

#else /* the way things should be */
  XPixmapFormatValues *xf;
  unsigned int nxf, a;

  xf = XListPixmapFormats(disp, &nxf);
  for (a = 0; a < nxf; a++)
    if (xf[a].depth == closest_depth)
      return(disp->pixmap_format[a].bits_per_pixel);
#endif

  /* this should never happen; if it does, we're in trouble
   */

  fprintf(stderr, "bitsPerPixelAtDepth: Can't find pixmap depth info!\n");

/*  exit(1);*/
  return 1;
}
     
XImageInfo *imageToXImage(disp, scrn, visual, ddepth, image, private_cmap, fit,
			  verbose)
     Display      *disp;
     int           scrn;
     Visual       *visual; /* visual to use */
     unsigned int  ddepth; /* depth of the visual to use */
     Image        *image;
     unsigned int  private_cmap;
     unsigned int  fit;
     unsigned int  verbose;
{ Pixel        *index, *redvalue, *greenvalue, *bluevalue;
  unsigned int  a, b, newmap, x, y, linelen, dpixlen, dbits;
  XColor        xcolor;
  XGCValues     gcv;
  XImageInfo   *ximageinfo;
  Image        *orig_image;
  int XServerInGrip = 0;

  goodImage(image, "imageToXimage");

  xcolor.flags= DoRed | DoGreen | DoBlue;
  index= redvalue= greenvalue= bluevalue= NULL;
  orig_image= image;
  ximageinfo= (XImageInfo *)lmalloc(sizeof(XImageInfo));
  ximageinfo->disp= disp;
  ximageinfo->scrn= scrn;
  ximageinfo->depth= 0;
  ximageinfo->drawable= None;
  ximageinfo->foreground= ximageinfo->background= 0;
  ximageinfo->gc= NULL;
  ximageinfo->ximage= NULL;

  /* process image based on type of visual we're sending to
   */

  switch (image->type) {
  case ITRUE:
    switch (visual->class) {
    case TrueColor:
    case DirectColor:
      /* goody goody */
      break;
    default:
      if (visual->bits_per_rgb > 1)
	image= reduce(image, depthToColors(visual->bits_per_rgb), verbose);
      else
	image= dither(image, verbose);
    }
    break;

  case IRGB:
    switch(visual->class) {
    case TrueColor:
    case DirectColor:
      /* no problem, we handle this just fine */
      break;
    default:
      if (visual->bits_per_rgb < 2)
	image= dither(image, verbose);
      break;
    }

  case IBITMAP:
    /* no processing ever needs to be done for bitmaps */
    break;
  }

  /* do color allocation
   */

  switch (visual->class) {
  case TrueColor:
  case DirectColor:
    { Pixel pixval;
      unsigned int redcolors, greencolors, bluecolors;
      unsigned int redstep, greenstep, bluestep;
      unsigned int redbottom, greenbottom, bluebottom;
      unsigned int redtop, greentop, bluetop;

      redvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      greenvalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);
      bluevalue= (Pixel *)lmalloc(sizeof(Pixel) * 256);

      if (visual == DefaultVisual(disp, scrn))
	ximageinfo->cmap= DefaultColormap(disp, scrn);
      else
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);

      retry_direct: /* tag we hit if a DirectColor allocation fails on
		     * default colormap */

      /* calculate number of distinct colors in each band
       */

      redcolors= greencolors= bluecolors= 1;
      for (pixval= 1; pixval; pixval <<= 1) {
	if (pixval & visual->red_mask)
	  redcolors <<= 1;
	if (pixval & visual->green_mask)
	  greencolors <<= 1;
	if (pixval & visual->blue_mask)
	  bluecolors <<= 1;
      }
      
      /* sanity check
       */

      if ((redcolors > visual->map_entries) ||
	  (greencolors > visual->map_entries) ||
	  (bluecolors > visual->map_entries)) {
	fprintf(stderr, "\
Warning: inconsistency in color information (this may be ugly)\n");
      }

      redstep= 256 / redcolors;
      greenstep= 256 / greencolors;
      bluestep= 256 / bluecolors;
      redbottom= greenbottom= bluebottom= 0;
      for (a= 0; a < visual->map_entries; a++) {
	if (redbottom < 256)
	  redtop= redbottom + redstep;
	if (greenbottom < 256)
	  greentop= greenbottom + greenstep;
	if (bluebottom < 256)
	  bluetop= bluebottom + bluestep;

	xcolor.red= (redtop - 1) << 8;
	xcolor.green= (greentop - 1) << 8;
	xcolor.blue= (bluetop - 1) << 8;
	if (! XAllocColor(disp, ximageinfo->cmap, &xcolor)) {

	  /* if an allocation fails for a DirectColor default visual then
	   * we should create a private colormap and try again.
	   */

	  if ((visual->class == DirectColor) &&
	      (visual == DefaultVisual(disp, scrn))) {
	    ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					      visual, AllocNone);
	    goto retry_direct;
	  }

	  /* something completely unexpected happened
	   */

	  fprintf(stderr, "\
imageToXImage: XAllocColor failed on a TrueColor/Directcolor visual\n");
	  return(NULL);
	}

	/* fill in pixel values for each band at this intensity
	 */

	while ((redbottom < 256) && (redbottom < redtop))
	  redvalue[redbottom++]= xcolor.pixel & visual->red_mask;
	while ((greenbottom < 256) && (greenbottom < greentop))
	  greenvalue[greenbottom++]= xcolor.pixel & visual->green_mask;
	while ((bluebottom < 256) && (bluebottom < bluetop))
	  bluevalue[bluebottom++]= xcolor.pixel & visual->blue_mask;
      }
    }
    break;

  default:
  retry: /* this tag is used when retrying because we couldn't get a fit */
    XServerInGrip = 0;
    index= (Pixel *)lmalloc(sizeof(Pixel) * image->rgb.used);

    /* private_cmap flag is invalid if not a dynamic visual
     */

    switch (visual->class) {
    case StaticColor:
    case StaticGray:
      private_cmap= 0;
    }

    /* get the colormap to use.
     */

    if (private_cmap) { /* user asked us to use a private cmap */
      newmap= 1;
      fit= 0;
    }
    else if ((visual == DefaultVisual(disp, scrn)) ||
	     (visual->class == StaticGray) ||
	     (visual->class == StaticColor)) {

      /* if we're using the default visual, try to alloc colors shareable.
       * otherwise we're using a static visual and should treat it
       * accordingly.
       */

      if (visual == DefaultVisual(disp, scrn))
	ximageinfo->cmap= DefaultColormap(disp, scrn);
      else
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);
      newmap= 0;

      /* allocate colors shareable (if we can)
       */

      for (a= 0; a < image->rgb.used; a++) {
	xcolor.red= *(image->rgb.red + a);
	xcolor.green= *(image->rgb.green + a);
	xcolor.blue= *(image->rgb.blue + a);
	if (! XAllocColor(disp, ximageinfo->cmap, &xcolor))
	  if ((visual->class == StaticColor) ||
	      (visual->class == StaticGray)) {
	    printf("imageToXImage: XAllocColor failed on a static visual\n");
	    return(NULL);
	  }
	  else {

	    /* we can't allocate the colors shareable so free all the colors
	     * we had allocated and create a private colormap (or fit
	     * into the default cmap if `fit' is true).
	     */

	    XFreeColors(disp, ximageinfo->cmap, index, a, 0);
	    newmap= 1;
	    break;
	  }
	*(index + a)= xcolor.pixel;
      }
    }
    else {
      newmap= 1;
      fit= 0;
    }

    if (newmap) {

      /* either create a new colormap or fit the image into the one we
       * have.  to create a new one, we create a private cmap and allocate
       * the colors writable.  fitting the colors is harder, we have to:
       *  1. grab the server so no one can goof with the colormap.
       *  2. count the available colors using XAllocColorCells.
       *  3. free the colors we just allocated.
       *  4. reduce the depth of the image to fit.
       *  5. allocate the colors again shareable.
       *  6. ungrab the server and continue on our way.
       * someone should shoot the people who designed X color allocation.
       */

      if (fit) {
	if (verbose)
	  printf("  Fitting image into default colormap\n");
/*	XGrabServer(disp);
	XServerInGrip = 1;
*/
      }
      else {
	if (verbose)
	  printf("  Using private colormap\n");

	/* create new colormap
	 */
	ximageinfo->cmap= XCreateColormap(disp, RootWindow(disp, scrn),
					  visual, AllocNone);
      }
a = 0;
#ifdef dfdfdfdssdf
      for (a= 0; a < image->rgb.used; a++) /* count entries we got */
	if (! XAllocColorCells(disp, ximageinfo->cmap, False, NULL, 0,
			       index + a, 1))
	  break;
#endif      
      if (fit) {
/*	if (a > 0)
	  XFreeColors(disp, ximageinfo->cmap, index, a, 0);
*/
	if (a < image->rgb.used) {
	  XColor ctab[256];
	  int i, dc;
	  extern Colormap theCmap;
	  extern int dispcells;
	  
	  dc = (dispcells<256) ? dispcells : 256;
	  
	  /* read in the color table */
	  for (i=0; i<dc; i++) ctab[i].pixel = i;
	  XQueryColors(disp,theCmap,ctab,dc);
/*
	  for (i=0; i<dc; i++) 
	    printf("%d: %d %d,%d,%d\n", i,ctab[i].pixel,
		   ctab[i].red, ctab[i].green, ctab[i].blue);
*/
	  if (verbose) {
	    printf("  Cannot fit into default colormap, reducing...");
	    fflush(stdout);
	  }
	  /*image= reduce(image, depthToColors(visual->bits_per_rgb), verbose);*/
	  /*image= reduce(image, 150, verbose);*/
	  if (verbose) {
	    printf("  Cannot fit into default colormap, using closest colors...");
	    fflush(stdout);
	  }
	  image= approx(image, 0);

	  /* run through the used colors.  any used color that has a pixel
	     value of 0xffff wasn't allocated.  for such colors, run through
	     the entire X colormap and pick the closest color */

	  for (i=0; i<image->rgb.used; i++) {
/*	    if (cols[i]==0xffff) { */ /* an unallocated pixel ??*/
	    int j, d, mdist, close;
	    unsigned long r,g,b;

	    mdist = 100000; close = -1;
	    r =  *(image->rgb.red + i);
	    g =  *(image->rgb.green + i);
	    b =  *(image->rgb.blue + i);
	    for (j=0; j<dc; j++) {
	      d = abs(r - (ctab[j].red)) +
		abs(g - (ctab[j].green)) +
		  abs(b - (ctab[j].blue));
/*
	      d = abs(r - (ctab[j].red>>8)) +
		abs(g - (ctab[j].green>>8)) +
		  abs(b - (ctab[j].blue>>8));
*/
	      if (d<mdist) { mdist=d; close=j; }
	    }
	    if (close> 150) {
/*	      xcolor.red= r >> 8;
	      xcolor.green= g >> 8;
	      xcolor.blue= b >> 8;
*/
	      xcolor.red= r;
	      xcolor.green= g;
	      xcolor.blue= b;
	      xcolor.flags = DoRed | DoGreen | DoBlue;
/*
	      printf("d=%d  mdist=%d close=%d\n", d, mdist, close);
	      printf("no close match, try allocating %d,%d,%d (%d %d %d)###\n",
		     r, g, b, xcolor.red, xcolor.green, xcolor.blue);
*/
/*	      if (XAllocColorCells(disp, ximageinfo->cmap, &xcolor))*/
	      if (XAllocColor(disp, theCmap, &xcolor)) {
		*(index + i)= xcolor.pixel;
/*		printf("result pixel=%d  (%d %d %d)\n",
		       xcolor.pixel, 
		       xcolor.red, xcolor.green, xcolor.blue);
*/
	      } else {
/*		printf("simply can't do it!!! Using closest color.\n");*/
		*(index + i)= ctab[close].pixel;
	      }
	    } else {
/*
#if defined(SVR4)
	      memcpy(&defs[i], &defs[close], sizeof(XColor));
#else
	      bcopy(&defs[close], &defs[i], sizeof(XColor));
#endif
*/
/*	      printf("approximating %d,%d,%d to %d,%d,%d\n",
		     r, g, b,
		     ctab[close].red, ctab[close].green, ctab[close].blue);
*/
	      *(index + i)= ctab[close].pixel;
	    }
	    /*}*/
	  }
	} /* end 'failed to pull it off' */
	
#ifdef DONTDOTHIS_HACK_HACK_XXX
	  if (verbose) {
	    printf("  Cannot fit into default colormap, dithering...");
	    fflush(stdout);
	  }
	  image= dither(image, 0);
	  if (verbose)
	    printf("done\n");
	  fit= 0;
#endif
	if (verbose)
	  printf("done\n");
	if (XServerInGrip) {
	  XServerInGrip = 0;
	  XUngrabServer(disp);
	  XFlush(disp);
	}
	break;
      }
      if (a == 0) {
	fprintf(stderr, "imageToXImage: Color allocation failed!\n");
	lfree(index);
	if (XServerInGrip) {
	  XServerInGrip = 0;
	  XUngrabServer(disp);
	  XFlush(disp);
	}
	return(NULL);
      }

      if (a < image->rgb.used)
	image= reduce(image, a, verbose);

      if (fit) {
	for (a= 0; a < image->rgb.used; a++) {
	  xcolor.red= *(image->rgb.red + a);
	  xcolor.green= *(image->rgb.green + a);
	  xcolor.blue= *(image->rgb.blue + a);

	  /* if this fails we're in trouble
	   */

	  if (! XAllocColor(disp, ximageinfo->cmap, &xcolor)) {
/*	    printf("XAllocColor failed while fitting colormap!\n");*/
	    if (XServerInGrip) {
	      XServerInGrip = 0;
	      XUngrabServer(disp);
	      XFlush(disp);
	    }
	    return(NULL);
	  }
	  *(index + a)= xcolor.pixel;
	}
	XServerInGrip = 0;
	XUngrabServer(disp);
	XFlush(disp);
      } else {
	for (b= 0; b < a; b++) {
	  xcolor.pixel= *(index + b);
	  xcolor.red= *(image->rgb.red + b);
	  xcolor.green= *(image->rgb.green + b);
	  xcolor.blue= *(image->rgb.blue + b);
	  XStoreColor(disp, ximageinfo->cmap, &xcolor);
	}
      }
    }
    break;
  }
  if (XServerInGrip) {
    XServerInGrip = 0;
    XUngrabServer(disp);
    XFlush(disp);
  }

  /* create an XImage and related colormap based on the image type
   * we have.
   */

  if (verbose) {
    printf("  Building XImage...");
    fflush(stdout);
  }

  switch (image->type) {
  case IBITMAP:
    { byte *data;

      /* we copy the data to be more consistent
       */

      data= lmalloc((image->width + 7) / 8 * image->height);
      bcopy(image->data, data, ((image->width + 7) / 8) * image->height);

      gcv.function= GXcopy;
      ximageinfo->ximage= XCreateImage(disp, visual, 1, XYBitmap,
				       0, data, image->width, image->height,
				       8, 0);
      ximageinfo->depth= ddepth;
      ximageinfo->foreground= *(index + 1);
      ximageinfo->background= *index;
      ximageinfo->ximage->bitmap_bit_order= MSBFirst;
      ximageinfo->ximage->byte_order= MSBFirst;
      break;
    }

  case IRGB:
  case ITRUE:

    /* modify image data to match visual and colormap
     */

    dbits= bitsPerPixelAtDepth(disp, scrn, ddepth);
    ximageinfo->depth= ddepth;
    dpixlen= (dbits + 7) / 8;

    switch (visual->class) {
    case DirectColor:
    case TrueColor:
      { byte *data, *destptr, *srcptr;
	Pixel pixval, newpixval;

	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0,
					  NULL, image->width, image->height,
					  8, 0);
	data= lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	destptr= data;
	srcptr= image->data;
	switch (image->type) {
	case ITRUE:
	  for (y= 0; y < image->height; y++)
	    for (x= 0; x < image->width; x++) {
	      pixval= memToVal(srcptr, image->pixlen);
	      newpixval= redvalue[TRUE_RED(pixval)] |
		greenvalue[TRUE_GREEN(pixval)] | bluevalue[TRUE_BLUE(pixval)];
	      valToMem(newpixval, destptr, dpixlen);
	      srcptr += image->pixlen;
	      destptr += dpixlen;
	    }
	  break;
	case IRGB:
	  for (y= 0; y < image->height; y++)
	    for (x= 0; x < image->width; x++) {
	      pixval= memToVal(srcptr, image->pixlen);
	      pixval= redvalue[image->rgb.red[pixval] >> 8] |
		greenvalue[image->rgb.green[pixval] >> 8] |
		  bluevalue[image->rgb.blue[pixval] >> 8];
	      valToMem(pixval, destptr, dpixlen);
	      srcptr += image->pixlen;
	      destptr += dpixlen;
	    }
	  break;
	default: /* something's broken */
	  printf("Unexpected image type for DirectColor/TrueColor visual!\n");
	  /*exit(0);*/
	  return 0;
	}
	ximageinfo->ximage->byte_order= MSBFirst; /* trust me, i know what
						   * i'm talking about */
	break;
      }
    default:

      /* only IRGB images make it this far.
       */

      /* if our XImage doesn't have modulus 8 bits per pixel, it's unclear
       * how to pack bits so we instead use an XYPixmap image.  this is
       * slower.
       */

      if (dbits % 8) {
	byte *data, *destdata, *destptr, *srcptr, mask;
	Pixel pixmask, pixval;

	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, XYPixmap, 0,
					  NULL, image->width, image->height,
					  8, 0);

	data= (byte *)lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	bzero(data, image->width * image->height * dpixlen);
	ximageinfo->ximage->bitmap_bit_order= MSBFirst;
	ximageinfo->ximage->byte_order= MSBFirst;
	linelen= (image->width + 7) / 8;
	for (a= 0; a < dbits; a++) {
	  pixmask= 1 << a;
	  destdata= data + ((dbits - a - 1) * image->height * linelen);
	  srcptr= image->data;
	  for (y= 0; y < image->height; y++) {
	    destptr= destdata + (y * linelen);
	    *destptr= 0;
	    mask= 0x80;
	    for (x= 0; x < image->width; x++) {
	      pixval= memToVal(srcptr, image->pixlen);
	      srcptr += image->pixlen;
	      if (index[pixval] & pixmask)
		*destptr |= mask;
	      mask >>= 1;
	      if (mask == 0) {
		mask= 0x80;
		destptr++;
	      }
	    }
	  }
	}
      }
      else {
	byte *data, *srcptr, *destptr;

	ximageinfo->ximage = XCreateImage(disp, visual, ddepth, ZPixmap, 0,
					  NULL, image->width, image->height,
					  8, 0);

	dpixlen= (ximageinfo->ximage->bits_per_pixel + 7) / 8;
	data= (byte *)lmalloc(image->width * image->height * dpixlen);
	ximageinfo->ximage->data= (char *)data;
	ximageinfo->ximage->byte_order= MSBFirst; /* trust me, i know what
						   * i'm talking about */
	srcptr= image->data;
	destptr= data;
	for (y= 0; y < image->height; y++)
	  for (x= 0; x < image->width; x++) {
	    valToMem(index[memToVal(srcptr, image->pixlen)], destptr, dpixlen);
	    srcptr += image->pixlen;
	    destptr += dpixlen;
	  }
      }
      break;
    }
  }

  if (verbose)
    printf("done\n");

  if (index)
    lfree((byte *)index);
  if (redvalue) {
    lfree((byte *)redvalue);
    lfree((byte *)greenvalue);
    lfree((byte *)bluevalue);
  }
  if (image != orig_image)
    freeImage(image);
  return(ximageinfo);
}

/* Given an XImage and a drawable, move a rectangle from the Ximage
 * to the drawable.
 */

void sendXImage(ximageinfo, src_x, src_y, dst_x, dst_y, w, h)
     XImageInfo  *ximageinfo;
     int          src_x, src_y, dst_x, dst_y;
     unsigned int w, h;
{
  XGCValues gcv;

  /* build and cache the GC
   */

  if (!ximageinfo->gc) {
    gcv.function= GXcopy;
    if (ximageinfo->ximage->depth == 1) {
      gcv.foreground= ximageinfo->foreground;
      gcv.background= ximageinfo->background;
      ximageinfo->gc= XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				GCFunction | GCForeground | GCBackground,
				&gcv);
    }
    else
      ximageinfo->gc= XCreateGC(ximageinfo->disp, ximageinfo->drawable,
				GCFunction, &gcv);
  }
  XPutImage(ximageinfo->disp, ximageinfo->drawable, ximageinfo->gc,
	    ximageinfo->ximage, src_x, src_y, dst_x, dst_y, w, h);
}

/* free up anything cached in the local Ximage structure.
 */

void freeXImage(image, ximageinfo)
     Image        *image;
     XImageInfo   *ximageinfo;
{
  if (ximageinfo->gc)
    XFreeGC(ximageinfo->disp, ximageinfo->gc);
  lfree((byte *)ximageinfo->ximage->data);
  ximageinfo->ximage->data= NULL;
  XDestroyImage(ximageinfo->ximage);
  lfree((byte *)ximageinfo);
}
