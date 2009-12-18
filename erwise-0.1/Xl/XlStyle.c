/*
 * XlStyle.c --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Thu Mar  5 00:56:46 1992 tvr
 * Last modified: Wed May 13 06:21:48 1992 tvr
 *
 */

#include <stdio.h>
#include <X11/Xlib.h>

#include "HTAnchor.h"
#include "HTStyle.h"
#include "../HText/HText.h"

#include "Xl.h"
#include "XlStyle.h"
#include "XlTypes.h"
#include "XlFormatText.h"

#include "XlDefaultFonts.h"

#include "XlConfig.h"

int xl_object_mode();

/*
 * All hypertext styles. Also needed configure data (font configure-name)
 */

#define CREATE_STYLE(font, cname, tag, mode, first, left, right) \
{NULL, font, 0, cname, tag, 0, mode, first, left, right, 140, 100, 100, 50}

XlStyle_t XlStyles[] =
{
    CREATE_STYLE(FONT_NORMAL,
		 C_FONT_NORMAL,
		 "P",
		 STYLE_LEFTIFY,
		 0, 0, 100),

    CREATE_STYLE(FONT_LIST,
		 C_FONT_LIST,
		 "UL",
		 STYLE_LEFTIFY,
		 0, 5, 95),

    CREATE_STYLE(FONT_LISTCOMPACT,
		 C_FONT_LISTCOMPACT,
		 "ULC",
		 STYLE_LEFTIFY,
		 0, 5, 95),

    CREATE_STYLE(FONT_GLOSSARY,
		 C_FONT_GLOSSARY,
		 "DL",
		 STYLE_LEFTIFY | STYLE_CHANGE_REGION_ON_TAB,
		 0, 0, 100),

    CREATE_STYLE(FONT_GLOSSARYCOMPACT,
		 C_FONT_GLOSSARYCOMPACT,
		 "DLC",
		 STYLE_LEFTIFY | STYLE_CHANGE_REGION_ON_TAB,
		 0, 0, 100),

    CREATE_STYLE(FONT_EXAMPLE,
		 C_FONT_EXAMPLE,
		 "XMP",
		 STYLE_LEFTIFY | STYLE_RAW,
		 0, 0, 100),

    CREATE_STYLE(FONT_LISTING,
		 C_FONT_LISTING,
		 "LISTING",
		 STYLE_LEFTIFY | STYLE_RAW,
		 0, 0, 100),

    CREATE_STYLE(FONT_ADDRESS,
		 C_FONT_ADDRESS,
		 "ADDRESS",
		 STYLE_LEFTIFY,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER1,
		 C_FONT_HEADER1,
		 "H1",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER2,
		 C_FONT_HEADER2,
		 "H2",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER3,
		 C_FONT_HEADER3,
		 "H3",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER4,
		 C_FONT_HEADER4,
		 "H4",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER5,
		 C_FONT_HEADER5,
		 "H5",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER6,
		 C_FONT_HEADER6,
		 "H6",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(FONT_HEADER7,
		 C_FONT_HEADER7,
		 "H7",
		 STYLE_CENTER,
		 0, 0, 100),

    CREATE_STYLE(NULL,
		 NULL,
		 NULL,
		 0,
		 0, 0, 0),
};


/*
 * How much to increase y coordinate after this line ?
 */
/* ARGS_IN_USE */

int xl_linespace(p_start, p_end)
HTextObject_t *p_start;
HTextObject_t *p_end;
{
    if (p_start->xl_data->style->fontinfo) {

	return ((p_start->xl_data->style->fontinfo->ascent +
		 p_start->xl_data->style->fontinfo->descent) *
		p_start->xl_data->style->space_line) / 100;
    } else {

	return p_start->xl_data->style->space_line / 100;
    }
}


int xl_paragraphspace(p_start, p_end)
HTextObject_t *p_start;
HTextObject_t *p_end;
{
    if (p_start->xl_data->style->fontinfo) {

	return ((p_start->xl_data->style->fontinfo->ascent +
		 p_start->xl_data->style->fontinfo->descent) *
		p_start->xl_data->style->space_paragraph) / 100;
    } else {

	return p_start->xl_data->style->space_paragraph / 100;
    }
}


int xl_stylespace(p_start, p_end)
HTextObject_t *p_start;
HTextObject_t *p_end;
{
    if (p_start->xl_data->style->fontinfo) {

	return ((p_start->xl_data->style->fontinfo->ascent +
		 p_start->xl_data->style->fontinfo->descent) *
		p_start->xl_data->style->space_style) / 100;

    } else {

	return 1;
    }
}


int xl_stylespace_before(p_start, p_end)
HTextObject_t *p_start;
HTextObject_t *p_end;
{
    if (p_start->xl_data->style->fontinfo) {

	return ((p_start->xl_data->style->fontinfo->ascent +
		 p_start->xl_data->style->fontinfo->descent) *
		p_start->xl_data->style->space_style_before) / 100;
    } else {

	return 1;
    }
}


/*
 * How much to increase x-coordinate after this object ?
 */
/* ARGS_IN_USE */
int xl_wordgap(p)
HTextObject_t *p;
{
    if (xl_object_mode(p) & STYLE_RAW)
	return 0;
    else
	return p->xl_data->style->char_width;
}


/*
 * Return width of a character 'a'. Used to mostly when calculating
 * tabulator spaces.
 */
int xl_character_width(p)
HTextObject_t *p;
{
    return p->xl_data->style->char_width;
}


/*
 * Return style of this object
 */
int xl_object_style(p)
HTextObject_t *p;
{
    return p->xl_data->style->mode & STYLE_MASK;
}


/*
 * Return mode of this object
 */
int xl_object_mode(p)
HTextObject_t *p;
{
    return p->xl_data->style->mode;
}


/*
 * Calculate position of a coordinate according to left and right margins
 */

int xl_calc_position(p, which, left, right)
HTextObject_t *p;
int which;
int left;
int right;
{
    int r = 0;
    int delta = right - left;

    XlStyle_t *s = p->xl_data->style;

    if (which & I_LEFT) {
	r += (s->left * delta) / 100;
    }
    if (which & I_MIDDLE_L) {
	r += (MIDDLE_L_HACK * delta) / 100;
    }
    if (which & I_MIDDLE_R) {
	r += (MIDDLE_R_HACK * delta) / 100;
    }
    if (which & I_RIGHT) {
	r += (s->right * delta) / 100;
    }
    if (which & I_FIRST) {
	r += (s->indent1st * delta) / 100;
    }
    r += left;

    return r;
}
