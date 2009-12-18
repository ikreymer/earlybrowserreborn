/*
 * glib_x.h
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
/*#include <X11/Xmu/Xmu.h>*/
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>

#define eventWindow(e) 		((e).xany.window)
#define eventType(e)		((e).type)
#define mouseX(e) 		(int)(((XButtonEvent *)&e)->x)
#define mouseY(e) 		(int)(((XButtonEvent *)&e)->y)
#define rootX(e) 		(int)(((XButtonEvent *)&e)->x_root)
#define rootY(e) 		(int)(((XButtonEvent *)&e)->y_root)
#define buttonNumber(e)		(int)(((XButtonEvent *)&e)->button)
#define eventRegionX(e) 	((e).eventSpecific.exposure.regionX)
#define eventExposeX(e) 	(int)((e).xexpose.x)
#define eventExposeY(e) 	(int)((e).xexpose.y)
#define eventExposeWidth(e)     (int)((e).xexpose.width)
#define eventExposeHeight(e)    (int)((e).xexpose.height)
#define eventExposeCount(e)     (int)((e).xexpose.count)
#define eventExposeSerial(e)    (int)((e).xexpose.serial)
#define eventRegionY(e)		((e).eventSpecific.exposure.regionY)
#define eventRegionHeight(e)    ((e).eventSpecific.exposure.height)
#define eventRegionWidth(e)	((e).eventSpecific.exposure.width)

typedef struct EventMaskNameStruct {
	int	mask;
	char 	*name;
} EventMaskNameStruct;

extern EventMaskNameStruct eventMaskName[];

#define ANYWINDOW ((Window) NULL)

#define FG_COLOR "white"
#define BG_COLOR "black"
#define BD_COLOR "white"
#define CR_COLOR "white"

typedef int InkColor;

#ifdef BEFORE_SHARING_COLOR_INFO_________________________________________
typedef struct ColorStruct {
	char *name;
	XColor xcolor;
} ColorStruct;

typedef struct ColorInfoStruct {
	ColorStruct bg;   /* background color */
	ColorStruct fg;   /* foreground color */
	ColorStruct bd;   /* border color */
	ColorStruct cr;   /* cursor color */
} ColorInfo;
#endif BEFORE_SHARING_COLOR_INFO_________________________________________

typedef struct ColorStruct {
	XColor xcolor;
	char *name;
	int refc;
} ColorStruct;

typedef struct ColorInfoStruct {
	ColorStruct *bg;   /* background color */
	ColorStruct *fg;   /* foreground color */
	ColorStruct *bd;   /* border color */
	ColorStruct *cr;   /* cursor color */
} ColorInfo;

#define BDColorName(visual) ((colorInfo)->bd ? (colorInfo)->bd->name : NULL)
#define BGColorName(visual) ((colorInfo)->bg ? (colorInfo)->bg->name : NULL)
#define CRColorName(visual) ((colorInfo)->cr ? (colorInfo)->cr->name : NULL)
#define FGColorName(visual) ((colorInfo)->fg ? (colorInfo)->fg->name : NULL)

extern char *startUpDisplay;
extern int startUpGeom_x;
extern int startUpGeom_y;
extern int startUpGeom_width;
extern int startUpGeom_height;
extern XEvent e;
extern char keybuf[];
extern Display *display;
extern int screen_number;
extern Screen *screen;
int displayWidth;
int displayHeight;

extern int FGPixel, BGPixel, BDPixel, CRPixel, InvertPixel;
extern GC gc;
extern GC gc_fg;
extern GC gc_bg;
extern GC gc_bg_default;
extern GC gc_bg_lighter;
extern GC gc_bg_darker;
extern GC gc_bg_darkest;
extern GC gc_bd;
extern GC gc_cr;
extern GC gc_invert;
extern GC gc_invert_dash;
extern GC gc_copy;
extern GC gc_copy0;
extern GC gc_or;
extern GC gc_dash;
extern GC gc_mesh;
extern GC gc_subWindow;
extern int mouseCursorIsBusy;
extern short *bits;
extern int bits_width, bits_height;
extern int x_hot_spot, y_hot_spot;
extern Window rootWindow;
extern int pen_x, pen_y;
extern InkColor currentInkColor;
extern InkColor saveInkColor;
extern int screenDPI;
extern int sync_event;

extern XFontStruct   *mfinfo;

extern int numberOfFontIDs;

#define menuPixmap_width 8
#define menuPixmap_height 8
extern Pixmap menuPixmap;

#include "violaIcon.xbm"
extern Pixmap violaPixmap;

#include "dunselIcon.xbm"
extern Pixmap dunselPixmap;

#define togglePixmap_width 16
#define togglePixmap_height 16
extern Pixmap togglePixmap_button0;
extern Pixmap togglePixmap_button1;
extern Pixmap togglePixmap_disc0;
extern Pixmap togglePixmap_disc1;
extern Pixmap togglePixmap_check0;
extern Pixmap togglePixmap_check1;

/****************************************************************************
 * procedures
 */
int init_fonts();
void free_fonts();

#define GLClearWindow(w) XClearWindow(display, w)
#define GLLowerWindow(w) XLowerWindow(display, w)
#define GLRaiseWindow(w) XRaiseWindow(display, w)

/* window 
 */
Window 	GLOpenWindow();
void 	GLCloseWindow();
int 	GLDestroyWindow();
int 	GLWindowSize();
int 	GLSetWindowName();
int	GLSetIconName();
Window 	GLQueryWindow();

/* cursor 
 */
void 	GLRootPosition();
Window 	GLQueryMouse();
void 	GLChangeToBusyMouseCursor();
void 	GLChangeToNormalMouseCursor();

/* line 
 */
int 	GLPaintLine();

#define GLDrawLine(w, x1, y1, x2, y2)\
	GLPaintLine(w, gc_fg, x1, y1, x2, y2)

#define GLEraseLine(w, x1, y1, x2, y2)\
	GLPaintLine(w, gc_bg, x1, y1, x2, y2)

#define GLInvertLine(w, x1, y1, x2, y2)\
	GLPaintLine(w, gc_invert, x1, y1, x2, y2)

/* rectangle
 */
int 	GLPaintRect();

#define GLDrawRect(w, x1, y1, x2, y2)\
	GLPaintRect(w, gc_fg, x1, y1, x2, y2)

#define GLEraseRect(w, x1, y1, x2, y2)\
	GLPaintRect(w, gc_bg, x1, y1, x2, y2)

#define GLInvertRect(w, x1, y1, x2, y2)\
	GLPaintRect(w, gc_invert, x1, y1, x2, y2)

int 	GLPaintRect();

#define GLDrawFillRect(w, x1, y1, x2, y2)\
	GLPaintFillRect(w, gc_fg, x1, y1, x2, y2)

#define GLEraseFillRect(w, x1, y1, x2, y2)\
	GLPaintFillRect(w, gc_bg, x1, y1, x2, y2)

#define GLInvertFillRect(w, x1, y1, x2, y2)\
	GLPaintFillRect(w, gc_invert, x1, y1, x2, y2)

/* circle/oval
 */
int 	GLPaintOval();

#define	GLDrawOval(w, x, y, width, height)\
	GLPaintOval(w, gc_fg, x, y, width, height)

#define	GLEraseOval(w, x, y, width, height)\
	GLPaintOval(w, gc_bg, x, y, width, height)

#define	GLInvertOval(w, x, y, width, height)\
	GLPaintOval(w, gc_invert, x, y, width, height)

int 	GLPaintFillOval();

int	GLPaintFillOvalDegree();

int	GLPaintFillArc();

#define	GLDrawFillOval(w, x, y, width, height)\
	GLPaintFillOval(w, gc_fg, x, y, width, height)

#define	GLEraseFillOval(w, x, y, width, height)\
	GLPaintFillOval(w, gc_bg, x, y, width, height)

#define	GLInvertFillOval(w, x, y, width, height)\
	GLPaintFillOval(w, gc_invert, x, y, width, height)

/* text
 */
int 	GLDefineNewFont();
int 	GLUpdateFontInfo();
int 	GLTextWidth();
int 	GLTextHeight();

int 	GLPaintText();
int 	GLPaintTextLength();

#define GLDrawText(w, fontID, x0, y0, str)\
	GLPaintText(w, gc_fg, fontID, x0, y0, str)
#define GLDrawTextLength(w, fontID, x0, y0, str, length)\
	GLPaintTextLength(w, gc_fg, fontID, x0, y0, length);

#define GLEraseText(w, fontID, x0, y0, str)\
	GLPaintText(w, gc_bg, fontID, x0, y0, str)
#define GLEraseTextLength(w, fontID, x0, y0, str, length)\
	GLPaintTextLength(w, gc_bg, fontID, x0, y0, str, length)

#define GLInvertText(w, fontID, x0, y0, str)\
	GLPaintText(w, gc_invert, fontID, x0, y0, str)
#define GLInvertTextLength(w, fontID, x0, y0, str, length)\
	GLPaintTextLength(w, gc_invert, fontID, x0, y0, str, length)

/* bitmap
 */
Pixmap 	GLMakeXBMFromASCII();
Pixmap 	GLMakeXPMFromASCII();
int 	GLDisplayXBMFromASCII();
int 	GLDisplayXPMFromASCII();
int 	GLDisplayXBM();
int 	GLDisplayXBMBG();
int 	GLDisplayXPM();
int 	GLDisplayXPMBG();
int 	GLFreeXBM();
int 	GLFreeXPM();
XImage 	*GLReadBitmapImage();

/* widget drawings
 */
void 	GLDrawBorder();
void 	GLEraseBorder();
int 	GLDrawRubberFrame();
void 	GLDrawScrollBarH();
void 	GLDrawScrollBarV();
void 	GLDrawDial();

/* widget geometry
 */
void 	GLUpdateGeometry();
void 	GLUpdatePosition();
void 	GLUpdateSize();

/* widget color 
 */
int 	GLSetColor();
int 	GLSetBDColor();
int 	GLSetBGColor();
int 	GLSetFGColor();
int 	GLSetCRColor();
char 	*GLGetBGColor();
char 	*GLGetFGColor();
char 	*GLGetBDColor();
char 	*GLGetCRColor();
void 	GLUpdateColorReference();
ColorInfo *GLPrepareObjColor();
void 	GLDumpColorRef();

#ifdef USE_XGIF_PACKAGE
#include "../libGIF/xgif.h"
XImage *GLGIFResize();
int GLGIFDraw();
#endif

/* cut buffer 
 */
void GLClearSelection();
int GLGetSelection();
int GLSetSelection();

/* access to X resources
 */
char *GLGetResource();

void freeGLibResources();

ColorStruct *getColorStruct();
ColorStruct *addColorStructByString();
ColorStruct *addColorName();
void releaseColor();
void releaseColorByString();

int GLShadeColor(/*orig_red, orig_green, orig_blue, shade*/);
