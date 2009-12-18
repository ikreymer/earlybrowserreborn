/*
 * XlWindow.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Tue Mar  3 02:59:10 1992 tvr
 * Last modified: Tue Apr 21 00:34:09 1992 tvr
 *
 */

#include <stdio.h>
#include <sys/types.h>

#include <X11/Xlib.h>

#include "HTAnchor.h"
#include "HTStyle.h"
#include "../HText/HText.h"

#include "Xl.h"
#include "XlStyle.h"
#include "XlTypes.h"
#include "XlFormatText.h"

/*
 * Make cursor positioning more intelligent
 */
#define Y_MULTIPLIER 50

/*
 * Is there spaces at the end of data ?
 */
int xl_space_index(data, length)
char *data;
int length;
{
    int i;

    if (!data)
	return -1;

    for (i = 0; i < length; i++)
	if (*data++ == ' ')
	    return i;

    return length;
}


/*
 * Draw one Xl object to screen
 */
void xl_draw_object(htext, object)
HText_t *htext;
HTextObject_t *object;
{
    /*
     * If this object is cursor, clear area or else partial updating
     * does not work correctly
     */

    if (object == htext->cursor) {
	XClearArea(htext->xl_global->dpy,
		   htext->xl_global->window,
		   object->x - htext->xl_global->x,
		   object->y - htext->xl_global->y,
		   object->width,
		   object->height,
		   False);
    }
    XDrawString(htext->xl_global->dpy,
		htext->xl_global->window,
		object->xl_data->gc,
		object->x - htext->xl_global->x,
		object->y - htext->xl_global->y +
		object->xl_data->style->fontinfo->ascent,
		object->data,
		object->length);

    /*
     * Draw underline for anchors
     */
    if (object->anchor) {
	int y;
	int x;
	int width;

	y = object->y + object->xl_data->style->fontinfo->ascent +
	    object->xl_data->style->fontinfo->descent -
	    htext->xl_global->y;

	x = object->x - htext->xl_global->x;

	width = object->width;

	if (object->next && (object->anchor == object->next->anchor) &&
	    (object->y == object->next->y)) {
	    width += xl_wordgap(object);
	}
	XDrawLine(htext->xl_global->dpy,
		  htext->xl_global->window,
		  object->xl_data->gc,
		  x, y,
		  x + width, y);
    }
    /*
     * Is this object cursor ?
     */
    if (object == htext->cursor) {
	int cursorwidth;

	/*
         * If cursor is on raw mode, get rid of spaces at the end
         */
	if (xl_object_mode(object) & STYLE_RAW) {
	    int i = xl_space_index(object->data, object->length);

	    if (i >= 0) {
		cursorwidth = (int) XTextWidth(object->xl_data->style->fontinfo,
					       object->data,
					       i);
	    } else {
		cursorwidth = object->width;
	    }
	} else {
	    cursorwidth = object->width;
	}

	XDrawRectangle(htext->xl_global->dpy,
		       htext->xl_global->window,
		       htext->xl_global->xorgc,
		       object->x - htext->xl_global->x,
		       object->y - htext->xl_global->y,
		       cursorwidth - 1,
		       object->height - 1);
    }
}


/*
 * Check is object is on wanted region. Return 0 if point is on region.
 * Otherwise return distance. NOTE. y-distance is multiplied by a big
 * number because this way cursor positioning works more intelligently than
 * without it.
 */
int xl_object_on_region(htext, object, x, y, w, h)
HText_t *htext;
HTextObject_t *object;
int x, y;
int w, h;
{
    int r = 0;

    /*
     * Change real coordinates to virtual coordinates
     */
    x += htext->xl_global->x;
    y += htext->xl_global->y;

    if ((object->y + object->height) < y)
	r += (y - (object->y + object->height)) * Y_MULTIPLIER;

    if (object->y > (y + h))
	r += (object->y - (y + h)) * Y_MULTIPLIER;

    if ((object->x + object->width) < x)
	r += x - (object->x + object->width);

    if (object->x > (x + w))
	r += object->x - (x + w);

    return r;
}



/*
 *  Redraw region of the window. Draw all objects (or parts of the objects)
 *  on redraw region.
 */
void XlRedraw(windowx, windowy, width, height, htext)
int windowx, windowy;
int width, height;
HText_t *htext;
{
    HTextObject_t *p;

    /*
     * This is stupid. This has to be written again
     */

    p = htext->first;

    while (p) {
	if (xl_object_on_region(htext, p, windowx, windowy, width, height) == 0) {
	    xl_draw_object(htext, p);
	}
	p = p->next;
    }
}


/*
 *  Move window to other coordinates. Xl can optimize window moving by using
 *  regioncopy and updating only parts of display.
 */
void XlMoveWindow(newvx, newvy, htext)
int newvx, newvy;
HText_t *htext;
{
    XWindowAttributes a;

    long deltax, deltay;

    /*
     * Need to know window dimensions
     */
    XGetWindowAttributes(htext->xl_global->dpy,
			 htext->xl_global->window,
			 &a);

    /*
     * How much to move
     */
    deltax = newvx - htext->xl_global->x;
    deltay = newvy - htext->xl_global->y;

    /*
     * Can move be optimized ?
     */
    if ((abs(deltax) < a.width) && (abs(deltay) < a.height)) {
	XCopyArea(htext->xl_global->dpy,
		  htext->xl_global->window,
		  htext->xl_global->window,
		  htext->xl_global->gcs[0],	/* dummy GC */
		  deltax < 0 ? 0 : deltax,
		  deltay < 0 ? 0 : deltay,
		  a.width - abs(deltax),
		  a.height - abs(deltay),
		  deltax < 0 ? -deltax : 0,
		  deltay < 0 ? -deltay : 0);
    }
    /*
     * Set new corner coordinates
     */

    XlSetPageCoordinates(newvx, newvy, htext);

    /*
     * Clear borders to be updated again
     */
    if (deltax != 0) {
	XClearArea(htext->xl_global->dpy,
		   htext->xl_global->window,
		   deltax > 0 ? a.width - deltax : 0,
		   0,
		   abs(deltax),
		   a.height,
		   True);
    }
    if (deltay != 0) {
	XClearArea(htext->xl_global->dpy,
		   htext->xl_global->window,
		   0,
		   deltay > 0 ? a.height - deltay : 0,
		   a.width,
		   abs(deltay),
		   True);
    }
}



/*
 * Clear window
 */
void XlClearWindow(width, height, htext)
int width;
int height;
HText_t *htext;
{
    XClearArea(htext->xl_global->dpy,
	       htext->xl_global->window,
	       0, 0, width, height, True);
}


/*
 * Setup cursorposition on a page
 */
void XlSetCursor(htext, object)
HText_t *htext;
HTextObject_t *object;
{
    XWindowAttributes a;
    HTextObject_t *old_object;


    if (!htext || !htext->xl_global ||
	!htext->xl_global->dpy ||
	!htext->xl_global->window)
	return;

    /*
     * Need to know window dimensions
     */
    XGetWindowAttributes(htext->xl_global->dpy,
			 htext->xl_global->window,
			 &a);

    /*
     * Clear old cursor
     */
    if (htext->cursor) {
	if (xl_object_on_region(htext, htext->cursor, 0, 0,
				a.width, a.height) == 0) {
	    XClearArea(htext->xl_global->dpy,
		       htext->xl_global->window,
		       htext->cursor->x - htext->xl_global->x,
		       htext->cursor->y - htext->xl_global->y,
		       htext->cursor->width,
		       htext->cursor->height,
		       False);

	    /*
             * When drawing old area, must clear htext->cursor for a while
             */
	    old_object = htext->cursor;

	    htext->cursor = 0;

	    xl_draw_object(htext, old_object);
	}
    }
    old_object = object;
    htext->cursor = object;

    if (htext->cursor) {
	if (xl_object_on_region(htext, object, 0, 0, a.width, a.height) == 0) {
#if 0
	    /*
             * This is moved to draw object. partial drawing updating of
             * cursor causes otherwise problems
             */
	    XClearArea(htext->xl_global->dpy,
		       htext->xl_global->window,
		       htext->cursor->x - htext->xl_global->x,
		       htext->cursor->y - htext->xl_global->y,
		       htext->cursor->width,
		       htext->cursor->height,
		       False);
#endif
	    xl_draw_object(htext, htext->cursor);
	}
    }
}
