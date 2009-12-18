/*** mightily hacked up library version of xloadimage.c 
 *** PYW
 ***/

/* xloadimage.c:
 *
 * generic image loader for X11
 *
 * jim frost 09.27.89
 *
 * Copyright 1989, 1990, 1991 Jim Frost.
 * See included file "copyright.h" for complete copyright information.
 */

#include "copyright.h"
#include "xloadimage.h"
#include "patchlevel"
#include <signal.h>

extern double atof();
extern int _Xdebug;

/* if an image loader needs to have our display and screen, it will get
 * them from here.  this is done to keep most of the image routines
 * clean
 */

Display      *Disp= NULL;
int           Scrn= 0;

/* used for the -default option.  this is the root weave bitmap with
 * the bits in the order that xloadimage likes.
 */

#define root_weave_width 4
#define root_weave_height 4
static byte root_weave_bits[] = {
  0xe0, 0xb0, 0xd0, 0x70
};

int initImgLib()
{
  char         *dname = NULL;

  /* set up internal error handlers
   */
  signal(SIGSEGV, internalError);
  signal(SIGBUS, internalError);
  signal(SIGFPE, internalError);
  signal(SIGILL, internalError);
#if defined(_AIX) && defined(_IBMR2)
  /* the RS/6000 (AIX 3.1) has a new signal, SIGDANGER, which you get
   * when memory is exhausted.  since malloc() can overcommit, it's a good
   * idea to trap this one.
   */
  signal(SIGDANGER, memoryExhausted);
#endif



  if (! (Disp= XOpenDisplay(dname))) {
    printf("%s: Cannot open display\n", XDisplayName(dname));
    return 0;
  }
  Scrn= DefaultScreen(Disp);
  XSetErrorHandler(errorHandler);


  return 1;
}

/* the real thing
 */

int loadImg(imagename, /*image,*/ ximageinfo, width, height)
     char *imagename;
/*   Image **image;*/
     XImageInfo **ximageinfo;
     int *width, *height;
{ char         *border;
  Display      *disp;           /* display we're sending to */
  int           scrn;           /* screen we're sending to */

  Image        *dispimage;      /* image that will be sent to the display */
  Image        *newimage;       /* new image we're loading */
  Image        *tmpimage;
  XColor        xcolor;         /* color for border option */
  ImageOptions  images[MAXIMAGES + 1]; /* list of image w/ options to load */

  int           a;
  unsigned int  imagecount;     /* number of images in ImageName array */
  char         *user_geometry;	/* -geometry passed by user */
  unsigned int  dest_window;    /* window id to put image onto */
  unsigned int  do_fork;
  unsigned int  fit;
  unsigned int  fullscreen;
  unsigned int  identify;
  unsigned int  install;
  unsigned int  onroot;
  unsigned int  private_cmap;
  unsigned int  set_default;
  unsigned int  use_pixmap;
  unsigned int  verbose;
  int           visual_class;    /* user-defined visual class */
  unsigned int  winwidth, winheight; /* geometry of image */
  unsigned int  global_bright= 0; /* options which are propagated to all */
  unsigned int  global_colors= 0; /* remaining images */
  unsigned int  global_delay= 0;
  unsigned int  global_dither= 0;
  float         global_gamma= 1.0;
  unsigned int  global_normalize= 0;
  unsigned int  global_smooth= 0;
  unsigned int  global_xzoom= 0;
  unsigned int  global_yzoom= 0;

  int argc = 0;
  char **argv = 0;

  /* defaults and other initial settings.  some of these depend on what
   * our name was when invoked.
   */

  loadPathsAndExts();
  onroot= 0;
  verbose= 0;
  border= NULL;
/*  fit= 0;*/
  fit= 1;/*MAKE DEFAULT PYW*/
  fullscreen= 0;
  do_fork= 0;
  identify= 0;
  install= 0;
  private_cmap= 0;
  use_pixmap= 0;
  set_default= 0;
  dest_window= 0;
  user_geometry = NULL;
  winwidth= winheight= 0;
  visual_class= -1;

  imagecount= 0;
    images[0].name= NULL;
    images[0].atx= images[0].aty= 0;
    images[0].bright= 0;
    images[0].center= 0;
    images[0].clipx= images[0].clipy= 0;
    images[0].clipw= images[0].cliph= 0;
    images[0].colors= 0;
    images[0].delay= 0;
    images[0].dither= 0;
    images[0].gamma= 1.0;
    images[0].go_to= NULL;
    images[0].gray= 0;
    images[0].merge= 0;
    images[0].normalize= 0;
    images[0].rotate= 0;
    images[0].smooth= 0;
    images[0].xzoom= images[0].yzoom= 0;
    images[0].fg= images[0].bg= NULL;

  images[0].name = imagename;

  imagecount++;

  if (fit && (visual_class != -1)) {
    printf("-fit and -visual options are mutually exclusive (ignoring -visual)\n");
    visual_class= -1;
  }

  if (!imagecount && !set_default) /* NO-OP from here on */
    return 0;

  if (identify) {                    /* identify the named image(s) */
    for (a= 0; a < imagecount; a++)
      identifyImage(images[a].name);
    return 0;
  }

  /* start talking to the display
   */
  disp = Disp;
  scrn = Scrn;

  dispimage= NULL;

  if (onroot && (winwidth || winheight || images[0].center ||
      images[0].atx || images[0].aty || fullscreen)) {
    if (!winwidth)
	winwidth= DisplayWidth(disp, scrn);
    if (!winheight)
      winheight= DisplayHeight(disp, scrn);
    if (DefaultDepth(disp, scrn) == 1)
      dispimage= newBitImage(winwidth, winheight);
    else {
      dispimage= newRGBImage(winwidth, winheight, DefaultDepth(disp, scrn));
      dispimage->rgb.used= 1;
    }
    *(dispimage->rgb.red)= 65535;   /* default border value is white */
    *(dispimage->rgb.green)= 65535;
    *(dispimage->rgb.blue)= 65535;
    if (border) {
      XParseColor(disp, DefaultColormap(disp, scrn), border, &xcolor);
      *dispimage->rgb.red= xcolor.red;
      *dispimage->rgb.green= xcolor.green;
      *dispimage->rgb.blue= xcolor.blue;
    }

    /* bitmap needs both black and white
     */

    if (DefaultDepth(disp, scrn) == 1) {
	if (*(dispimage->rgb.red)) {
	    *(dispimage->rgb.red + 1)= 0;
	    *(dispimage->rgb.green + 1)= 0;
	    *(dispimage->rgb.blue + 1)= 0;
	}
	else {
	    *(dispimage->rgb.red + 1)= 65535;
	    *(dispimage->rgb.green + 1)= 65535;
	    *(dispimage->rgb.blue + 1)= 65535;
	}
    }
    fill(dispimage, 0, 0, winwidth, winheight, 0);
  }

  /* load in each named image
   */

  for (a= 0; a < imagecount; a++) {
    if (! (newimage= loadImage(images[a].name, verbose)))
      continue;
    if (!images[a].dither &&
	((dispimage && BITMAPP(dispimage)) || (DefaultDepth(disp, scrn) == 1)))
      images[a].dither= 1;

    /* if this is the first image and we're putting it on the root window
     * in fullscreen mode, set the zoom factors and
     * location to something reasonable.
     */

    if ((a == 0) && onroot && fullscreen &&
	(images[0].xzoom == 0.0) && (images[0].yzoom == 0.0) &&
	(images[0].atx == 0) && (images[0].aty == 0) &&
	(images[0].center == 0)) {
      XWindowAttributes wa;
      images[0].center= 1;
    
      if ((newimage->width > DisplayWidth(disp, scrn)) ||
	  (newimage->height > DisplayHeight(disp, scrn))) {
	images[0].xzoom= images[0].yzoom=
	  (newimage->width - DisplayWidth(disp, scrn) >
	   newimage->height - DisplayHeight(disp, scrn) ?
	   (float)DisplayWidth(disp, scrn) / (float)newimage->width * 100.0 :
	   (float)DisplayHeight(disp, scrn) / (float)newimage->height * 100.0);
      }
      else {
	images[0].xzoom= images[0].yzoom=
	  (DisplayWidth(disp, scrn) - newimage->width <
	   DisplayHeight(disp, scrn) - newimage->height ?
	   (float)DisplayWidth(disp, scrn) / (float)newimage->width * 100.0 :
	   (float)DisplayHeight(disp, scrn) / (float)newimage->height * 100.0);
      }
    }

    newimage= processImage(disp, scrn, newimage, &images[a], verbose);

    if (images[a].center && dispimage) {
      images[a].atx= (int)(dispimage->width - newimage->width) / 2;
      images[a].aty= (int)(dispimage->height - newimage->height) / 2;
    }
    if (dispimage) {
      if (! dispimage->title)
	dispimage->title= dupString(newimage->title);
      tmpimage= merge(dispimage, newimage, images[a].atx, images[a].aty,
		      verbose);
      if (dispimage != tmpimage) {
	freeImage(dispimage);
	dispimage= tmpimage;
      }
      freeImage(newimage);
    }
    else
      dispimage= newimage;

    *width = dispimage->width;
    *height = dispimage->height;
/*
    *image = dispimage;
*/

    *ximageinfo = 
      imageInWindow(disp, scrn, dispimage, user_geometry,
		 fullscreen, install, private_cmap, fit, use_pixmap,
		 images[a].delay, visual_class, argc, argv, verbose);

    dispimage= NULL;
  }
  return 1;
}

