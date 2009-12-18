/*
 * xpa - X Pull-Aside menu
 */

#ifndef XPA_HDR
#define XPA_HDR

#include "X11/Xlib.h"

/* Function prototypes */
#ifdef __STDC__
#define ARGS(args)	args
#else
#define ARGS(args)	()
#endif

/* Menu Entry Types */
#define XPA_ITEM_TYPE_NONE 0
#define XPA_ITEM_TYPE_ENTRY 1
#define XPA_ITEM_TYPE_SEPARATOR 2

/* Fatal Errors */
#define XPA_NOMEM	1
#define XPA_BADEVT	2
#define XPA_NOIMPL	3
#define XPA_TYPE	4
#define XPA_GRAB	5
#define XPA_NOTEVT	6
#define XPA_BADSTAT	7
#define XPA_NOFONT	8
#define XPA_ZAPP	9
#define XPA_BADCLR	10

#define XPA_NOCHAR	-1

typedef struct xpa_entry_defn {
    int  item_type;
    char *item_name;
    char *item_message;
    int key_char;
    char *sub_title;
    struct xpa_entry_defn *sub_entrys;
} xpa_entry;
typedef xpa_entry *xpa_entrys;

typedef struct xpa_dummy_defn {
    int dummy;
} xpa_dummy;
typedef xpa_dummy *xpa_menu;

#define XPA_T_FONT	0x01
#define XPA_I_FONT	0x02
#define XPA_K_FONT	0x04
#define XPA_T_FG	0x08
#define XPA_T_BG	0x10
#define XPA_I_FG	0x20
#define XPA_I_BG	0x40
#define XPA_C_FG	0x80
#define XPA_A_ALL	(~0)

typedef struct xpa_appearance_defn {
    XFontStruct *title_font;	/* Pane title font   */
    XFontStruct *item_font;	/* Item font         */
    XFontStruct *key_font;	/* Accelerator font  */
    unsigned long title_fg;	/* Title foreground  */
    unsigned long title_bg;	/* Title background  */
    unsigned long item_fg;	/* Item foreground   */
    unsigned long item_bg;	/* Item background   */
    unsigned long cur_fg;	/* Cursor foreground */
    unsigned long border_bd;	/* border */
} xpa_appearance;

extern xpa_menu xpa_create
  ARGS((Display *disp, char *title, xpa_entry *entrys,
	int mask, xpa_appearance *appearance));

#define XPA_PRESS	0x01

extern void xpa_post
  ARGS((xpa_menu menu, int x, int y, int depth, int *vals, int opt));

extern void xpa_unpost
  ARGS((xpa_menu menu));

#define XPA_NOGRAB	-3
#define XPA_FOREIGN	-2
#define XPA_HANDLED	-1

extern int xpa_filter
  ARGS((XEvent *evt, xpa_menu *menu, int **result));

extern int xpa_moded
  ARGS((xpa_menu menu, int x, int y, int depth, int *vals, int opt,
	int (*handler)(), int **result));

extern void xpa_destroy
  ARGS((xpa_menu menu));

extern char *xpa_error
  ARGS((int code));

extern void xpa_set_error
  ARGS((void (*func)()));

#endif /* XPA_HDR */

