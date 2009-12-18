/*LINTLIBRARY*/
/*
 * xpa - X Pull-Aside menu
 *
 * David Harrison
 * University of California, Berkeley
 * 1989
 *
 * Native pull-aside menus for X11.  The inspiration for these menus
 * come from those developed for the Xerox Dandelion under InterLisp.
 * Pull-aside menus support arbitrarily deep menus by allowing the
 * user to pull out new levels from the side of a menu choice.
 * See xpa.doc for details about the interface.
 */
#ifndef OCTTOOLS_COPYRIGHT_H
#define OCTTOOLS_COPYRIGHT_H
/*
 * Oct Tools Distribution 3.0
 *
 * Copyright (c) 1988, 1989, Regents of the University of California.
 * All rights reserved.
 *
 * Use and copying of this software and preparation of derivative works
 * based upon this software are permitted.  However, any distribution of
 * this software or derivative works must include the above copyright
 * notice.
 *
 * This software is made available AS IS, and neither the Electronics
 * Research Laboratory or the University of California make any
 * warranty about the software, its performance or its conformity to
 * any specification.
 *
 * Suggestions, comments, or improvements are welcome and should be
 * addressed to:
 *
 *   octtools@eros.berkeley.edu
 *   ..!ucbvax!eros!octtools
 */

#if !defined(lint) && !defined(SABER)
static char octtools_copyright[] = "Copyright (c) 1988, 1989, Regents of the University of California.  All rights reserved.";
#endif
#endif 

/*#include "defhell.h"*/
#include <stddef.h>
#include <stdio.h>
#include "xpa.h"

/* there is a better way, right? */
#ifdef hpux
#include "X11/Xos.h"
#include "X11/Xutil.h"
#include "X11/cursorfont.h"
#else
#include "X11/Xos.h"
#include "X11/Xutil.h"
#include "X11/cursorfont.h"
#endif

#define XPA_BORDER	1
#define XPA_PB_W	pb_sym_w
#define XPA_PB_H	pb_sym_h
#define XPA_PB_B	0
#define XPA_PB_SYM	XC_sb_right_arrow
#define XPA_PB_FONT	"cursor"
#ifdef NOTDEF
#define XPA_PB_SYM	'>'
#define XPA_PB_FONT	"6x10"
#endif
#define XPA_PB_PAD	1
#define XPA_HORPAD	4
#define XPA_VERPAD	1
#define XPA_ACBUFL	10

#define XPA_MEM(type)		(type *) malloc(sizeof(type))
#define XPA_MEMX(type, num)	(type *) malloc((unsigned) (sizeof(type) * (num)))
#define XPA_SAVE(str)		strcpy(XPA_MEMX(char, strlen(str)+1), str)

#define XPA_FREE(ptr)		free((char *) ptr)
/*
extern char *malloc();
extern char *realloc();
extern void free();
extern void exit();
*/
static XContext xpa_xcon = (XContext) 0;
static void pb_draw();

/* Size of pullbox symbol - set in set_pb_sym() */
static XFontStruct *pb_sym_font;
static int pb_sym_w;
static int pb_sym_h;
static XCharStruct pb_sym_metric;



typedef enum xpa_type_defn {
    PANE_T, ITEM_T, PB_T, TITLE_T
} xpa_type;

/*
 * All xpa_data structures share the first two fields as described below:
 */
typedef struct xpa_any_defn {
    xpa_type type;		/* Any type        */
    Window window;		/* Primary window  */
} xpa_any;

/*
 * Panes have the following associated data
 */
typedef struct xpa_pane_defn {
    xpa_type type;		/* PANE_T             */
    Window pane_win;		/* Window itself      */
    int level;			/* Level in hierarchy */
} xpa_pane;

/*
 * Items in a menu have the following associated data
 */
typedef struct xpa_item_defn {
    xpa_type type;		/* ITEM_T               */
    Window item_win;		/* Window for item      */
    char *name;			/* Name to display      */
    char key_buf[XPA_ACBUFL];	/* Accelerator key      */
    int level;			/* Level in hierarchy   */
    int idx;			/* Index in pane        */
    int iw, ih;			/* Size of item         */
    int baseline;		/* Offset to baseline   */
    int fromtop;		/* Distance from top    */
    int key_off;		/* End of accelerator   */
    Window sub_pane;		/* Posted subpane       */
    struct xpa_pb_defn *pb;	/* Pull box info        */
} xpa_item;

/*
 * Pull-boxes have the following information
 */
typedef struct xpa_pb_defn {
    xpa_type type;		/* PB_T            */
    Window pb_win;		/* Window for pb   */
    xpa_item *item;		/* Associated item */
    int height;			/* Height of box   */
} xpa_pb;

/*
 * Pane titles have the following information
 */
typedef struct xpa_title_defn {
    xpa_type type;		/* TITLE_T              */
    Window title_win;		/* Window for title     */
    char *title_str;		/* String for title     */
    XCharStruct t_metric;	/* Title string metric  */
    int fullwidth;		/* Full width of window */
    int level;			/* Level in heirarchy   */
} xpa_title;

/*
 * Generic data stored in hash table
 */
typedef union xpa_data_defn {
    xpa_any  any;
    xpa_pane pane;
    xpa_item item;
    xpa_pb   pb;
    xpa_title title;
} xpa_data;

/*
 * Current menu state structure
 */
typedef struct xpa_ms_defn {
    int x, y;			/* Pane location */
    Window pane;		/* Pane itself   */
    xpa_item *item;		/* Selected item */
} xpa_ms;

/*
 * xpa_menu is a hidden pointer to the following structure
 */
typedef struct xpa_intern_defn {
    Display *disp;		/* Connection to X server 	*/
    struct xpa_context_defn *allwins; 	/* Linked contexts for disposal */
    int depth;			/* Depth of menu          	*/
    xpa_ms *state;		/* Menu state array (depth)     */
    xpa_item **cur_items;	/* Currently selected items 	*/
    xpa_appearance *attr;	/* Local menu attributes  	*/
    Cursor cursor;		/* Menu cursor                  */
    Window top;			/* Top level window       	*/
    int press_flag;		/* Whether to react on press    */
    int width, height;		/*XXX*/
} xpa_intern;

/*
 * The following is stored in a hash table indexed by window.
 * A general dispatch mechanism calls the function
 * when an event happens in the window and passes the data
 * as an argument.
 */

typedef struct xpa_context_defn {
    xpa_data *data;		/* Call specific data     */
    int (*callback)();		/* Callback function      */
    xpa_intern *intern; 	/* Internal menu info */
    struct xpa_context_defn *next; /* Linked for disposal */
} xpa_context;



/*
 * Error handling
 */

char *xpa_error(code)
int code;			/* Error code */
/* Returns a textual description for `code'. */
{
    switch (code) {
    case XPA_NOMEM:
	return "out of memory";
    case XPA_BADEVT:
	return "received unexpected X event";
    case XPA_NOIMPL:
	return "function not implemented";
    case XPA_TYPE:
	return "type conflict";
    case XPA_GRAB:
	return "cannot grab mouse";
    case XPA_NOTEVT:
	return "encountered event not destined for xpa";
    case XPA_BADSTAT:
	return "bad status returned from X function";
    case XPA_NOFONT:
	return "unable to load necessary font";
    case XPA_ZAPP:
	return "null display attribute structure";
    case XPA_BADCLR:
	return "bad color specification";
    default:
	return "unknown error";
    }
}


static void def_err_func(code)
int code;			/* Error code */
/* Default error handler - prints a message and exits */
{
    (void) fprintf(stderr, "Fatal error in xpa: %s\n", xpa_error(code));
    abort();
}

static void (*err_func)() = def_err_func;

static void xpa_raise(code)
int code;			/* Error code */
{
    if (err_func) {
	(*err_func)(code);
    }
}

void xpa_set_error(func)
void (*func)();			/* New error function */
/*
 * Sets the current xpa error function to `func'.  The function
 * should have the following form:
 *   void func(code)
 *   int code;
 * `code' will be one of the error codes given in xpa.h.  A
 * textual representation is available by calling xpa_error().
 * Specifying zero resets the function to the default.
 */
{
    if (func) {
	err_func = func;
    } else {
	err_func = def_err_func;
    }
}



#define SPACE_ASCII	0040
#define DEL_ASCII	0177
#define META_ASCII	0200
#define AT_ASCII	0100

static int key_string(buf, key)
char *buf;			/* Destination buffer (at least XPA_ACBUFL) */
int key;			/* ASCII key description                    */
/*
 * Builds a readable description for the ASCII character `key'.  If
 * it is a control character,  it is prefixed with a hat (^).  If
 * it is above 0177,  it is prefixed with 'M-'.  The description
 * is written into `buf' (which should be at least XPA_ACBUFL in length).
 * The actual length is returned.  There are ASCII dependencies here.
 */
{
    int len = 0;

    if (key >= META_ASCII) {
	while (key >= META_ASCII) key -= META_ASCII;
	buf[len++] = 'M';
	buf[len++] = '-';
    }
    if (key < SPACE_ASCII) {
	/* Control key */
	key += AT_ASCII;
	buf[len++] = '^';
    }
    if (key == SPACE_ASCII) {
	buf[len++] = 's';
	buf[len++] = 'p';
	buf[len++] = 'c';
    } else if (key == DEL_ASCII) {
	buf[len++] = 'd';
	buf[len++] = 'e';
	buf[len++] = 'l';
    } else {
	buf[len++] = (char) key;
    }
    buf[len] = '\0';
    return len;
}



static void set_pb_sym(disp)
Display *disp;
/*
 * Loads the XPA_PB_FONT font.  Determines the size of the XPA_PB_SYM
 * character.  Sets these up in globals pb_sym_font, pb_sym_w, pb_sym_h.
 */
{
    char ch[4];
    int dir, ascent, descent;

    if (!(pb_sym_font = XLoadQueryFont(disp, XPA_PB_FONT))) {
	/* Could try to recover in the future */
	xpa_raise(XPA_NOFONT);
    }
    ch[0] = XPA_PB_SYM;
    XTextExtents(pb_sym_font, ch, 1, &dir, &ascent, &descent, &pb_sym_metric);
    pb_sym_w = pb_sym_metric.rbearing - pb_sym_metric.lbearing + 2*XPA_PB_PAD;
    pb_sym_h = pb_sym_metric.ascent + pb_sym_metric.descent + 2*XPA_PB_PAD;
}


/*
 * Allocator functions for each context table type
 */

static xpa_pane *new_pane(win, level)
Window win;			/* Pane window        */
int level;			/* Level in heirarchy */
/* Creates a new pane structure */
{
    xpa_pane *result = XPA_MEM(xpa_pane);

    if (!result) xpa_raise(XPA_NOMEM);
    result->type = PANE_T;
    result->pane_win = win;
    result->level = level;
    return result;
}

static xpa_item *new_item(win, name, key_idx, level, idx, iw, ih, baseline,
			  fromtop, key_off, sub_pane, pb)
Window win;			/* Item window              */
char *name;			/* Name of item 	    */
int key_idx;			/* Accelerator key          */
int level;			/* Level in hierarchy 	    */
int idx;			/* Offset in pane 	    */
int iw, ih;			/* Physical size 	    */
int baseline;			/* Offset to baseline       */
int fromtop;			/* Distance from top        */
int key_off;			/* End of accelerator       */
Window sub_pane;		/* Posted sub-pane (if any) */
xpa_pb *pb;			/* Pullbox info             */
/* 
 * Creates a new item structure - automatically copies `name'.
 * Automatically expands `key_idx' into a textual representation.
 */
{
    xpa_item *result = XPA_MEM(xpa_item);

    if (!result) xpa_raise(XPA_NOMEM);
    result->type = ITEM_T;
    result->item_win = win;
    result->name = XPA_SAVE(name);
    if (key_idx > XPA_NOCHAR) {
	(void) key_string(result->key_buf, key_idx);
    } else {
	result->key_buf[0] = '\0';
    }
    result->level = level;
    result->idx = idx;
    result->iw = iw;
    result->ih = ih;
    result->baseline = baseline;
    result->fromtop = fromtop;
    result->key_off = key_off;
    result->sub_pane = sub_pane;
    result->pb = pb;
    return result;
}

static xpa_pb *new_pb(win, item, height)
Window win;			/* Pullbox window  */
xpa_item *item;			/* Associated item */
int height;			/* Height of box   */
/* Creates a new pullbox structure */
{
    xpa_pb *result = XPA_MEM(xpa_pb);

    if (!result) xpa_raise(XPA_NOMEM);
    result->type = PB_T;
    result->pb_win = win;
    result->item = item;
    result->height = height;
    return result;
}

static xpa_title *new_title(win, str, metric, fullwidth, lev)
Window win;			/* Title window  */
char *str;			/* Title string  */
XCharStruct *metric;		/* String metric */
int fullwidth;			/* Full width    */
int lev;			/* Hierarchy lev */
/* Creates a new title structure (string is automatically copied) */
{
    xpa_title *result = XPA_MEM(xpa_title);

    if (!result) xpa_raise(XPA_NOMEM);
    result->type = TITLE_T;
    result->title_win = win;
    result->title_str = XPA_SAVE(str);
    result->t_metric = *metric;
    result->fullwidth = fullwidth;
    result->level = lev;
    return result;
}


#ifdef DEBUG
static void out_menu(title, entrys, indent)
char *title;
xpa_entry *entrys;
int indent;
/* For debugging */
{
    int i;

    if (title) {
	for (i = 0;  i < indent-1;  i++) putchar(' ');
	(void) printf("%s (pane name)\n", title);
    }
    while (entrys->item_name) {
	for (i = 0;  i < indent;  i++) putchar(' ');
	if (entrys->key_char > 0) {
	    (void) printf("%s (%d=%c)\n", entrys->item_name,
			  entrys->key_char, entrys->key_char);
	} else {
	    (void) printf("%s\n", entrys->item_name);
	}
	if (entrys->sub_entrys) {
	    out_menu(entrys->sub_title, (xpa_entrys) entrys->sub_entrys, indent+3);
	}
	entrys++;
    }
}

static void out_detail(evt, str)
XEvent *evt;
char *str;
/* Shows detail of leave event */
{
    (void) fputs(str, stdout);
    switch (evt->xcrossing.detail) {
    case NotifyAncestor:
	(void) printf("%s\n", "NotifyAncestor");
	break;
    case NotifyVirtual:
	(void) printf("%s\n", "NotifyVirtual");
	break;
    case NotifyInferior:
	(void) printf("%s\n", "NotifyInferior");
	break;
    case NotifyNonlinear:
	(void) printf("%s\n", "NotifyNonlinear");
	break;
    case NotifyNonlinearVirtual:
	(void) printf("%s\n", "NotifyNonlinearVirtual");
	break;
    }
}
#endif

/*
 * Handle graphics contexts
 */

static GC fg_gc(disp, win, font, fg)
Display *disp;			/* X Connection       */
Window win;			/* What window to use */
XFontStruct *font;		/* Font to use        */
unsigned long fg;		/* Foreground color   */
/*
 * Creates or modifies a static graphics context to have the
 * specified values.  This one is generally used for drawing
 * text.
 */
{
    static GC gc = (GC) 0;
    XGCValues gcvals;

    gcvals.foreground = fg;
    gcvals.font = font->fid;
    if (gc) {
	XChangeGC(disp, gc, GCForeground|GCFont, &gcvals);
    } else {
	gc = XCreateGC(disp, win, GCForeground|GCFont, &gcvals);
    }
    return gc;
}

static GC bg_gc(disp, win, bg)
Display *disp;			/* X Connection       */
Window win;			/* What window to use */
unsigned long bg;		/* Background color   */
/*
 * Creates or modifies a static graphics context to have the
 * specified values.  This one is generally used for filling
 * areas.
 */
{
    static GC gc = (GC) 0;
    XGCValues gcvals;

    gcvals.foreground = bg;
    if (gc) {
	XChangeGC(disp, gc, GCForeground, &gcvals);
    } else {
	gc = XCreateGC(disp, win, GCForeground, &gcvals);
    }
    return gc;
}


static GC bd_gc(disp, win, bd)
Display *disp;			/* X Connection       */
Window win;			/* What window to use */
unsigned long bd;		/* Border color   */
/*
 * Creates or modifies a static graphics context to have the
 * specified values.  This one is generally used for filling
 * areas.
 */
{
    static GC gc = (GC) 0;
    XGCValues gcvals;

    gcvals.foreground = bd;
    if (gc) {
	XChangeGC(disp, gc, GCForeground, &gcvals);
    } else {
	gc = XCreateGC(disp, win, GCForeground, &gcvals);
    }
    return gc;
}


static GC cr_gc(disp, win, cr)
Display *disp;			/* X Connection       */
Window win;			/* What window to use */
unsigned long cr;		/* Border color   */
/*
 * Creates or modifies a static graphics context to have the
 * specified values.  This one is generally used for filling
 * areas.
 */
{
    static GC gc = (GC) 0;
    XGCValues gcvals;

    gcvals.foreground = cr;
    if (gc) {
	XChangeGC(disp, gc, GCForeground, &gcvals);
    } else {
	gc = XCreateGC(disp, win, GCForeground, &gcvals);
    }
    return gc;
}


static int depth(entrys)
xpa_entry *entrys;
/*
 * Examines menu specification and returns the maximum depth.
 */
{
    xpa_entry *idx;
    int max, d;

    max = 0;
    for (idx = entrys;  idx->item_name;  idx++) {
	if (idx->sub_entrys) {
	    d = depth((xpa_entry *) idx->sub_entrys);
	    if (d > max) max = d;
	}
    }
    return max+1;
}



static void dispatch(win, intern, data, callback)
Window win;			/* Window id             */
xpa_intern *intern;		/* Internal view of menu */
xpa_data *data;			/* Data itself           */
int (*callback)();		/* Function to call      */
/*
 * This routine adds a new item to the context table of
 * menu `intern'.  The `data' and `callback' information
 * is added under the `win' identifier.
 */
{
    xpa_context *new_context;

    if (!(new_context = XPA_MEM(xpa_context))) xpa_raise(XPA_NOMEM);
    new_context->data = data;
    new_context->callback = callback;
    new_context->intern = intern;
    new_context->next = intern->allwins;
    intern->allwins = new_context;
    if (XSaveContext(intern->disp, win, xpa_xcon, (caddr_t) new_context)) {
	xpa_raise(XPA_NOMEM);
    }
}


static void item_draw(intern, item)
xpa_intern *intern;		/* Internal view of menu */
xpa_item *item;			/* Item to turn on       */
/*
 * Draws the specified item.  Checks to see if it is currently
 * selected.  If so,  the window is redrawn highlighted.  Now
 * draws accelerator key against the left margin and the
 * text at the key offset for the item.
 */
{
    if (intern->state[item->level].item == item) {
	/* Selected */
/*
	XFillRectangle(intern->disp, item->item_win,
		       cr_gc(intern->disp, item->item_win,
			     intern->attr->cur_fg),
		       0, 0, item->iw, item->ih);
*/
/*	XDrawRectangle(intern->disp, item->item_win,
		       cr_gc(intern->disp, item->item_win,
			     intern->attr->cur_fg),
		       1, 1, item->iw-3, item->ih-3);

*/
	XDrawRectangle(intern->disp, item->item_win,
		       cr_gc(intern->disp, item->item_win,
			     intern->attr->cur_fg),
		       0, 0, item->iw-1, item->ih-1);

	if (item->key_buf[0]) {
	    XDrawString(intern->disp, item->item_win,
			fg_gc(intern->disp, item->item_win,
			      intern->attr->key_font,
			      intern->attr->item_fg),
			XPA_HORPAD, item->baseline,
			item->key_buf, strlen(item->key_buf));
	}
	XDrawString(intern->disp, item->item_win,
		    fg_gc(intern->disp, item->item_win,
			  intern->attr->item_font,
			  intern->attr->item_fg),
		    item->key_off+XPA_HORPAD, item->baseline,
		    item->name, strlen(item->name));
    } else {
	/* Not selected */
	XFillRectangle(intern->disp, item->item_win,
		       bg_gc(intern->disp, item->item_win,
			     intern->attr->item_bg),
		       0, 0, item->iw, item->ih);
	if (item->key_buf[0]) {
	    XDrawString(intern->disp, item->item_win,
			fg_gc(intern->disp, item->item_win,
			      intern->attr->key_font,
			      intern->attr->item_fg),
			XPA_HORPAD, item->baseline,
			item->key_buf, strlen(item->key_buf));
	}
	XDrawString(intern->disp, item->item_win,
		    fg_gc(intern->disp, item->item_win,
			  intern->attr->item_font,
			  intern->attr->item_fg),
		    item->key_off+XPA_HORPAD, item->baseline,
		    item->name, strlen(item->name));
    }
}



static void item_off(intern, off_item)
xpa_intern *intern;		/* Internal view of menu */
xpa_item *off_item;		/* Item itself           */
/*
 * Turns off a given item.  This includes unmapping all subpanes
 * of the item (if any), turning off the hightlighting of the
 * item,  and zeroing out the selection in the state field
 * of `intern'.
 */
{
    int i;

    /* Turn off all subitems (outside to inside) */
    for (i = intern->depth-1;  i > off_item->level;  i--) {
	if (intern->state[i].pane) {
	    XUnmapWindow(intern->disp, intern->state[i].pane);
	}
	intern->state[i].pane = (Window) 0;
	intern->state[i].item = (xpa_item *) 0;
    }
    /* Remove this item from state */
    intern->state[off_item->level].item = (xpa_item *) 0;
    /* Redraw the item */
    item_draw(intern, off_item);
    /* Redraw pullbox */
    if (off_item->pb) pb_draw(intern, off_item->pb);
}



static void item_on(intern, on_item)
xpa_intern *intern;		/* Internal view of menu */
xpa_item *on_item;		/* Item to turn on       */
/*
 * Turns on the specified item.  This includes setting the
 * appropriate pointer in the state field of `intern'
 * and actually redrawing the item.
 */
{
    /* Turn on the selection of this item */
    intern->state[on_item->level].item = on_item;
    /* Redraw item */
    item_draw(intern, on_item);
    /* Redraw pullbox */
    if (on_item->pb) pb_draw(intern, on_item->pb);
}


static void pane_on(intern, pane, lev, x, y, item)
xpa_intern *intern;		/* Internal view of menu */
Window pane;			/* What pane             */
int lev;			/* Level in heirarchy    */
int x;				/* Left edge of pane     */
int y;				/* Location of `item'    */
xpa_item *item;			/* Item in pane          */
/*
 * This routine posts the pane `pane'.  The left edge of the
 * pane will appear at `x' (relative to the root window).
 * The baseline of `item' will be aligned with `y'.  If
 * `item' is zero, either the title or the first item will
 * be aligned with `y' (depending on whether the pane has
 * a title).  Note: should compensate for the screen as well.
 */
{
    int true_y, width, height;
    extern int displayWidth;
    extern int displayHeight;

    if (item) {
	true_y = y - item->fromtop;
    } else {
	/* Should be a test for title */
	true_y = y - intern->attr->item_font->ascent;
    }
    width = intern->width;
    height = intern->height;

    if (x + width > displayWidth)
      x = displayWidth - width;
    if (true_y + height > displayHeight) 
      true_y = displayHeight - height;

    intern->state[lev].x = x;
    intern->state[lev].y = true_y;
    intern->state[lev].pane = pane;
    XMoveWindow(intern->disp, pane, x, true_y);
    XRaiseWindow(intern->disp, pane);
    XMapWindow(intern->disp, pane);
}


static void pb_draw(intern, pb)
xpa_intern *intern;		/* Internal view of menu */
xpa_pb *pb;			/* Item to turn on       */
/*
 * Draws the specified pullbox.  A small arrow is drawn using
 * the cursor font and a special character.  Eventually
 * should recover if there isn't a cursor font.
 */
{
    char ch[4];
    int ox, oy;

    ch[0] = XPA_PB_SYM;
    ox = XPA_PB_PAD - pb_sym_metric.lbearing;
    oy = pb->item->baseline - pb_sym_metric.descent - 2;
    if (intern->state[pb->item->level].item == pb->item) {
	/* Selected */
/*
	XFillRectangle(intern->disp, pb->pb_win,
		       cr_gc(intern->disp, pb->pb_win,
			     intern->attr->cur_fg),
		       0, 0, XPA_PB_W, pb->height);
	XDrawString(intern->disp, pb->pb_win,
		    fg_gc(intern->disp, pb->pb_win,
			  pb_sym_font, intern->attr->item_bg),
		    ox, oy,
		    ch, 1);
*/
    } else {
	/* Not selected */
/*	XFillRectangle(intern->disp, pb->pb_win,
		       bg_gc(intern->disp, pb->pb_win,
			     intern->attr->item_bg),
		       1, 0, XPA_PB_W, pb->height-2);
*/
	XDrawString(intern->disp, pb->pb_win,
		    fg_gc(intern->disp, pb->pb_win,
			  pb_sym_font, intern->attr->item_fg),
		    ox, oy,
		    ch, 1);
    }
}



static void title_draw(intern, title)
xpa_intern *intern;		/* Internal view of menu */
xpa_title *title;		/* Title to draw         */
/* 
 * Draws the given title.  Since the background is set to
 * the title background,  all that is required is that
 * the string be rendered.
 */
{
    int x;

#ifdef OLD
    x = (title->fullwidth -
	 (title->t_metric.rbearing - title->t_metric.lbearing))/2 -
	   title->t_metric.lbearing;
#endif
    x = (title->fullwidth-title->t_metric.rbearing-title->t_metric.lbearing)/2;

    XDrawString(intern->disp, title->title_win,
		fg_gc(intern->disp, title->title_win,
		      intern->attr->title_font,
		      intern->attr->title_fg),
		x, intern->attr->title_font->ascent,
		title->title_str, strlen(title->title_str));
}


/*
 * Callback functions begin on this page.
 */

static int cb_pane(evt, intern, data)
XEvent *evt;			/* X Event causing callback */
xpa_intern *intern;		/* Internal view of menu    */
xpa_data *data;			/* Specific instance data   */
/*
 * This function is called when an event occurs on a pane.  Right
 * now,  only a leave event is recognized.  The data is (xpa_pane *).
 * If there isn't a sub-pane posted,  it will turn off the currently
 * selected item.
 */
{
    int rv;

    if (data->any.type != PANE_T) xpa_raise(XPA_TYPE);
    switch (evt->type) {
    case LeaveNotify:
	if ((evt->xcrossing.mode == NotifyNormal) &&
	    ((evt->xcrossing.detail == NotifyNonlinear) ||
	     (evt->xcrossing.detail == NotifyNonlinearVirtual))) {
	    if (intern->state[data->pane.level].item &&
		((data->pane.level >= intern->depth-1) ||
		 (!(intern->state[data->pane.level+1].pane)))) {
		/* Turn it off */
		item_off(intern, intern->state[data->pane.level].item);
	    }
	}
	rv = 0;
	break;
    case ButtonPress:
	/* Only if press_flag is on */
	rv = intern->press_flag;
	break;
    case ButtonRelease:
	/* Only if press_flag is off */
	rv = intern->press_flag == 0;
	{
		extern int mouseButtonPressedState;
		mouseButtonPressedState &= 
			~(1<<(((XButtonEvent*)evt)->button));
	}
	break;
    default:
	xpa_raise(XPA_BADEVT);
    }
    return rv;
}



static int cb_item(evt, intern, data)
XEvent *evt;			/* X Event causing callback */
xpa_intern *intern;		/* Internal view of menu    */
xpa_data *data;			/* Specific instance data   */
/*
 * This function is called when an event occurs in an item.
 * Two events are processed: entry and exposure.  Upon entry,
 * any other selected item is turned off on its level.  The
 * selected item is then turned on.  Exposure simply causes
 * the item to be redrawn.
 */
{
    if (data->any.type != ITEM_T) xpa_raise(XPA_TYPE);
    switch (evt->type) {
    case EnterNotify:
	/* Turn off current selection on this level (if any) */
	if (intern->state[data->item.level].item) {
	    item_off(intern, intern->state[data->item.level].item);
	}
	/* Turn on this item */
	item_on(intern, (xpa_item *) data);
	break;
    case Expose:
	if (evt->xexpose.count == 0) {
	    item_draw(intern, (xpa_item *) data);
	}
	break;
    default:
	xpa_raise(XPA_BADEVT);
    }
    return 0;
}


static int cb_pb(evt, intern, data)
XEvent *evt;			/* X Event causing callback */
xpa_intern *intern;		/* Internal view of menu    */
xpa_data *data;			/* Specific instance data   */
/*
 * This function is called when events occur on the 
 * pullbox.  Entering this window posts the subpane of 
 * the associated item.  Exposure draws a small arrow
 * in the box.
 */
{
    if (data->any.type != PB_T) xpa_raise(XPA_TYPE);
    switch (evt->type) {
    case EnterNotify:
	/* Post subpane */
	if (evt->xcrossing.detail == NotifyAncestor) {
	    pane_on(intern, data->pb.item->sub_pane,
		    data->pb.item->level+1,
		    intern->state[data->pb.item->level].x +
		    data->pb.item->iw - XPA_HORPAD - XPA_PB_W,
		    intern->state[data->pb.item->level].y +
		    data->pb.item->fromtop, (xpa_item *) 0);
	}
	break;
    case Expose:
	if (evt->xexpose.count == 0) {
	    pb_draw(intern, (xpa_pb *) data);
	}
	break;
    default:
	xpa_raise(XPA_BADEVT);
    }
    return 0;
}



static int cb_title(evt, intern, data)
XEvent *evt;			/* X Event causing callback */
xpa_intern *intern;		/* Internal view of menu    */
xpa_data *data;			/* Specific instance data   */
/*
 * This function is called when events occur in a title.  Entering
 * this item turns off the currently selected item.  Exposure redraws
 * the item.
 */
{
    if (data->any.type != TITLE_T) xpa_raise(XPA_TYPE);
    switch (evt->type) {
    case EnterNotify:
	/* Turn off current selection on this level (if any) */
	if (intern->state[data->title.level].item) {
	    item_off(intern, intern->state[data->title.level].item);
	}
	break;
    case Expose:
	if (evt->xexpose.count == 0) {
	    title_draw(intern, (xpa_title *) data);
	}
	break;
    default:
	xpa_raise(XPA_BADEVT);
    }
    return 0;
}



#define SAVEUNDER	0x01
#define OVERRIDE	0x02

static Window make_window(disp, parent, x, y, width, height,
			  bwidth, bor, bg, cur, events, options)
Display *disp;			/* X Connection  */
Window parent;			/* Parent window */
int x, y;			/* Location      */
int width, height;		/* Size          */
int bwidth;			/* Border width  */
unsigned long bor, bg;		/* Border and background pixels */
Cursor cur;			/* Window cursor (or None)      */
long events;			/* Interesting events           */
int options;			/* SAVEUNDER, OVERRIDE          */
/*
 * Makes a new window and returns its handle.  Basically,  this
 * is a convenient way to call XCreateWindow.  Saveunder will be
 * used if `options' contains SAVEUNDER.  Override-redirect will
 * be used if `options' contains OVERRIDE.
 */
{
    XSetWindowAttributes attr;
    unsigned long wamask;

    attr.background_pixel = bg;
    attr.border_pixel = bor;
    attr.save_under = (options & SAVEUNDER) != 0;
    attr.override_redirect = (options & OVERRIDE) != 0;
    attr.event_mask = events;
    attr.cursor = cur;
    wamask = CWBackPixel|CWBorderPixel|CWSaveUnder|CWOverrideRedirect|
      CWEventMask|CWCursor;

    return XCreateWindow(disp, parent, x, y, width, height, bwidth,
			 CopyFromParent, InputOutput, CopyFromParent,
			 wamask, &attr);
}


static void item_size(font, name, sub_flag, ac_font, ac_key, rw, rh, basel, ac_off)
XFontStruct *font;		/* Font for display     */
char *name;			/* Name of item         */
int sub_flag;			/* Does it have submenu */
XFontStruct *ac_font;		/* Accelerator font     */
int ac_key;			/* Accelerator key      */
int *rw, *rh;			/* Returned size        */
int *basel;			/* Returned baseline    */
int *ac_off;			/* End of accelerator   */
/*
 * Computes the minimal size for an item given its font, name,
 * whether it has a submenu, and the key and font of the character
 * accelerator.  `ac_off' gives the offset of the end of the accelerator
 * in pixels.  Note that the returned width DOES NOT include the
 * accelerator space.
 */
{
    char buf[XPA_ACBUFL];
    int len, largest;

    if (ac_key > XPA_NOCHAR) {
	len = key_string(buf, ac_key);
	*ac_off = XPA_HORPAD + XTextWidth(ac_font, buf, len);
    } else {
	*ac_off = 0;
    }
    *rw = 2*XPA_HORPAD + XTextWidth(font, name, strlen(name));
    if (sub_flag) {
	*rw += XPA_HORPAD + XPA_PB_W;
    }
    largest = 0;
    if (font->ascent > largest) largest = font->ascent;
    if (ac_font->ascent > largest) largest = ac_font->ascent;
    if (XPA_PB_H > largest) largest = XPA_PB_H;
    *basel = largest + XPA_VERPAD;

    largest = 0;
    if (font->descent > largest) largest = font->descent;
    if (ac_font->descent > largest) largest = ac_font->descent;
    *rh = *basel + largest + XPA_VERPAD;
}

static void title_size(font, title, rw, rh)
XFontStruct *font;		/* Font for display */
char *title;			/* Title itself     */
int *rw, *rh;			/* Returned size    */
/*
 * Computes a suitable bounding box for `title'.
 */
{
    *rw = 2 * XPA_HORPAD + XTextWidth(font, title, strlen(title));
    *rh = 2 * XPA_VERPAD + font->ascent + font->descent;
}



static struct xpa_pb_defn *make_pullbox(intern, parent, item, x, height)
xpa_intern *intern;		/* Internal view of menu */
Window parent;			/* Parent of pullbox     */
xpa_item *item;			/* Associated item       */
int x;				/* Horizontal location   */
int height;			/* Height of pullbox     */
/*
 * Creates a new window under `parent' for the pullbox.  This
 * window has no border and its background is an arrow bitmap.
 * It has an associated callback that posts a sub-pane for
 * an item.
 */
{
    Window pullbox;
    xpa_pb *pb;

    pullbox = make_window(intern->disp, parent, x, 1,
			  XPA_PB_W, height, XPA_PB_B,
			  intern->attr->border_bd, intern->attr->item_bg,
			  (Cursor) None, EnterWindowMask | ExposureMask, 0);
    pb = new_pb(pullbox, item, height);
    dispatch(pullbox, intern, (xpa_data *) pb, cb_pb);
    XMapWindow(intern->disp, pullbox);
    return (struct xpa_pb_defn *) pb;
}



static int make_title(intern, parent, title_str, lev, width, yspot)
xpa_intern *intern;		/* Internval view of menu */
Window parent;			/* Parent window          */
char *title_str;		/* Title string           */
int lev;			/* Level in Hierarchy     */
int width;			/* Width of item          */
int yspot;			/* Y location             */
/*
 * Creates a subwindow of `parent' that displays a the title
 * string `title_str'.  `width' specifies the width of the
 * item and `yspot' specifies the y location.  The appropriate
 * callback will be added to the dispatch table.  The height
 * of the new item will be returned.
 */
{
    Window title_win;
    XCharStruct metric;
    int dir, ascent, descent;
    int tw, th;

    title_size(intern->attr->title_font, title_str, &tw, &th);
    title_win = make_window(intern->disp, parent, 0, yspot,
			    width, th, 0, intern->attr->border_bd, 
			    intern->attr->title_bg, (Cursor) None,
			    EnterWindowMask | ExposureMask, 0);
    XTextExtents(intern->attr->title_font, title_str, strlen(title_str),
		 &dir, &ascent, &descent, &metric);
    dispatch(title_win, intern,
	     (xpa_data *) new_title(title_win, title_str, &metric, width, lev),
	     cb_title);
    XMapWindow(intern->disp, title_win);
    return th;
}



/* Forward declaration */
static Window make_pane();

static int make_item(intern, parent, entry, lev, idx, width, yspot, key_off)
xpa_intern *intern;		/* Internal view of menu */
Window parent;			/* Parent window         */
xpa_entry *entry;		/* Entry itself          */
int lev;			/* Level in heirarchy    */
int idx;			/* Numerical index       */
int width;			/* Width of item         */
int yspot;			/* Y location            */
int key_off;			/* End of accelerator    */
/*
 * Creates a subwindow of `parent' that displays the item
 * `entry'.  The width of the item is given by `width'
 * and its y location is given by `yspot'.  If the item
 * has a sub-pane,  it will be created as well.  The window
 * will be added to the dispatch table.  The height of
 * the new item will be returned.  The expose routine
 * for the item will draw the accelerator key (if any)
 * at the left margin.  The text itself will be drawn
 * at `key_off'.
 */
{
    Window item_win, sub_win;
    xpa_item *item;
    int exp_w, exp_h, bl, off;

    if (entry->item_type == XPA_ITEM_TYPE_ENTRY) {
	    /* Called only to determine height of item */
	    item_size(intern->attr->item_font, entry->item_name,
		      (entry->sub_entrys != (xpa_entry *) 0),
		      intern->attr->key_font, entry->key_char,
		      &exp_w, &exp_h, &bl, &off);
	    item_win = make_window(intern->disp, parent, 0, yspot,
				   width, exp_h, 0,
				   intern->attr->border_bd,
				   intern->attr->item_bg, (Cursor) None,
				   EnterWindowMask | ExposureMask, 0);
    } else if (entry->item_type == XPA_ITEM_TYPE_SEPARATOR) {
/*
	    item_size(intern->attr->item_font, entry->item_name,
		      (entry->sub_entrys != (xpa_entry *) 0),
		      intern->attr->key_font, entry->key_char,
		      &exp_w, &exp_h, &bl, &off);
*/
	    bl = 1;
	    exp_h = 1;
	    item_win = make_window(intern->disp, parent, 0, yspot,
				   width, exp_h, 1, 
				   intern->attr->border_bd,
				   intern->attr->item_fg, (Cursor) None,
				   EnterWindowMask | ExposureMask, 0);

    }
    if (entry->sub_entrys) {
	sub_win = make_pane(intern, entry->sub_title,
			    (xpa_entry *) entry->sub_entrys, lev+1);
    } else {
	sub_win = (Window) 0;
    }
    item = new_item(item_win, entry->item_name, entry->key_char, lev, idx,
		    width, exp_h, bl, yspot + bl, key_off,
		    sub_win, (xpa_pb *) 0);
    if (entry->sub_entrys) {
	item->pb = make_pullbox(intern, item_win, item,
				width - XPA_HORPAD - XPA_PB_W,
				exp_h-2/*XXXXX*/);
    }
    dispatch(item_win, intern, (xpa_data *) item, cb_item);
    XMapWindow(intern->disp, item_win);
    return exp_h;
}

static Window make_pane(intern, title, entrys, lev)
xpa_intern *intern;		/* Internal view of menu */
char *title;			/* Title (zero if none)  */
xpa_entry *entrys;		/* Menu itself           */
int lev;			/* Level in heirarchy    */
/*
 * Makes a pane and inserts it into the context
 * table.  Also makes any submenu panes.
 */
{
    Window result;
    int w, h, tw, th, bl, off, key_off, count, i;
    xpa_entry *idx;

    /* Determine size of result window */
    if (title) {
	title_size(intern->attr->title_font, title, &w, &h);
    } else {
	w = h = 0;
    }
    count = 0;
    key_off = 0;
    for (idx = entrys;  idx->item_name;  idx++) {
        if (idx->item_type == XPA_ITEM_TYPE_ENTRY) {
	    item_size(intern->attr->item_font, idx->item_name,
		  (idx->sub_entrys != (xpa_entrys) 0),
		  intern->attr->key_font, idx->key_char,
		  &tw, &th, &bl, &off);
        } else if (idx->item_type == XPA_ITEM_TYPE_SEPARATOR) {
		  tw = 0;
		  th = 1;
		  bl = 1;
		  off = 0;
        }
	h += th;
	if (tw > w) w = tw;
	if (off > key_off) key_off = off;
	count++;
    }
    intern->width = w+key_off;
    intern->height = h;

    /* Make window */
    result = make_window(intern->disp, DefaultRootWindow(intern->disp),
			 0, 0, w+key_off, h, XPA_BORDER, 
			 intern->attr->border_bd,
			 intern->attr->title_bg, intern->cursor,
			 LeaveWindowMask | ButtonPressMask | ButtonReleaseMask,
			 SAVEUNDER | OVERRIDE);
    dispatch(result, intern, (xpa_data *) new_pane(result, lev), cb_pane);

    /* Make subwindows and subpanes */
    th = 0;
    if (title) {
	th += make_title(intern, result, title, lev, w+key_off, th);
    }
    for (i = 0;  i < count;  i++) {
	th += make_item(intern, result, &(entrys[i]), lev, i, w+key_off, th, key_off);
    }
    return result;
}


static XFontStruct *def_font(disp, mask, bit, new, defmask, defvalue, fontspec)
Display *disp;			/* X Connection  */
int mask;			/* Passed mask   */
int bit;			/* Bit to check  */
XFontStruct *new;		/* User value    */
int *defmask;			/* Default mask  */
XFontStruct **defvalue;		/* Default value */
char *fontspec;			/* Font specif.  */
/*
 * Handles default handling for a font.  If `bit' is set in `mask',
 * `new' is returned, otherwise `defValue' is returned.  If `bit'
 * is not set in `defmask',  `defValue' is first loaded using
 * `fontspec' and the bit is set in `defmask'.
 */
{
    char **font_list;
    int count;

    if (mask & bit) return new;
    if (!(*defmask & bit)) {
	font_list = XListFonts(disp, fontspec, 1, &count);
	*defvalue = XLoadQueryFont(disp, font_list[0]);
	XFreeFontNames(font_list);
	*defmask |= bit;
    }
    return *defvalue;
}

static unsigned long def_pixel(disp, mask, bit, new, defmask, defvalue, clrspec)
Display *disp;			/* X Connection  */
int mask;			/* Passed mask   */
int bit;			/* Bit to check  */
unsigned long new;		/* User value    */
int *defmask;			/* Default mask  */
unsigned long *defvalue;	/* Default value */
char *clrspec;			/* Color specif. */
/*
 * Handles default handling for a color.  Similar to def_font().
 * Color specification only handles "white" and "black".  Later
 * it could look up the named color.
 */
{
    if (mask & bit) return new;
    if (!(*defmask & bit)) {
	if (strcmp(clrspec, "white") == 0) {
	    *defvalue = WhitePixel(disp, DefaultScreen(disp));
	} else if (strcmp(clrspec, "black") == 0) {
	    *defvalue = BlackPixel(disp, DefaultScreen(disp));
	} else {
	    xpa_raise(XPA_BADCLR);
	}
	*defmask |= bit;
    }
    return *defvalue;
}

#define PF(ptr, field)	(ptr ? ptr->field : 0)

static xpa_appearance *def_appearance(disp, mask, passed)
Display *disp;			/* X Connection    */
int mask;			/* Appearance mask */
xpa_appearance *passed;		/* Passed values   */
/*
 * Returns an appearance where values are filled from default values
 * unless the appropriate bit in `mask' is set.  If the bit is
 * set,  the value is taken from `passed'.
 */
{
    static xpa_appearance def_app;
    static xpa_appearance rtn_app;
    static int app_init = 0;

    if (mask & !passed) xpa_raise(XPA_ZAPP);
    rtn_app.title_font =
      def_font(disp, mask, XPA_T_FONT, PF(passed, title_font),
	       &app_init, &(def_app.title_font), 
		"*-bold-r-*-140-*-iso8859-*");
    rtn_app.item_font =
      def_font(disp, mask, XPA_I_FONT, PF(passed, item_font),
       &app_init, &(def_app.item_font), 
		"*-medium-r-*-120-*-iso8859-*"
/*		"-adobe-times-medium-r-*-*-16-*-*-*-p-*-*-*"*/);
    rtn_app.key_font =
      def_font(disp, mask, XPA_K_FONT, PF(passed, key_font),
	       &app_init, &(def_app.key_font), "*-medium-r-*-100-*-iso8859-*");

    rtn_app.title_fg =
      def_pixel(disp, mask, XPA_T_FG, PF(passed, title_fg),
		&app_init, &(def_app.title_fg), "white");
    rtn_app.title_bg =
      def_pixel(disp, mask, XPA_T_BG, PF(passed, title_bg),
		&app_init, &(def_app.title_bg), "black");
    rtn_app.item_fg =
      def_pixel(disp, mask, XPA_I_FG, PF(passed, item_fg),
		&app_init, &(def_app.item_fg), "black");
    rtn_app.item_bg =
      def_pixel(disp, mask, XPA_I_BG, PF(passed, item_bg),
		&app_init, &(def_app.item_bg), "white");
    rtn_app.border_bd =
      def_pixel(disp, mask, XPA_I_FG, PF(passed, border_bd),
		&app_init, &(def_app.border_bd), "orange");    /*XXX*/
    rtn_app.cur_fg =
      def_pixel(disp, mask, XPA_C_FG, PF(passed, cur_fg),
		&app_init, &(def_app.cur_fg), "black");
    return &rtn_app;
}



xpa_menu xpa_create(disp, title, entrys, mask, appearance)
Display *disp;			/* X Connection            */
char *title;			/* Top pane title          */
xpa_entry *entrys;		/* Menu contents           */
int mask;			/* Display attr mask       */
xpa_appearance *appearance;	/* Menu display attributes */
/*
 * Creates a new multi-level menu and returns a handle to it.
 * Those display attributes whose mask appears in `mask' will
 * use the corresponding value in `appearance' instead of the
 * default value.
 */
{
    xpa_intern *intern;
    XColor cur_fg, cur_bg;
    int i;

    if (xpa_xcon == (XContext) 0) {
	xpa_xcon = XUniqueContext();
	set_pb_sym(disp);
    }
    if (!(intern = XPA_MEM(xpa_intern))) {
	return (xpa_menu) 0;
    }
    intern->disp = disp;
    intern->allwins = (xpa_context *) 0;
    intern->depth = depth(entrys);
    if (!(intern->state = XPA_MEMX(xpa_ms, intern->depth))) {
	xpa_raise(XPA_NOMEM);
    }
    for (i = 0;  i < intern->depth;  i++) {
	intern->state[i].x = intern->state[i].y = 0;
	intern->state[i].pane = (Window) 0;
	intern->state[i].item = (xpa_item *) 0;
    }
    if (!(intern->attr = XPA_MEM(xpa_appearance))) {
	xpa_raise(XPA_NOMEM);
    }
    /* Default appearance handling */
    appearance = def_appearance(disp, mask, appearance);
    *(intern->attr) = *appearance;
    intern->cursor = XCreateFontCursor(disp, XC_top_left_arrow);
    cur_fg.pixel = intern->attr->cur_fg;
    cur_bg.pixel = intern->attr->item_bg;
    XQueryColor(disp, DefaultColormap(disp, DefaultScreen(disp)), &cur_fg);
    XQueryColor(disp, DefaultColormap(disp, DefaultScreen(disp)), &cur_bg);
    XRecolorCursor(disp, intern->cursor, &cur_fg, &cur_bg);
    if (!(intern->top = make_pane(intern, title, entrys, 0))) {
	xpa_raise(XPA_NOMEM);
    }
    return (xpa_menu) intern;
}


#define RES_INIT	10
static int *res_ary = (int *) 0;
static int res_len = 0;
static int res_alloc = 0;

int response(intern, ary)
xpa_intern *intern;		/* Internal view of menu */
int **ary;			/* Returned array        */
/*
 * Compiles a static array of offsets for returning to
 * the user.  If no selections made,  returns zero.
 */
{
    int i;

    if (res_alloc == 0) {
	res_alloc = RES_INIT;
	res_ary = XPA_MEMX(int, res_alloc);
    }
    if (res_alloc <= intern->depth) {
	res_alloc = intern->depth + RES_INIT;
	res_ary = (int *) realloc((char *) res_ary,
				  (unsigned) (sizeof(int) * res_alloc));
    }
    res_len = 0;
    for (i = 0;  i < intern->depth;  i++) {
	if (intern->state[i].item) {
	    res_ary[res_len++] = intern->state[i].item->idx;
	}
    }
    if ((res_len == intern->depth) ||
	((res_len > 0) && (!intern->state[res_len].pane))) {
	*ary = res_ary;
    } else {
	*ary = (int *) 0;
	res_len = 0;
    }
    return res_len;
}



void xpa_post(menu, x, y, depth, vals, opt)
xpa_menu menu;			/* Menu to post 	*/
int x, y;			/* Location     	*/
int depth;			/* Depth                */
int *vals;			/* Array values         */
int opt;			/* XPA_PRESS            */
/*
 * Shows menu at location (x,y).  If `depth' is greater than
 * zero,  the menu pops up with the selection given by
 * the integer offsets `vals' (where depth specifies the
 * number of offsets).  If `depth' is zero,  the top
 * level pane is shown with the mousein the title or
 * over the first item if there is no title.  If XPA_PRESS is 
 * specified as an option,  the menu will react to button press 
 * otherwise it will react to button release.
 * NOTE: depth and vals are not implemented.
 */
{
    xpa_intern *intern = (xpa_intern *) menu;
    int i;

    intern->press_flag = (opt & XPA_PRESS) != 0;
    for (i = 0;  i < intern->depth;  i++) {
	intern->state[i].x = intern->state[i].y = 0;
	intern->state[i].pane = (Window) 0;
	intern->state[i].item = (xpa_item *) 0;
    }
    pane_on(intern, intern->top, 0, x, y, (xpa_item *) 0);
}



void xpa_unpost(menu)
xpa_menu menu;			/* Menu to turn off */
/*
 * Unposts `menu'.  This includes all of its subpanes.  Does
 * not effect menu state.
 */
{
    xpa_intern *intern = (xpa_intern *) menu;
    int i;

    for (i = intern->depth-1;  i >= 0;  i--) {
	if (intern->state[i].pane) {
	    XUnmapWindow(intern->disp, intern->state[i].pane);
	}
    }
}



int xpa_filter(evt, menu, result)
XEvent *evt;			/* Event to handle */
xpa_menu *menu;			/* Returned menu   */
int **result;			/* Returned choice */
/*
 * This routine handles events for the xpa menu package.
 * Possible return codes:
 *   XPA_FOREIGN	Event is not for the menu package
 *   XPA_HANDLED	Event was handled with no response
 * These return codes are less than zero.  If the return
 * code is zero or more,  it indicates a selection occurred
 * on `menu'.  Zero indicates no selection.  Greater than
 * zero indicates the length of `result' which gives indicies
 * into each pane in the heirarchy.
 */
{
    xpa_context *cb;
    int val;

    if (!XFindContext(evt->xany.display, evt->xany.window, xpa_xcon,
		      (caddr_t *) &cb)) {
	/* Found it */
	if (cb->callback) {
	    val = (*cb->callback)(evt, cb->intern, cb->data);
	    if (val > 0) {
		/* Something happened */
		*menu = (xpa_menu) cb->intern;
		return response(cb->intern, result);
	    }
	}
	/* Nothing happened */
	*menu = (xpa_menu) 0;
	*result = (int *) 0;
	return XPA_HANDLED;
    } else {
	/* Not our event */
	*menu = (xpa_menu) 0;
	*result = (int *) 0;
	return XPA_FOREIGN;
    }
}



int xpa_moded(menu, x, y, depth, vals, opt, handler, result)
xpa_menu menu;			/* Menu to post       */
int x, y;			/* Screen coordinates */
int depth;			/* Depth of spec      */
int *vals;			/* Starting spot      */
int opt;			/* XPA_PRESS or none  */
int (*handler)();		/* Event handler      */
int **result;			/* Returned result    */
/*
 * Posts a moded menu.  This is basically a wrapper around
 * xpa_post, xpa_filter, and xpa_unpost.  However,  it
 * will guarantee there are no more menu events before
 * returning.  If XPA_LAST is specified,  the previous
 * menu state is used.  If XPA_PRESS is specified,  the menu will
 * react to press events otherwise it will react to
 * release events.  May return XPA_NOGRAB if it can't grab
 * the mouse.  Events not recognized by the menu package
 * will be passed to `handler'.  If it returns a non-zero
 * value,  the menu will be unposted immediately.
 */
{
    XEvent evt;
    xpa_intern *intern = (xpa_intern *) menu;
    xpa_menu rtn_menu, sub_menu;
    int val, sub_val;
    int *sub_result;

    xpa_post(menu, x, y, depth, vals, opt);
    XUngrabPointer(intern->disp, CurrentTime);
    if (XGrabPointer(intern->disp, intern->top, True,
		     ButtonPressMask | ButtonReleaseMask,
		     GrabModeAsync, GrabModeAsync, None,
		     intern->cursor, CurrentTime) != GrabSuccess) {
	return XPA_NOGRAB;
    }
    for (;;) {
	XNextEvent(intern->disp, &evt);
	val = xpa_filter(&evt, &rtn_menu, result);
	if ((val >= 0) && (rtn_menu == menu)) {
	  break;
	}
	if (val == XPA_FOREIGN) {
	    if ((*handler)(&evt)) {
		break;
	    }
	}
    }
    XUngrabPointer(intern->disp, CurrentTime);
    xpa_unpost(menu);
    XSync(intern->disp, 0);
    while (XPending(intern->disp) > 0) {
	XNextEvent(intern->disp, &evt);
	sub_val = xpa_filter(&evt, &sub_menu, &sub_result);
	if (sub_val == XPA_FOREIGN) {
	    (void) (*handler)(&evt);
	}
    }

    return val;
}



void xpa_destroy(menu)
xpa_menu menu;			/* Menu to destroy */
/*
 * Frees all resources associated with a menu.
 */
{
    xpa_intern *intern = (xpa_intern *) menu;
    struct xpa_context_defn *idx, *temp;

    idx = intern->allwins;
    while (idx) {
	if (idx->data->any.type == PANE_T) {
	    XDestroyWindow(intern->disp, idx->data->any.window);
	}
	temp = idx;
	idx = idx->next;
	XPA_FREE(temp);
    }
    XPA_FREE(intern->state);
    XPA_FREE(intern->attr);
    XFreeCursor(intern->disp, intern->cursor);
    XPA_FREE(intern);
}
