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

/*PYW begin*/
typedef struct ImgObj {
  Image *image;
  XImageInfo *ximageinfo;
  Window w;
} ImgObj;
ImgObj imgObj[10];
int imgObjIdx = 0;
GC gc;
/*PYW end*/

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

int           dispcells;
Colormap      theCmap;


/* the real thing
 */

main(argc, argv)
     int argc;
     char *argv[];
{ char         *border;
  char         *dname;
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

  dispcells = DisplayCells(Disp, Scrn);
  theCmap   = DefaultColormap(Disp, Scrn);

  if (argc < 2)
    usage(argv[0]);

  /* defaults and other initial settings.  some of these depend on what
   * our name was when invoked.
   */

  loadPathsAndExts();
  onroot= 0;
  verbose= 1;
  if (!strcmp(tail(argv[0]), "xview")) {
    onroot= 0;
    verbose= 1;
  }
  else if (!strcmp(tail(argv[0]), "xsetbg")) {
    onroot= 1;
    verbose= 0;
  }
  border= NULL;
  dname= NULL;
  fit= 0;
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
  for (a= 0; a < MAXIMAGES; a++) {
    images[a].name= NULL;
    images[a].atx= images[a].aty= 0;
    images[a].bright= 0;
    images[a].center= 0;
    images[a].clipx= images[a].clipy= 0;
    images[a].clipw= images[a].cliph= 0;
    images[a].colors= 0;
    images[a].delay= 0;
    images[a].dither= 0;
    images[a].gamma= 1.0;
    images[a].go_to= NULL;
    images[a].gray= 0;
    images[a].merge= 0;
    images[a].normalize= 0;
    images[a].rotate= 0;
    images[a].smooth= 0;
    images[a].xzoom= images[a].yzoom= 0;
    images[a].fg= images[a].bg= NULL;
  }
  for (a= 1; a < argc; a++) {
    switch (optionNumber(argv[a])) {
    case OPT_BADOPT:
      printf("%s: Bad option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */

    case OPT_NOTOPT:
      if (imagecount == MAXIMAGES)
	printf("%s: Too many images (ignoring)\n", argv[++a]);
      else {
	images[imagecount++].name= argv[a];
	if (imagecount < MAXIMAGES) {
	  images[imagecount].bright= global_bright;
	  images[imagecount].colors= global_colors;
	  images[imagecount].delay= global_delay;
	  images[imagecount].dither= global_dither;
	  images[imagecount].gamma= global_gamma;
	  images[imagecount].normalize= global_normalize;
	  images[imagecount].smooth= global_smooth;
	  images[imagecount].xzoom= global_xzoom;
	  images[imagecount].yzoom= global_yzoom;
	}
      }
      break;

    case OPT_SHORTOPT:
      printf("%s: Not enough characters to identify option\n", argv[a]);
      usage(argv[0]);
      /* NOTREACHED */

    /* process options global to everything
     */

    case OA_ONROOT:
      onroot= 1;
      fit= 1; /* assume -fit */
      break;

    case OA_BORDER:
      if (argv[++a])
	border= argv[a];
      break;

    case OA_DBUG:
      _Xdebug= True;
      break;

    case OA_DEFAULT:
      set_default= 1;
      break;

    case OA_DELAY:
      if (!argv[++a])
	  break;
      global_delay= atoi(argv[a]);
      if (global_delay <= 0) {
	  global_delay= 0;
	  printf("Bad argument to -delay\n");
	  break;
      }
      images[imagecount].delay= global_delay;
      break;

    case OA_DISPLAY:
      if (argv[++a])
	dname= argv[a];
      break;

    case OA_FIT:
      fit= 1;
      break;

    case OA_FORK:
      do_fork= 1;
      verbose= 0; /* background processes should be seen but not heard */
      break;

    case OA_FULLSCREEN:
      fullscreen= 1;
      break;

    case OA_GEOMETRY:
      if (argv[++a])
	user_geometry = argv[a];
      break;

    case OA_HELP:
      if (argv[++a])
	do {
	  help(argv[a++]);
	} while (argv[a]); 
      else
	help(NULL);
      exit(0);

    case OA_IDENTIFY:
      identify= 1;
      break;

    case OA_LIST:
      listImages();
      exit(0);

    case OA_INSTALL:
      install= 1;
      break;

    case OA_PATH:
      showPath();
      break;

    case OA_PIXMAP:
      use_pixmap= 1;
      break;

    case OA_PRIVATE:
      private_cmap= 1;
      break;

    case OA_QUIET:
      verbose= 0;
      break;

    case OA_SUPPORTED:
      supportedImageTypes();
      break;

    case OA_VERBOSE:
      verbose= 1;
      break;

    case OA_VER_NUM:
      version();
      break;

    case OA_VIEW:
      onroot= 0;
      break;

    case OA_VISUAL:
      if (argv[++a])
	visual_class= visualClassFromName(argv[a]);
      break;

    case OA_WINDOWID:
      if (!argv[++a])
	break;
      if (sscanf(argv[a], "0x%x", &dest_window) != 1) {
	printf("Bad argument to -windowid\n");
	usage(argv[0]);
	/* NOTREACHED */
      }
      onroot= 1; /* this means "on special root" */
      fit= 1; /* assume -fit */
      break;

    /* process options local to an image
     */

    case OA_AT:
      if (!argv[++a])
	break;
      if (sscanf(argv[a], "%d,%d",
		 &images[imagecount].atx, &images[imagecount].aty) != 2) {
	printf("Bad argument to -at\n");
	usage(argv[0]);
	/* NOTREACHED */
      }
      break;

    case OA_BACKGROUND:
      if (argv[++a])
	images[imagecount].bg= argv[a];
      break;

    case OA_BRIGHT:
      if (argv[++a]) {
	images[imagecount].bright= atoi(argv[a]);
	global_bright= images[imagecount].bright;
      }
      break;

    case OA_GAMMA:
      if (argv[++a]) {
	images[imagecount].gamma= atof(argv[a]);
	global_gamma= images[imagecount].gamma;
      }
      break;

    case OA_GOTO:
      if (argv[++a])
	images[imagecount].go_to= argv[a];
      break;

    case OA_GRAY:
      images[imagecount].gray= 1;
      break;

    case OA_CENTER:
      images[imagecount].center= 1;
      break;

    case OA_CLIP:
      if (!argv[++a])
	break;
      if (sscanf(argv[a], "%d,%d,%d,%d",
		 &images[imagecount].clipx, &images[imagecount].clipy,
		 &images[imagecount].clipw, &images[imagecount].cliph) != 4) {
	printf("Bad argument to -clip\n");
	usage(argv[0]);
	/* NOTREACHED */
      }
      break;

    case OA_COLORS:
      if (!argv[++a])
	break;
      images[imagecount].colors= atoi(argv[a]);
      if (images[imagecount].colors < 2) {
	printf("Argument to -colors is too low (ignored)\n");
	images[imagecount].colors= 0;
      }
      else if (images[imagecount].colors > 65536) {
	printf("Argument to -colors is too high (ignored)\n");
	images[imagecount].colors= 0;
      }
      global_colors= images[imagecount].colors;
      break;

    case OA_DITHER:
      images[imagecount].dither= 1;
      global_dither= 1;
      break;

    case OA_FOREGROUND:
      if (argv[++a])
	images[imagecount].fg= argv[a];
      break;

    case OA_HALFTONE:
      images[imagecount].dither= 2;
      global_dither= 2;
      break;

    case OA_IDELAY:
      if (!argv[++a])
	break;
      images[a].delay= atoi(argv[a]);
      if (images[a].delay <= 0) {
	printf("Bad argument to -idelay\n");
	images[a].delay= 0;
      }
      break;

    case OA_INVERT:
      images[imagecount].fg= "white";
      images[imagecount].bg= "black";
      break;

    case OA_MERGE:
      images[imagecount].merge= 1;
      break;

    case OA_NAME:
      if (imagecount == MAXIMAGES)
	printf("%s: Too many images (ignoring)\n", argv[++a]);
      else
	images[imagecount++].name= argv[++a];
      break;

    case OA_NEWOPTIONS:
      global_bright= 0;
      global_colors= 0;
      global_delay= 0;
      global_dither= 0;
      global_gamma= 1.0;
      global_normalize= 0;
      global_smooth= 0;
      global_xzoom= 0;
      global_yzoom= 0;
      break;

    case OA_NORMALIZE:
      images[imagecount].normalize= 1;
      global_normalize= images[imagecount].normalize;
      break;

    case OA_ROTATE:
      if (!argv[++a])
	break;
      images[imagecount].rotate = atoi(argv[a]);
      if ((images[imagecount].rotate % 90) != 0)
	{ printf("Argument to -rotate must be a multiple of 90 (ignored)\n");
	  images[imagecount].rotate = 0;
	}
      else 
	while (images[imagecount].rotate < 0)
	  images[imagecount].rotate += 360;
      break;

    case OA_SMOOTH:
      global_smooth++;
      images[imagecount].smooth= global_smooth;
      break;

    case OA_XZOOM:
      if (argv[++a]) {
	if (atoi(argv[a]) < 0) {
	  printf("Zoom argument must be positive (ignored).\n");
	  continue;
	}
	images[imagecount].xzoom= atoi(argv[a]);
	global_xzoom= images[imagecount].xzoom;
      }
      break;

    case OA_YZOOM:
      if (argv[++a]) {
	if (atoi(argv[a]) < 0) {
	  printf("Zoom argument must be positive (ignored).\n");
	  continue;
	}
	images[imagecount].yzoom= atoi(argv[a]);
	global_yzoom= images[imagecount].yzoom;
      }
      break;

    case OA_ZOOM:
      if (argv[++a]) {
	if (atoi(argv[a]) < 0) {
	  printf("Zoom argument must be positive (ignored).\n");
	  continue;
	}
	images[imagecount].xzoom= images[imagecount].yzoom= atoi(argv[a]);
	global_xzoom= global_yzoom= images[imagecount].xzoom;
      }
      break;

    default:

      /* this should not happen!
       */

      printf("%s: Internal error parsing arguments\n", argv[0]);
      exit(1);
    }
  }

  if (fit && (visual_class != -1)) {
    printf("-fit and -visual options are mutually exclusive (ignoring -visual)\n");
    visual_class= -1;
  }

  if (!imagecount && !set_default) /* NO-OP from here on */
    exit(0);

  if (identify) {                    /* identify the named image(s) */
    for (a= 0; a < imagecount; a++)
      identifyImage(images[a].name);
    exit(0);
  }

  /* start talking to the display
   */

  if (! (Disp= disp= XOpenDisplay(dname))) {
    printf("%s: Cannot open display\n", XDisplayName(dname));
    exit(1);
  }
  Scrn= scrn= DefaultScreen(disp);
  XSetErrorHandler(errorHandler);

  /* background ourselves if the user asked us to
   */

  if (do_fork)
    switch(fork()) {
    case -1:
      perror("fork");
      /* FALLTHRU */
    case 0:
      break;
    default:
      exit(0);
    }

  /* handle -default option.  this resets the colormap and loads the
   * default root weave.
   */

  if (set_default) {
    byte *old_data;

    dispimage= newBitImage(root_weave_width, root_weave_height);
    old_data= dispimage->data;
    dispimage->data= root_weave_bits;
    imageOnRoot(disp, scrn, dest_window, dispimage, 0);
    dispimage->data= old_data;
    freeImage(dispimage);
    if (!imagecount) /* all done */
      exit(0);
  }

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

    imgObj[imgObjIdx].image = dispimage;

    printf("MADE imgObj[%d]  (%d,%d)\n", 
	   imgObjIdx-1, dispimage->width, dispimage->height);

    imgObj[imgObjIdx].ximageinfo = 
      imageInWindow(disp, scrn, dispimage, user_geometry,
		 fullscreen, install, private_cmap, fit, use_pixmap,
		 images[a].delay, visual_class, argc, argv, verbose);

    imgObjIdx++;

#ifdef NONO
    /* if next image is to be merged onto this one, do it now
     */

    if (onroot || ((a < imagecount) && (images[a + 1].merge)))
      continue;

    switch(imageInWindow(disp, scrn, dispimage, user_geometry,
			 fullscreen, install, private_cmap, fit, use_pixmap,
			 images[a].delay, visual_class, argc, argv, verbose)) {
    case '\0': /* window got nuked by someone */
      XCloseDisplay(disp);
      exit(1);
    case '\003':
    case 'q':  /* user quit */
      cleanUpWindow(disp);
      XCloseDisplay(disp);
      exit(0);
    case ' ':
    case 'n':  /* next image */
      if (images[a + 1].go_to) {
	int b;
	for (b= 0; b < imagecount; b++)
	  if (!strcmp(images[b].name, images[a + 1].go_to)) {
	    a= b - 1;
	    goto next_image;
	  }
	fprintf(stderr, "Target for -goto %s was not found\n",
		images[a + 1].go_to);
      next_image:
	;
      }
      break;
    case 'p':  /* previous image */
      if (a > 0)
	a -= 2;
      else
	a--;
      break;
    }
#endif
/*    freeImage(dispimage);
*/
    dispimage= NULL;

  }
  {
    int i;
    XSetWindowAttributes attrs;
    XEvent event;
    void processEvent();
    int ForegroundPixel, BackgroundPixel;

    ForegroundPixel = BlackPixel(disp, scrn);
    BackgroundPixel = WhitePixel(disp, scrn);
    gc = DefaultGC(disp, scrn);

    for (i = 0; i < imgObjIdx; i++) {
      attrs.background_pixel = BackgroundPixel;
      attrs.border_pixel = ForegroundPixel;
      attrs.event_mask = StructureNotifyMask | KeyPressMask |
			ButtonPressMask | ButtonReleaseMask | 
			ExposureMask;

      imgObj[i].w = XCreateWindow (disp, RootWindow(disp, scrn),
				   (int)10 + (i * 50), (int)10,
				   (int)imgObj[i].image->width,
				   (int)imgObj[i].image->height,
				   (int)1, /* border thickness */
				   CopyFromParent,
				   CopyFromParent,
				   CopyFromParent,
				   CWBackPixel | CWBorderPixel | CWEventMask,
				   &attrs);
      XMapRaised(disp, imgObj[i].w);
      XFlush(disp);

    }

    printf("Press right button to exit.\n");
    while(1){
      XNextEvent(disp, &event);
      processEvent(&event);
    }
  }

/*

  if (onroot)
    imageOnRoot(disp, scrn, dest_window, dispimage, verbose);
  XCloseDisplay(disp);
  exit(0);
*/
}

void processEvent(event)
     XEvent *event;
{
  int i, x,y;
  Window w = event->xany.window;

  printf("window=x%x\n", w);

  switch(event->type) {
  case ButtonPress :
    x = ((XButtonEvent *)event)->x;
    y = ((XButtonEvent *)event)->y;
    printf("x=%d, y=%d\n", x, y);

    switch(((XButtonPressedEvent *)event)->button) {
    case 1: /* left button */
      printf("left button press\n");
      for (i = 0; i < imgObjIdx; i++) {
	if (imgObj[i].w == w) {
	  printf("DESTROYING ImgObj[%d]\n", i);
	  freeXImage(imgObj[i].image, imgObj[i].ximageinfo);
	  freeImage(imgObj[i].image);
	  XDestroyWindow(Disp, imgObj[i].w);
	  imgObj[i].w = 0;
	  imgObj[i].image = 0;
	  imgObj[i].ximageinfo = 0;
	}
      }
      break;

    case 2: /* middle button */
      printf("middle button press\n");
      break;

    case 3: /* right button */
      printf("right button press: \n");
      break;
    }
    break;

  case Expose :
    {
      int x1, y1, x2, y2;

      printf("expose\n");

      for (i = 0; i < imgObjIdx; i++) {
	if (imgObj[i].w == w) {
	  printf("EXPOSURE FOR ImgObj[%d]\n", i);
	  XPutImage(Disp, w, gc, imgObj[i].ximageinfo->ximage, 0, 0, 
		    0, 0, imgObj[i].image->width, imgObj[i].image->height);
	  XFlush(Disp);
	}
      }
    }
  }
}

