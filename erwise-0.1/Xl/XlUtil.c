/*
 * XlUtil.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Tue Mar  3 02:55:55 1992 tvr
 * Last modified: Mon May 11 00:16:48 1992 tvr
 *
 */


#include <stdio.h>
#include <sys/types.h>

#include <X11/Xlib.h>

#include "HTAnchor.h"
#include "HTStyle.h"
#include "../HText/HText.h"

#include "XlStyle.h"
#include "XlFormatText.h"
#include "XlTypes.h"

/*
 *  Does nothing but updates htext-object positions. This has to be
 *  used after text is reformatted (on window size change).
 */
void XlSetPageCoordinates(virtualx, virtualy, htext)
int virtualx, virtualy;
HText_t *htext;
{
    htext->xl_global->x = virtualx;
    htext->xl_global->y = virtualy;
}


/*
 *  Get HTextObject position (virtual position of screen area to display)
 */
void XlGetCoordinates(virtualx, virtualy, htextobject)
int *virtualx, *virtualy;
HTextObject_t *htextobject;
{
    if (!htextobject)
	return;

    *virtualx = htextobject->x;
    *virtualy = htextobject->y;
}


/*
 * Can this object be cursor?
 */
int xl_can_be_cursor(htextobject)
HTextObject_t *htextobject;
{
    int i;

    if (htextobject->paragraph)
	return FALSE;

    for (i = 0; i < htextobject->length; i++)
	if (htextobject->data[i] != ' ')
	    return TRUE;

    return FALSE;
}


/*
 *  Get pointer to hypertextobject nearest to chosen coordinates
 */
HTextObject_t *
 XlLocateHTextObject(windowx, windowy, htext)
int windowx, windowy;
HText_t *htext;
{
    HTextObject_t *p;
    HTextObject_t *p_closest = NULL;

    int virtualx;
    int virtualy;
    int distance;

    virtualx = windowx + htext->xl_global->x;
    virtualy = windowy + htext->xl_global->y;

    for (p = htext->first; p; p = p->next) {
	int i = xl_object_on_region(htext, p, windowx, windowy, 0, 0);

	if (xl_can_be_cursor(p)) {
	    if (!i) {
		/*
	         * Point inside object ?
	         */
		p_closest = p;
		p = htext->last;

	    } else {
		/*
	         * Is this object closest ?
	         */
		if (!p_closest) {
		    p_closest = p;

		    distance = i;
		} else if (i < distance) {
		    distance = i;

		    p_closest = p;
		}
	    }
	}
    }

#ifdef XL_DEBUG
    if (p_closest) {
	printf("XlLocateHTextObject(%d, %d) -> (%d, %d) tag %s object '%s'\n",
	       windowx, windowy,
	       p_closest->x, p_closest->y,
	       p_closest->xl_data->style->styletags,
	       p_closest ? p_closest->data : "");
    }
#endif
    return p_closest;
}

/*
 * Delete all Xl data from a page. Free all data allocated on Xl-library.
 */
void XlDeleteText(htext)
HText_t *htext;
{
    int i;

    /*
     * Free X11 resources
     */
    for (i = 0; i < htext->xl_global->nr_gcs; i++) {
	XFreeGC(htext->xl_global->dpy, htext->xl_global->gcs[i]);
	XFreeFont(htext->xl_global->dpy,
		  htext->xl_global->XlStyles[i].fontinfo);
    }

    XFreeGC(htext->xl_global->dpy, htext->xl_global->xorgc);

    /*
     * Free data allocated here
     */

    {
	HTextObject_t *p;

	p = htext->first;

	while (p) {
	    free(p->xl_data);

	    p = p->next;
	}
    }

    free(htext->xl_global->XlStyles);

    free(htext->xl_global->gcs);

    free(htext->xl_global);
}


/*
 * What is the length of this without spaces
 */
int xl_nospacelength(p, data, len)
HTextObject_t *p;
char *data;
int len;
{
    if (xl_object_mode(p) & STYLE_RAW) {

	return len;
    } else {

	int i;

	for (i = 0; (i < len) && (*data++ != ' '); i++);

	return i;
    }
}


/*
 * Set up stuff needed to format text intelligently
 */
void XlFormatTextForPrinting(htext, lmargin, rmargin)
HText_t *htext;
int lmargin;
int rmargin;
{
    HTextObject_t *p;
    int i, struct_len;
    XlStyle_t *loop = XlStyles;
    int junk1;
    int junk2;

    if (!htext)
	return;

    /*
     * Set up format info
     */

    i = 1;

    while (loop->fontname) {

	loop++;

	i++;
    }

    struct_len = sizeof(XlStyle_t) * i;

    loop = (XlStyle_t *) malloc(struct_len);

    htext->xl_global = (XlGlobalData_t *) malloc(sizeof(XlGlobalData_t));

    htext->xl_global->XlStyles = loop;

    memcpy(loop, XlStyles, struct_len);

    while (--i) {

	loop->char_width = 1;

	loop++;
    }

    /*
     * Set up length and width
     */
    p = htext->first;

    while (p) {
	HTStyle *st = NULL;

	XlStyle_t *current;

	p->height = 1;

	if (!st || (st != p->style)) {

	    current = (XlStyle_t *) xl_get_style_and_gc(p, NULL, htext);

	    st = p->style;
	}
	p->xl_data = (XlObjectData_t *) malloc(sizeof(XlObjectData_t));

	p->xl_data->style = current;

	p->width = xl_nospacelength(p, p->data, p->length);

	p = p->next;
    }

    XlFormatText(lmargin, rmargin - lmargin, 0, &junk1, &junk2, htext);

    /*
     * Free local stuff
     */

    p = htext->first;

    while (p) {

	free(p->xl_data);

	p = p->next;
    }

    free(loop);

    free(htext->xl_global);
}
