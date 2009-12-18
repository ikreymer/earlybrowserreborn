#include <stdio.h>
#include <math.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xmu/Xmu.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include "xpm.h"

typedef struct _XpmIcon {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
}        XpmIcon;
static XpmIcon view;

#define TRUE 1
#define FALSE 0

Display *display = NULL;
int screen_num;
Screen *screen;
int ForegroundPixel, BackgroundPixel;
int InvertPlane;
GC gc;
GC gc_dash;

#define FG_COLOR "green"
#define BG_COLOR "blue"

Colormap colormap;
XColor fgcolor, bgcolor, exactcolor;

char buff[100];
char keybuf[10];
XEvent event;
finish = FALSE;

XImage image = {
  0, 0,                        /* width, height */
  0, XYBitmap, NULL,           /* xoffset, format, data */
  LSBFirst, 8,                 /* byte-order, bitmap-unit */
  LSBFirst, 8, 1               /* bitmap-bit-order, bitmap-pad, depth */
  };

Window w;

main()
{
  XSetWindowAttributes attrs;
  char *server = NULL;
  int ncolors;

  /* XPM */
    unsigned int numsymbols = 0;
    XpmColorSymbol symbols[10];
    unsigned long valuemask = 0;
  int ErrorStatus;
  FILE bitmapfd;
  char bitmapStr[1000];

  if((display = XOpenDisplay(server)) == NULL) {
    printf("Can't open display\n");
    exit(1);
  }
  screen_num = DefaultScreen(display);
  screen = ScreenOfDisplay(display, screen_num);
  gc = DefaultGC (display, screen_num);

  ForegroundPixel = BlackPixel(display, screen_num);
  BackgroundPixel = WhitePixel(display, screen_num);
  InvertPlane = ForegroundPixel ^ BackgroundPixel;

  /* see sec 3.1
    Visual visual = XDefaultVisualOfScreen(screen);
    */

  /* color stuff */
  ncolors = (int) pow(2.0, ((double) DefaultDepthOfScreen(screen)));
  fprintf(stderr, "There are %d colors available.\n", ncolors);
  fprintf(stderr, "There are %d colormap cells.\n", XCellsOfScreen(screen));

  colormap = XDefaultColormapOfScreen(screen);
/*
  XParseColor(display, colormap, FG_COLOR, &fgcolor);
  XParseColor(display, colormap, BG_COLOR, &bgcolor);
*/
  if (!XAllocNamedColor(display, colormap, FG_COLOR, &fgcolor, &exactcolor)) {
    fprintf(stderr, "failed to allocate fg '%s' color\n", FG_COLOR);
  }
  if (!XAllocNamedColor(display, colormap, BG_COLOR, &bgcolor, &exactcolor)) {
    fprintf(stderr, "failed to allocate bg '%s' color\n", BG_COLOR);
  }
  attrs.background_pixel = BackgroundPixel;
  attrs.border_pixel = ForegroundPixel;
  attrs.event_mask =/* NULL;*/StructureNotifyMask | KeyPressMask;
  attrs.cursor = NULL;

  w = XCreateWindow (display, RootWindow(display, screen_num),
		     (int)10, (int)10,
		     (int)200, (int)100,
		     (int)1, /* border thickness */
		     CopyFromParent, CopyFromParent, CopyFromParent,
		     CWBackPixel | CWBorderPixel | CWEventMask | CWCursor,
		     &attrs);

strcpy(bitmapStr, "\
/* XPM */\n\
static char * dot [] = {\n\
\"16 16 4 1\",\n\
\"       c green\",\n\
\".      c blue\",\n\
\"X      c red\",\n\
\"o      c white\",\n\
\"                \",\n\
\"                \",\n\
\"                \",\n\
\"    .......     \",\n\
\"   .........    \",\n\
\"   .........    \",\n\
\"  ......XXXXXXXX\",\n\
\"  ......X....ooX\",\n\
\"oo......X....ooX\",\n\
\"ooo.....X...oooX\",\n\
\"ooo.....X...oooX\",\n\
\"oooo....X..ooooX\",\n\
\"ooooooo.XooooooX\",\n\
\"ooooooooXooooooX\",\n\
\"ooooooooXooooooX\",\n\
\"ooooooooXXXXXXXX\"};\n\
");

#ifdef SYSV
  bitmapfd._cnt = strlen(bitmapStr);
  bitmapfd._base = bitmapfd._ptr = bitmapStr;
  bitmapfd._flag = stdin->_flag;
  bitmapfd._file = _NFILE;
  _bufend(&bitmapfd) = bitmapStr + bitmapfd._cnt;
#else
  bitmapfd._cnt = bitmapfd._bufsiz = strlen(bitmapStr);
  bitmapfd._base = bitmapStr;
  bitmapfd._ptr = bitmapStr;
  bitmapfd._flag = stdin->_flag;
  bitmapfd._file = _IOREAD | _IOSTRG;
#endif
    view.attributes.colorsymbols = symbols;
    view.attributes.numsymbols = numsymbols;
    view.attributes.valuemask = valuemask;
	view.attributes.valuemask |= XpmReturnInfos;
	view.attributes.valuemask |= XpmReturnPixels;
	ErrorStatus = XpmReadFileToPixmap(display, w, bitmapfd,
					  &view.pixmap, &view.mask,
					  &view.attributes);

  if (!gc_dash) {
    char dash_list[2];
    XGCValues gcv;
    XGCValues values;

    values.subwindow_mode = IncludeInferiors;
    values.foreground = 1;
    values.background = 1;
    values.dashes = 1;
    values.dash_offset = 0;
    values.line_style = LineOnOffDash;
    gc_dash = XCreateGC(display, w,
			GCForeground | GCBackground | GCDashOffset |
			GCDashList | GCLineStyle | GCSubwindowMode,
			&values);

    gcv.foreground = 1;
    gcv.background = 2;
    gcv.dash_offset = 1;
    gcv.line_style = LineOnOffDash;
    XChangeGC(display, gc_dash,
	      GCForeground | GCBackground | GCLineStyle, &gcv);

    dash_list[0] = 1;
    dash_list[1] = 1;
    XSetDashes(display, gc_dash, 2, dash_list, 2);
  }

  XMapRaised(display, w);
  XFlush(display);

  XSelectInput(display, w, KeyPressMask | ButtonPressMask | 
	       ButtonReleaseMask | PointerMotionMask |
	       ExposureMask | VisibilityChangeMask);

  printf("Press right button to exit.\n");
  while(!finish){
    XNextEvent(display, &event);
    processEvent(&event);
  }
}

char eventChar(e)
     XEvent *e;
{
  KeySym keysym;
  long ret;
  char keybuf[10];

  ret = (long)XLookupString (e, keybuf, sizeof keybuf, &keysym, NULL);
  printf("keysym=%ld, key[0]='%c' ret=%ld\n", (long)keysym, keybuf[0], ret); 
  printf("  IsModifierKey=%d", IsModifierKey(keysym));
  printf("  IsCursorKey=%d", IsCursorKey(keysym));
  return(keybuf[0]);
}

processEvent(event)
     XEvent *event;
{
  int x,y;
  Window w = event->xany.window;

  printf("window=x%x\n", w);

  switch(event->type) {
  case ButtonPress :
    x = ((XButtonEvent *)event)->x;
    y = ((XButtonEvent *)event)->y;
    printf("x=%d, y=%d\n", x, y);

    switch(((XButtonPressedEvent *)event)->button) {
    case 1: /* left button */
      printf("left button press: fgcolor\n");
      XSetState(display, gc, fgcolor.pixel, 0L, GXcopy, AllPlanes);
      XFillRectangle(display, w, gc, x, y, 10,10);
      XDrawLine(display, w, gc_dash, 20,20, x, y);
      break;

    case 2: /* middle button */
      printf("middle button press: bgcolor\n");
      XSetState(display, gc, bgcolor.pixel, 0L, GXcopy, AllPlanes);
      XFillRectangle(display, w, gc, x, y, 10,10);
      XDrawLine(display, w, gc, 10,40, x, y);
      break;

    case 3: /* right button */
      printf("right button press: \n");
      finish = TRUE;
      break;
    }
    break;

  case KeyPress :
    {
      char c;
      c = eventChar(event);
      
      switch(c) {
      case '1':

	break;
      }
    }
    break;
  case Expose :
    {
      int x1, y1, x2, y2;

      printf("expose\n");
	XSetWindowBackgroundPixmap(display, w, view.pixmap);
      x1 = 0; y1 = 0;
      x2 = 50; y2 = 20;
      XDrawLine(display, w, gc_dash, x1, y1, x2, y1);
      XDrawLine(display, w, gc_dash, x1, y2, x2, y2);
      XDrawLine(display, w, gc_dash, x1, y1, x1, y2);
      XDrawLine(display, w, gc_dash, x2, y1, x2, y2);
      XFlush(display);
    }
  }
}






