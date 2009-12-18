/*
 * XlFormatText.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Tue Mar  3 02:54:45 1992 tvr
 * Last modified: Tue May 12 04:26:29 1992 tvr
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <limits.h>

#include "HTAnchor.h"
#include "HTStyle.h"
#include "../HText/HText.h"

#include "XlFormatText.h"

int xl_calc_position();
int xl_character_width();

#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * Set up how to format text (current left and right margins)
 */
void xl_set_margins(p, left, right, c_l, c_r, x, y, low_y, m)
HTextObject_t *p;
int left, right;
int *c_l, *c_r;
int *x, *y;
int low_y;
int m;
{
    int i;

    switch (m) {
    case MARGIN_START_STYLE:
	i = xl_object_mode(p);

	*c_l = xl_calc_position(p, I_LEFT, left, right);

	*x = xl_calc_position(p, I_LEFT | I_FIRST, left, right);

	if (i & STYLE_RAW) {
	    *c_r = INT_MAX;
	} else {

	    if (i & STYLE_CHANGE_REGION_ON_TAB) {

		*c_r = xl_calc_position(p, I_MIDDLE_L, left, right);

	    } else {

		*c_r = xl_calc_position(p, I_RIGHT, left, right);

	    }
	}
	break;

    case MARGIN_HANDLE_P_OBJECT:
	if (xl_object_mode(p) & STYLE_CHANGE_REGION_ON_TAB) {

	    *c_l = xl_calc_position(p, I_MIDDLE_R, left, right);
	    *c_r = xl_calc_position(p, I_RIGHT, left, right);
	    *y = low_y;
	    *x = xl_calc_position(p, I_MIDDLE_R | I_FIRST, left, right);

	} else {

	    *x += 60;		/* :-) */

#ifdef XL_DEBUG
	    printf("xl_set_margins(): normal tab ... barf\n");
#endif
	}
	break;

#ifdef XL_DEBUG
    default:
	printf("xl_set_margins: illegal mode\n");
#endif
    }
}


/*
 * Check how many objects belongs to this line.
 */
int xl_check_objects(pp, x_corner, y, right)
HTextObject_t **pp;
int *x_corner;
int y;
int right;
{
    HTextObject_t *p, *p_old;

    int x;

    int r = 0;

    p = *pp;

    p_old = NULL;

    x = *x_corner;

    while (!r) {
	/*
         * Region ends ?
         */
	if (!p || (p->paragraph)) {
	    r |= FORMAT_REGION_ENDS;
	}
	/*
         * End of text
         */
	if (!p) {
	    break;
	}
	/*
         * New style?
         */
	if (p_old && (p_old->style != p->style)) {
	    r |= FORMAT_NEW_STYLE;
	    r |= FORMAT_REGION_ENDS;
	}
	/*
         * Text full ?
         */
	if ((x + p->width) >= right) {
	    r |= FORMAT_NEXT_OBJECT_TOO_LONG;
	}
	/*
         * Setup object
         */
	p->x = x;
	p->y = y;

	x += p->width + xl_wordgap(p);


	if (!r) {
	    p_old = p;

	    p = p->next;
	}
    }

    /*
     * Return pointer to where to go on.
     */
    if (p_old) {
	*pp = p;
    }
    *x_corner = x;

    return r;
}

/*
 * Set new x-koordinate if line of objects is not 'leftified'
 * Set new y coordinate above all objects.
 */
void xl_modify_objects(p_start, p_end, right, mode)
HTextObject_t *p_start;
HTextObject_t *p_end;
int right;
int mode;
{
    HTextObject_t *p = p_start;
    HTextObject_t *p_last;

    int offset;

    /*
     * Set last object
     */

    if (p_start == p_end) {

#ifdef XL_DEBUG
	if (!p_start->paragraph) {
	    printf("zero object ?\n");
	}
#endif
	p_last = p_start;

    } else if (p_end) {

	p_last = p_end->prev;

    } else {

	for (p_last = p; p_last->next; p_last = p_last->next);

    }

    switch (xl_object_style(p_start)) {
    case STYLE_LEFTIFY:
	/*
         * Do nothing.
         */
	break;

    case STYLE_CENTER:

	offset = (right - (p_last->x + p_last->width)) / 2;

	for (p; p != p_end; p = p->next) {
	    p->x += offset;
	}
	break;

#ifdef XL_DEBUG
    default:
	printf("Style not supported yet\n");
#endif
    }
#if 0
    *newy = p_start->y + xl_lineheightandgap(p_start, p_end);
#endif
}


/*
 *  width is a request which can be denied (in case of ftp-text for
 *  example)
 *  Maximum width used on the page is returned. Everything else on the screen
 *  is formatted using given width.
 */
int XlFormatText(leftmargin, width, topmargin, vwidth, vheight, htext)
int leftmargin, width;
int topmargin;
int *vwidth, *vheight;
HText_t *htext;
{
    int stepped_y;

    int max_right_position = leftmargin;

    /*
     * Current x,y coordinates.
     */
    int x, y;

    /*
     * When there are many regions next to other, store starting
     * upper and lower levels
     */
    int low_y = 0;
    int high_y = 0;

    /*
     * current format region
     */
    int current_left;
    int current_right;

    /*
     * Point out line
     */
    HTextObject_t *p = htext->first;

    HTextObject_t *p_start = 0;

    /*
     * Should check ascend (if there is any text)
     */
    if (p) {

	low_y = high_y = y = topmargin;

	xl_set_margins(p, leftmargin, leftmargin + width,
		       &current_left, &current_right,
		       &x, &y, low_y, MARGIN_START_STYLE);
    }
    /*
     * Loop through text
     */
    while (p) {
	int mode;

	int high_y_old;

	stepped_y = 0;

	high_y_old = y;

	/*
         * What objects are on this line ?
         */
	p_start = p;

	mode = xl_check_objects(&p, &x, y, current_right);

	/*
         * Check for zero objects. This prevents endless loop and makes
         * sure this object will be modified
         */
	if ((p == p_start) && p && !p->paragraph &&
	    (mode & FORMAT_NEXT_OBJECT_TOO_LONG)) {
	    mode |= FORMAT_ONLY_OBJECT_TOO_LONG;
	    p = p->next;
	}
	/*
         * Update object positions if needs justifying etc
         */
	xl_modify_objects(p_start, p, current_right, mode);

	/*
         * Check maximum object x position
         */

	if (p && p->prev &&
	    ((p->prev->x + p->prev->width) > max_right_position)) {
	    max_right_position = p->prev->x + p->prev->width;
	}
	if (xl_object_mode(p_start) & STYLE_RAW) {
	    /*
             * Raw style HTextObject paragraphs
             */

	    if (p && p_start->paragraph) {
		p = p->next;

	    }
	    switch (p_start->paragraph) {
	    case HTEXT_NEWLINE:
		/*
	         * New line
	         */
		x = current_left;

		y += xl_linespace(p_start, p);

		low_y = high_y = MAX(y, high_y);
		break;

	    case HTEXT_TAB:
		/*
	         * Tabulator
	         */
#if 0
		{
		    int i = 0;
		    HTextObject_t *pp;

		    for (pp = p_start; pp != p; pp = pp->next) {
			switch (pp->paragraph) {
			case 0:
			    i += pp->length;
			    break;

			case HTEXT_TAB:
			    i = 0;
			}
		    }

		    i = 8 - (i % 8);

		    x += xl_character_width(p_start) * i;
		}
#endif
		{
		    int i;
		    int cl = xl_character_width(p_start);

		    i = ((x - leftmargin) % (cl * 8));

		    x += (cl * 8) - i;
		}

		break;
	    }
	} else {
	    /*
             * Hypertext style
             */

	    /*
             * Wrap
             */
	    if (mode & FORMAT_NEXT_OBJECT_TOO_LONG) {
		x = current_left;
		y += xl_linespace(p_start, p);

		high_y = MAX(y, high_y);

		if (mode & FORMAT_ONLY_OBJECT_TOO_LONG) {
		    low_y = high_y;
		}
		stepped_y++;
	    }
	    /*
             * Check paragraph objects
             */
	    if (p_start->paragraph) {
		switch (p_start->paragraph) {
		case HTEXT_NEWLINE:

		    x = current_left;
		    if (!stepped_y) {
			y += xl_linespace(p_start, p);

			stepped_y++;

			high_y = MAX(y, high_y);
		    }
		    high_y = y;

#ifdef XL_DEBUG
		    printf("XlFormatText(): there should not be newlines in htext\n");
#endif
		    break;

		case HTEXT_PARAGRAPH:

		    if (!stepped_y) {
			y += xl_linespace(p_start, p);
			y += xl_paragraphspace(p_start, p);

			stepped_y++;
		    }
		    low_y = high_y = y;

		    xl_set_margins(p, leftmargin, leftmargin + width,
				   &current_left, &current_right,
				   &x, &y, low_y, MARGIN_START_STYLE);
		    break;

		case HTEXT_TAB:

		    xl_set_margins(p, leftmargin, leftmargin + width,
				   &current_left, &current_right, &x, &y,
				   low_y, MARGIN_HANDLE_P_OBJECT);
		    break;

		case HTEXT_CONTINUE:
		    printf("format: dont understand continued objects here\n");
		    exit(1);
		    break;

		}

		p = p->next;

	    }
	}

	/*
         * Make sure paragraph endings happen
         */
	if (p && (p->style != p_start->style)) {
	    mode |= FORMAT_NEW_STYLE;
	}
	/*
         * Check for new style
         */
	if (mode & FORMAT_NEW_STYLE) {
	    /*
             * Make sure one and only line is advanced always
             */
	    if (y < high_y) {
		if (!stepped_y) {
		    high_y += xl_linespace(p_start, p);
		}
		high_y += xl_stylespace(p_start, p);

		y = low_y = high_y;

		stepped_y++;
	    } else {
		if (high_y_old == high_y) {
		    high_y += xl_linespace(p_start, p);
		}
		if (p) {
		    high_y += xl_stylespace_before(p, p);
		}
		high_y += xl_stylespace(p_start, p);

		y = low_y = high_y;

		stepped_y++;
	    }

	    xl_set_margins(p, leftmargin, leftmargin + width,
			   &current_left, &current_right,
			   &x, &y, low_y, MARGIN_START_STYLE);
	}
    }

#if 0
    /*
     * Check region ending
     */
    /* tab sivulle tai tab sivulleyl|s depending on object and so on */
    /* p = p->next */
    /* set regions with margin_handle_p_object */
    /* check where to start (y, x). x = regionstart or tab */

    if ((mode & FORMAT_REGION_ENDS) && p) {
	int i;

	i = p->paragraph;

	switch (i) {
	case HTEXT_PARAGRAPH:
	    high_y += xl_lineheightandgap(p_start, p);

	    y = low_y = high_y;

	    xl_set_margins(p, leftmargin, leftmargin + width,
			   &current_left, &current_right,
			   &x, &y, low_y, MARGIN_START_STYLE);
	    break;

	case HTEXT_NEWLINE:
	    /*high_y += xl_lineheightandgap(p_start, p);*/
	    y = low_y = high_y;
	    x = current_left;

	    break;

	case HTEXT_TAB:
	    xl_set_margins(p, leftmargin, leftmargin + width,
			   &current_left, &current_right, &x, &y, low_y,
			   MARGIN_HANDLE_P_OBJECT);
	    break;

	case HTEXT_CONTINUE:
	    printf("format: dont understand continued objects here\n");
	    exit(1);
	    break;

	}

	if (i) {
	    p = p->next;
	}
    }
    /*
     * Check New style
     */
    if (mode & FORMAT_NEW_STYLE) {
	high_y = low_y = y;

	xl_set_margins(p, leftmargin, leftmargin + width,
		       &current_left, &current_right,
		       &x, &y, low_y, MARGIN_START_STYLE);
    }
#endif

    /*
     * Give width and height to caller
     */

    if (!stepped_y) {
	if (p_start)
	    y += xl_linespace(p_start, p);
	else
	    y = 1;
    }
    *vwidth = MAX(max_right_position, width);

    *vheight = y;

    return 0;
}
