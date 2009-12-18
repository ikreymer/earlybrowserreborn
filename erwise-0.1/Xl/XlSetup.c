/*
 * XlSetup.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Mon Mar  2 22:06:56 1992 tvr
 * Last modified: Wed May 13 06:44:12 1992 tvr
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

#include "XlConfig.h"

int xl_junk_object(HTextObject_t * p);

/*
 * Find a style
 */
XlStyle_t *
 xl_get_style_and_gc(p, currentgc, t)
HTextObject_t *p;
GC *currentgc;
HText_t *t;
{
    XlStyle_t *loop = t->xl_global->XlStyles;

    if (!p->style) {
	sleep(1);
    }
    while (loop->fontname && strcmp(loop->styletags, p->style->SGMLTag)) {
	loop++;
    }
    if (loop->fontname) {
	if (currentgc)
	    *currentgc = t->xl_global->gcs[loop - t->xl_global->XlStyles];

	return loop;
    }
#ifdef XL_DEBUG
    printf("holy mony, style not found\n");
#endif
    return NULL;
}



/*
 *  Setup resources for Xl (this can be changed at runtime some day)
 */
int XlSetupResources(func)
void *(*func) ();
{
    void *table;
    char *item;
    XlStyle_t *styles;

    char *resource;

    table = (void *) (*func) ((void *) NULL, C_FONTS);

    styles = XlStyles;

    for (styles = XlStyles; styles->fontname; styles++) {
	resource = (char *) (*func) (table, styles->config_name);

	if (resource) {

	    if (styles->must_free_fontname)
		free(styles->fontname);

	    styles->must_free_fontname = 1;

	    styles->fontname = (char *) strdup(resource);
#ifdef XL_DEBUG
	    printf("XlConfigure: %s -> %s\n", styles->config_name, resource);
#endif
	}
    }

    return 0;
}




/*
 *  For every page, setup Xl-stuff (including dpy, for multiple screens)
 *  - Setup GC's
 *  - Calculate width and height for each object
 *  - Set page display coordinates to (0, 0)
 *  - Delete null space-objects (only spaces on nonraw-mode)
 *  - Something else (uh?)
 */
int XlSetupText(display, window, fg, bg, htext)
Display *display;
Window window;
unsigned long fg;
unsigned long bg;
HText_t *htext;
{
    HTextObject_t *p;

    XlStyle_t *s, *s_alloc;

    HTStyle *st = NULL;
    XlStyle_t *current;

    GC currentgc;

    int i;

    int struct_length;

    /*
     * safe
     */
    if (!htext)
	return;


    /*
     * Duplicate Style-structure
     */

    s = XlStyles;

    while (s->fontname)
	s++;

    /*
     * NOTE!!! Add one element because of null element at the end !!!
     */
    struct_length = sizeof(XlStyle_t) * ((s - XlStyles) + 1);

    s_alloc = s = (XlStyle_t *) malloc(struct_length);

    if (!s) {
	printf("cannot malloc on Xl\n");

	exit(1);
    }
    memcpy(s, XlStyles, struct_length);

    /*
     * Load fonts. Note. Fonts should be cached for every display.
     */
    while (s->fontname) {
	s->fontinfo =
	    (XFontStruct *) XLoadQueryFont(display, s->fontname);

	if (!s->fontinfo) {
	    printf("XlSetup: Cannot load font %s\nXlSetup: trying 'fixed'.\n",
		   s->fontname);

	    s->fontinfo =
		(XFontStruct *) XLoadQueryFont(display, "fixed");

	    /*
             * Set font up on XlStyles -table so that font wont be tried
             * every time.
             */
	    {
		XlStyle_t *ss = XlStyles + (s - s_alloc);

		if (ss->must_free_fontname)
		    free(ss->fontname);

		ss->fontname = (char *) strdup("fixed");
	    }


	    if (!s->fontinfo) {

		printf("XlSetup: Cannot load 'fixed' ... sorry.\n");

		exit(1);
	    }
	}
	/*
         * Next font
         */
	s++;
    }

    /*
     * Allocate space for page specific Xl data
     */

    if (!htext->xl_global) {

	htext->xl_global = (XlGlobalData_t *) malloc(sizeof(XlGlobalData_t));

	memset(htext->xl_global, 0, sizeof(XlGlobalData_t));

    }
    htext->xl_global->dpy = display;

    htext->xl_global->window = window;

    htext->xl_global->XlStyles = s_alloc;

    /*
     * Set up GC:s
     */
    htext->xl_global->xorgc = XCreateGC(htext->xl_global->dpy,
					htext->xl_global->window,
					0,
					NULL);

    XSetFunction(htext->xl_global->dpy, htext->xl_global->xorgc, GXxor);

    XSetForeground(htext->xl_global->dpy, htext->xl_global->xorgc, fg ^ bg);


    htext->xl_global->nr_gcs = s - s_alloc;

    htext->xl_global->gcs = (GC *) malloc((s - s_alloc) * sizeof(GC));

    for (s = s_alloc, i = 0; s->fontname; s++, i++) {

	htext->xl_global->gcs[i] =
	    XCreateGC(htext->xl_global->dpy,
		      htext->xl_global->window, 0, NULL);

	XSetForeground(htext->xl_global->dpy,
		       htext->xl_global->gcs[i],
		       fg);

	XSetFont(htext->xl_global->dpy,
		 htext->xl_global->gcs[i],
		 s->fontinfo->fid);

	s->char_width = (int) XTextWidth(s->fontinfo, "i", 1);
    }

    /*
     * Set up graphics exposures to XCopyArea GC
     */
    XSetGraphicsExposures(display, htext->xl_global->gcs[0], True);

    /*
     * Loop all objects
     */
    p = htext->first;

    while (p) {

	/*
         * Allocate space for Xl specific object part
         */

	p->xl_data = (XlObjectData_t *) malloc(sizeof(XlObjectData_t));

	if (!p->xl_data) {
	    printf("cannot malloc in XlSetupText\n");
	    exit(1);
	}
	memset(p->xl_data, 0, sizeof(XlObjectData_t));

	/*
         * Setup Xl styles. Check Xl style only on style changes
         */
	if (!st || (st != p->style)) {

	    current = xl_get_style_and_gc(p, &currentgc, htext);

	    st = p->style;
	}
	p->xl_data->style = current;

	p->xl_data->gc = currentgc;

	/*
         * Set up text dimensions. Use 'XTextExtents()'.
         */
	p->width = (int) XTextWidth(current->fontinfo,
				    (const char *) p->data,
				    p->length);

	p->height = current->fontinfo->ascent + current->fontinfo->descent;

	/*
         * Next
         */

	/*
         * Check if this object is 'junk'-object and delete it if needed.
         * Has to be here, because we need styles.
         */
	if (xl_junk_object(p)) {
	    HTextObject_t *prev = p->prev;

	    /*
             * Set links
             */
	    if (prev) {

		prev->next = p->next;

	    } else {

		htext->first = p->next;

	    }

	    if (p->next) {

		p->next->prev = prev;

	    } else {

		htext->last = prev;

	    }

	    /*
             * Free everything allocated
             */
	    if (p->data)
		free(p->data);

	    free(p->xl_data);

	    free(p);
	}
	p = p->next;

    }

    /*
     * Cannot fail ;)
     */
    return 0;
}



/*
 * If this object is not needed, it is 'junk' -object
 */
int xl_junk_object(p)
HTextObject_t *p;
{
    register int i;

    if (p->paragraph)
	return 0;

    if (xl_object_mode(p) & STYLE_RAW)
	return 0;

    for (i = 0; i < p->length; i++)
	if (p->data[i] != ' ')
	    return 0;

    return 1;
}
