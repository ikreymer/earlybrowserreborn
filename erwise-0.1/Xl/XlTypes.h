/*
 * XlTypes.h --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Thu Feb 27 21:50:02 1992 tvr
 * Last modified: Tue Apr 21 00:32:43 1992 tvr
 *
 */

/*
 * This structure consists of all Xl specific object data
 */
typedef struct XlObjectData {
    /*
     * X11 info. This graphical context has all information about
     * the used font. Text sizes can be calculated when font is
     * known.
     */
    GC gc;

    /*
     * Pointer to XlStyle table.
     */
    struct XlStyle *style;

} XlObjectData_t;


/*
 * Global data stored for a hypertext page.
 */
typedef struct XlGlobalData {
    /*
     * X11 specific data for a page
     */
    Display *dpy;
    Window window;

    /*
     * Coordinates of a view window left upper corner
     */
    long x;
    long y;

    /*
     * store GC:s for all fonts here
     */
    GC *gcs;

    GC xorgc;

    int nr_gcs;

    /*
     * Save styledata here
     */

    XlStyle_t *XlStyles;

} XlGlobalData_t;



#if 0

/*
 * No paragraph ending
 */
#define PTYPE_NONE	0

/*
 * Normal paragraph ending.
 */
#define PTYPE_NORMAL	1

/*
 * On a paragraph ending reset y value to same level where we started.
 * store maximum y.
 */
#define PTYPE_CONTINUE	2

/*
 * On a paragraph ending set y value to maximum y value of stored
 * 'PTYPE_CONTINUE' -values.
 */
#define PTYPE_NEXT	3


#endif
