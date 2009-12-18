/*
 * Copyright 1990-1995 Pei-Yuan Wei.	All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */
/*
 * tfed.c 
 *
 * This code is still cursed and hashly evil. Read it at your own risk....
 *
 * Jon 6/17/93: added keyword search. but not usable yet.
 * Jon 6/17/93: fixed cut-buffer hightlighting, so it doesn't flicker.
 * Scott 7/?/93: added motif toolkit related event loop things
 */
#include "utils.h"
#include <ctype.h>
#include "hash.h"
#include "obj.h"
#include "sys.h"
#include "packet.h"
#include "membership.h"
#include "class.h"
#include "slotaccess.h"
#include "classlist.h"
#include "mystrings.h"
#include "glib.h"
#include "slib.h"
#include "misc.h"
#include "event.h"
#include "attr.h"
#include "cexec.h"
#include "tfed.h"
#include "cl_txtDisp.h"
#include "tfed_i.h"

int MSTAT_tfed = 0;
int MSTAT_tfed_convertNodeLinesToStr = 0;

MethodInfo defaultKeyBinding[] =
{
	CARRIAGE,	kbf_newline,
	CTRL_QUESTION,	kbf_dump,
	CTRL_a,		kbf_beginning_of_line,
	CTRL_b,		kbf_backward_char,
	CTRL_d,		kbf_delete_char,
	CTRL_e,		kbf_end_of_line,
	CTRL_f,		kbf_forward_char,
	CTRL_h,		kbf_delete_backward_char,
	CTRL_t,		kbf_join_line,
	CTRL_k,		kbf_kill_line,
	CTRL_l,		kbf_refresh,
	CTRL_n,		kbf_next_line,
	CTRL_o,		kbf_open_line,
	CTRL_p,		kbf_previous_line,
	CTRL_q,		kbf_scroll_up_line,
	CTRL_u,		kbf_delete_line,
	CTRL_v,		kbf_scroll_up,
	CTRL_w,		kbf_scroll_down,
	CTRL_x,		kbf_scroll_up,
	CTRL_y,		kbf_insert_yank,
	CTRL_z,		kbf_scroll_down_line,
	DELETE,		kbf_delete_backward_char,
/*	ESC,		kbf_esc_prefix,*/
	RETURN,		kbf_newline,
	TAB,		kbf_ident,
	NULL,		NULL
};

int (*kbflookup[128])();
char sbuff[128];
int buffi;

/*  
 * VT100 sequence for reversing and unreverse video 
 */
char enterReverse_vt100[] = {0};
char leaveReverse_vt100[] = {0};

TFChar TFC_ARRAY_NEWLINE[] = {
	{'\n', 0, 0, 0},
	{'\0', 0, 0, 0}
};

TFChar TFC_ARRAY_SPACES[] = {
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{' ', 0, 0, 0},
	{'\0', 0, 0, 0}
};

int refreshMode; 
int updateShown; 

TFChar tfcEditStr[TFCBUFF_SIZE];
TFChar tfcYankStr[TFCBUFF_SIZE];
TFChar tfcBuffStr[TFCBUFF_SIZE];
TFLineNode *theEditLN;	/* editing buffer		*/
TFLineNode *theYankLN;	/* cut/paste/kill/yank buffer 	*/
TFLineNode *theBuffLN;	/* very temporary buffer	*/

int tfed_anchorPatchIdx = 0;
char **tfed_anchorPatch[ANCHOR_LINES_SPAN];
int tfed_anchorInP = 0;

TFPic *dunselPic;

XTextItem xcharitem;	/* used by drawChar() */


/****************************************************************************
 * externally referenable routines: 
 */
/* something... */

/*
 * call on program startup 
 */
int init_tfed()
{
	int i;

/*	tfed_mg = newMemoryGroup(11024);*/

	/*
	 * bind key to functions 
	 */
	for (i = 0; i < 128; i++) kbflookup[i] = NULL;
	for (i = 0; defaultKeyBinding[i].id; i++)
		kbflookup[defaultKeyBinding[i].id] = 
		  defaultKeyBinding[i].method;

	/*
	 * used by drawChar()
	 */
	xcharitem.chars = (char*)malloc(sizeof(char));
	xcharitem.nchars = 1;
	xcharitem.delta = 0;

	/*
 	 * initialize the editing buffer shared by multiple text fields 
	 */
	theEditLN = (TFLineNode*)malloc(sizeof(struct TFLineNode));
	theEditLN->prev = NULL; /* never used */
	theEditLN->next = NULL; /* never used */
	theEditLN->linep = tfcEditStr;
	theEditLN->maxFontHeight = 0;
	theEditLN->maxFontDescent = 0;
	theEditLN->length = 0;
	theEditLN->breakc = 1;
	theEditLN->breakcIsValid = 0;
	theEditLN->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					      * TAGINFO_SIZE);
	bzero(theEditLN->tagInfo, sizeof(struct TagInfo) * TAGINFO_SIZE);
	theEditLN->tagInfoCount = TAGINFO_SIZE;

	/*
 	 * initialize the yank buffer shared by multiple text fields 
	 */
	theBuffLN = (TFLineNode *)malloc(sizeof(struct TFLineNode));
	theBuffLN->prev = NULL; /* never used */
	theBuffLN->next = NULL; /* never used */
	theBuffLN->linep = tfcBuffStr;
	theBuffLN->maxFontHeight = 0;
	theBuffLN->maxFontDescent = 0;
	theBuffLN->length = 0;
	theBuffLN->breakc = 1;
	theBuffLN->breakcIsValid = 0;
	theBuffLN->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					      * TAGINFO_SIZE);
	bzero(theBuffLN->tagInfo, sizeof(struct TagInfo) * TAGINFO_SIZE);
	theBuffLN->tagInfoCount = TAGINFO_SIZE;

	/*
 	 * initialize the yank buffer shared by multiple text fields 
	 */
	theYankLN = (TFLineNode *)malloc(sizeof(struct TFLineNode));
	theYankLN->prev = NULL; /* never used */
	theYankLN->next = NULL; /* never used */
	theYankLN->linep = tfcYankStr;
	theYankLN->maxFontHeight = 0;
	theYankLN->maxFontDescent = 0;
	theYankLN->length = 0;
	theYankLN->breakc = 1;
	theYankLN->breakcIsValid = 0;
	theYankLN->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					      * TAGINFO_SIZE);
	bzero(theYankLN->tagInfo, sizeof(struct TagInfo) * TAGINFO_SIZE);
	theYankLN->tagInfoCount = TAGINFO_SIZE;

	if (!violaPixmap) exit(123); /* call glib_init first! */

	dunselPic = (TFPic*)malloc(sizeof(struct TFPic));
	dunselPic->id = 0;
	dunselPic->type = TFPic_XBML;
	dunselPic->width = violaIcon_width;
	dunselPic->height = violaIcon_height;
	dunselPic->data = violaPixmap;
	dunselPic->canFree = 0; /* others may point to here */
	dunselPic->next = NULL;

	return 1;
}

/* 
 * initializes and sets up a text field structure 
 */
TFStruct *tfed_setUpTFStruct(self, text)
	VObj *self;
	char *text;
{
	TFStruct *tf = GET__TFStruct(self);

	if (!tf) {
		tf = (TFStruct*)Vmalloc(GET__memoryGroup(self), 
					sizeof(struct TFStruct));
		if (!tf) {
			fprintf(stderr, "malloc failed\n");
			return 0;
		}
		SET__TFStruct(self, tf);

		tf->firstp =
		tf->lastp =
		tf->offsetp =
		tf->currentp = NULL;

		tf->current_col_sticky = tf->current_col = 0;
		tf->current_row = 0;

		tf->screen_col_offset =
		tf->screen_row_offset = 0;

		tf->lineNodeCount = 0;
		tf->lineVisibleCount = 0;

		tf->num_of_lines = 0;

		tf->w = NULL;

		tf->xUL = PXX;
		tf->yUL = PYY;

		tf->xLR = tf->yLR =
		tf->width = tf->height = 0;

		tf->esc_toggle = 0;
		tf->bufferUsed = 0;
		tf->self = self;
		tf->fontID = 0;
		tf->currentFontID = fontID_normal;

		tf->cursorTimeInfo = 0;
		tf->cursorIsVisible = 0;
/*		tf->cursorBlinkDelay = 250;*/
		tf->cursorBlinkDelay = -1; /* means don't blink */
		tf->highLiteFrom_cx = -1;
		tf->highLiteFrom_cy = -1;
		tf->highLiteTo_cx = -1;
		tf->highLiteTo_cy = -1;

		tf->isRenderAble = 0;

		tf->shownPosition = 0;
		tf->shownSize = 0;

		tf->building_maxFontHeight = 0;
		tf->building_maxFontDescent = 0;
		tf->building_vspan = 0;

		tf->search_len = 0;
		tf->search_x = 0;
		tf->search_y = 0;

		tf->pics = NULL;

		/* could be better. !make an (inheritable) flag in class def */
		tf->editableP = (GET__classInfo(self) == &class_txtEdit ? 1:0);

/*		tf->mg = newMemoryGroup(1024);*/
		tf->mg = GET__memoryGroup(self);

		tf->csr_px_sticky = tf->csr_px = 0;
		tf->csr_py = tf->yUL;

		tf->align = GET__paneConfig(self);
	}
	tf->wrap = GET_wrap(self);

	tfed_updateTFStruct(self, text);

	if (helper_txtDisp_updateShownInfo(tf)) {
		VObjList *objl;
		for (objl = GET__shownDepend(self); objl; objl = objl->next) {
			if (objl->o) {
				sendMessage1N2int(objl->o,
					"shownInfoV", 
					GET_shownPositionV(self),
					GET_shownSizeV(self));
			}
		}
	}

	return tf;
}

TFStruct *tfed_updateTFStruct(self, text)
	VObj *self;
	char *text;
{
	TFStruct *tf = updateEStrUser(self);
	int fontID = GET__font(self);
	int newWidth, newHeight;
	int geometryChanged = 0;
	TFLineNode *currentp;

	if (!tf) {
/*		printf("tfed_updateTFStruct: tf=NULL!\n");*/
		return NULL;
	}
	/* 
	 * update geometry of the field
	 */
	/* to allow space between border and characters */
	tf->xUL = PXX;
	tf->yUL = PYY;
	tf->xLR = GET_width(self) - 2;
	tf->yLR = GET_height(self) - 2;
	newWidth = GET_width(self) - 4;
	newHeight = GET_height(self) - 4;

	if ((newWidth != tf->width) || (newHeight != tf->height)) {
		tf->width = newWidth;
		tf->height = newHeight;
		tf->csr_px = tf->xUL;
		tf->csr_py = tf->yUL;
		geometryChanged = 1;
	}

	tf->wrap = GET_wrap(self);
	tf->w = GET_window(self);
	tf->fontID = fontID;
	tf->currentFontID = fontID;

	/* 
	 * update content of the field, if specified.
	 */
	if (text && (tf->width > 2) && (tf->height > 2)) {
		TFChar tbuff[TBUFFSIZE];
		TFCBuildInfo buildInfo;

		tf->building_maxFontHeight = 0;
		tf->building_maxFontDescent = 0;
		tf->building_vspan = 0;

		if (tf->firstp) freeNodeLines(tf);

		tf->lineNodeCount = 0;
		tf->lineVisibleCount = 0;

		buildInfo.self = self;
		buildInfo.currentp = NULL;
		buildInfo.beginp = NULL;
		buildInfo.str = text; 
		buildInfo.fontID = fontID;
		buildInfo.lineNodeCountp = tf->lineNodeCount;
		buildInfo.lineVisibleCountp = tf->lineVisibleCount;
		buildInfo.tbuff = tbuff;
		buildInfo.tbuffi = 0;
		buildInfo.breaki = 0;
		buildInfo.tagID = 0;
		buildInfo.flags = 0;
		buildInfo.spaceWidth = FontWidths(fontID)[' '];

		buildInfo.maxFontHeight = FontMaxHeight(fontID);
		buildInfo.maxFontDescent = FontDescent(fontID);

		buildInfo.px = 0; 
		buildInfo.makeLinep = 1;
		buildInfo.verbatim = GET_verbatim(self);
		buildInfo.format = GET_format(self);
		buildInfo.vspan = 0;
		buildInfo.pics = NULL;

		tfed_buildLines(&buildInfo);

		tf->firstp = buildInfo.beginp;
		tf->offsetp = buildInfo.beginp;
		tf->currentp = buildInfo.beginp;
		tf->current_row = 0;
		tf->current_col_sticky = tf->current_col = 0;

		tf->building_vspan = buildInfo.vspan; 
/*WRONG!!
		if (tf->building_vspan > tf->yLR)
			tf->yLR = tf->building_vspan;
*/
		tf->lineNodeCount = buildInfo.lineNodeCountp;
		tf->lineVisibleCount = buildInfo.lineVisibleCountp;
		tf->screen_col_offset = 0;
		tf->screen_row_offset = 0;
		tf->csr_px_sticky = tf->csr_px = tf->yUL;
		tf->csr_py = tf->yUL;

		SET__content(self, buildInfo.pics);

/*dumpNodeLines(tf);*/

	}
	tf = updateEStrUser(self);

	if (tf->editableP)
	  replaceNodeLine(theEditLN, tf->currentp, 0, NULL);

	tf->isRenderAble = (TFWINDOW && newWidth > 2 && newHeight > 2) ? 1 : 0;

	if (geometryChanged && tf->isRenderAble) {
		VObjList *objl;

		scanVerticalMetrics(tf);
		renderTF(tf);
		helper_txtDisp_updateShownInfo(tf);
		for (objl = GET__shownDepend(self); 
			objl; objl = objl->next) {
			if (objl->o)
				sendMessage1N2int(objl->o,
					"shownInfoV", 
					GET_shownPositionV(self),
					GET_shownSizeV(self));
		}
	}
	return tf;
}

TFStruct *tfed_clone(orig, clone)
	VObj *orig, *clone;
{
	TFStruct *newtf;
	TFLineNode *prevp, *currentp, *newLN;
	TFChar *newChs;
	char *cp;
	int i, sz;
	TFStruct *origtf = GET__TFStruct(orig);
	TagInfo *ti, *cti;

	if (!origtf) return NULL;
	newtf = (TFStruct*)Vmalloc(GET__memoryGroup(clone), 
					sizeof(struct TFStruct));
	if (!newtf) {
		perror("malloc");
		return NULL;
	}
	bcopy(origtf, newtf, sizeof(struct TFStruct));

	newtf->firstp = NULL;
	newtf->offsetp = NULL;
	newtf->currentp = NULL;

	newtf->mg = GET__memoryGroup(clone);

	prevp = NULL;
	if (origtf->firstp) {
		currentp = origtf->firstp;
		while (currentp) {
			newLN = (TFLineNode*)Vmalloc(newtf->mg, 
							sizeof(TFLineNode));
			if (!newLN) return 0;
			sz = sizeof(TFChar) * (currentp->length + 1);
			newChs = (TFChar*)Vmalloc(newtf->mg, sz);
/*MSTAT_tfed+=sz;*/
			if (!newChs) return 0;
			bcopy(currentp, newLN, sizeof(TFLineNode));
			newLN->linep = newChs;
			newLN->length = currentp->length;
			bcopy(currentp->linep, newChs, sz);

			if (prevp) {
				prevp->next = newLN;
				newLN->prev = prevp;
			} else {
				newLN->prev = NULL;
			}

			if (currentp == origtf->firstp) 
				newtf->firstp = newLN; 
			if (currentp == origtf->offsetp) 
				newtf->offsetp = newLN; 
			if (currentp == origtf->currentp) 
				newtf->currentp = newLN; 

			cti = currentp->tagInfo;
			if (cti) 
			    for (i = currentp->tagInfoCount - 1; i >= 0; i--) {
				ti = &(newLN->tagInfo[i]);
				cp = cti[i].info;
				if (cp) {
					ti->info = VSaveString(newtf->mg, cp);
				} else {
					ti->info = NULL;
				}
/*				ti->x_begin = 0;
				ti->x_end = 0;
				ti->valid = 0;
*/
			    }
			prevp = newLN;
			currentp = currentp->next;
		}
	}
	return newtf;
}

char s[2];

int glyphWidth(tf, tfcp)
	TFStruct *tf;
	TFChar *tfcp;
{
	char c = TFCChar(tfcp);

	if ((c >= ' ') && (c != '\\')) {
		if (TFCFlags(tfcp) & MASK_PIC) {
		  	TFPic *pic = NULL, *picp, *pics;
			int picID;

			picID = TFCFontID(tfcp);
			picp = (TFPic*)GET__content(tf->self);

			if (!picp) {
			  VObj *parent = GET__parent(tf->self);
			  if (parent) picp = (TFPic*)GET__content(parent);
			}
			for (; picp; picp = picp->next)
				if (picp->id == picID) {
					pic = picp;
					break;
				}
			if (!pic) pic = dunselPic;
			return pic->width;
		}
		return TFCWidth(tfcp);
	} else {
		/* high ick */
		s[0] = c;
		return XTextWidth(fontInfo[TFCFontID(tfcp)].fontStruct, s, 1);
	}
}

/*
 * renderTextField - render text within the param of self.
 * returns: 1 for success, 0 for failure.
 */
int tfed_render(self)
	VObj *self;
{
	TFStruct *tf = updateEStrUser(self);

	if (tf) {
/*		replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);*/
	  	if (tf->w && tf->isRenderAble) renderTF(tf);
		return 1;
	}
	return 0;
}

update_tf_color(tf)
        TFStruct *tf;
{
	GLClearSelection();
	GLPrepareObjColor(tf->self);

	if (BDPixel == BGPixel) {
	    XSetForeground(display, gc_mesh, FGPixel);
	    XSetBackground(display, gc_mesh, BGPixel);
	    XSetForeground(display, gc_bd, FGPixel);
	} else {
	    XSetForeground(display, gc_mesh, BDPixel);
	}
}

/* mode: 0 just highlite
 *       1 delete
 */
updateHilite(tf, x1, y1, x2, y2, mode)
        TFStruct *tf;
        int x1, y1, x2, y2;
	int mode;
{
        int aux;
	unsigned long newstart, newend;
	unsigned long oldstart, oldend;

#define Swap(x, y) { \
        aux = x;     \
        x = y;       \
        y = aux;     \
}

#define LINE_MULTIPLIER 8192
/*
 * LINE_MULTIPLIER should be something bigger than the biggest width
 * of a text field we ever expect highlighting to work on, and maybe
 * also a power of 2 so that things will go fast.
 *
 */

	newstart = y1 * LINE_MULTIPLIER + x1;
	newend = y2 * LINE_MULTIPLIER + x2;

	/*
	 * Make sure the coordinates are given in "forward" order.
	 *
	 */
        if (newend < newstart) {
	    Swap(x1, x2);
	    Swap(y1, y2);
	    Swap(newstart, newend);  /* not kosher */
	}

	oldstart = tf->highLiteFrom_cy * LINE_MULTIPLIER +
	           tf->highLiteFrom_cx;

	oldend = tf->highLiteTo_cy * LINE_MULTIPLIER +
	           tf->highLiteTo_cx;

	if (oldstart < 0)   /* no previous hilite */
	    goto just_draw_new;

	if ((oldstart == newstart) && (oldend == newend)) {
	    /* no work */
	    return;
	}

	if (oldend < oldstart) {
	    Swap(tf->highLiteFrom_cx, tf->highLiteTo_cx);
	    Swap(tf->highLiteFrom_cy, tf->highLiteTo_cy);
	    Swap(oldstart, oldend);     /* again, not kosher */
	}

	update_tf_color(tf);

	if ((newstart >= oldend) || (newend <= oldstart)) {
	    goto totally_separate;
	}
	if (newstart < oldstart) {
	    /* We have some new highlighting at the beginning of the region */
	    rangeOperation(tf, x1, y1, tf->highLiteFrom_cx,
			       tf->highLiteFrom_cy, -1, 1, 0);
	} else if (newstart > oldstart) {
	    rangeOperation(tf, tf->highLiteFrom_cx, tf->highLiteFrom_cy,
			       x1, y1, 1, -1, 0);
	}

	if (newend < oldend) {
	    rangeOperation(tf, x2, y2, tf->highLiteTo_cx,
			               tf->highLiteTo_cy, 1, -1, 0);
	} else if (newend > oldend) {
	        rangeOperation(tf, tf->highLiteTo_cx, tf->highLiteTo_cy,
			           x2, y2, -1, 1, 0);
	}

    goto done;
	    
    totally_separate:
	    /* Erase old hilite */
	rangeOperation(tf, tf->highLiteFrom_cx, tf->highLiteFrom_cy,
		           tf->highLiteTo_cx, tf->highLiteTo_cy, 1, -1, 0);
    just_draw_new:
	    /* Do the new one */
	rangeOperation(tf, x1, y1, x2, y2, -1, 1, 0);

    done:
	tf->highLiteFrom_cx = x1;
	tf->highLiteFrom_cy = y1;
	tf->highLiteTo_cx = x2;
        tf->highLiteTo_cy = y2;
}
	
char *tfed_processMouseMove(self)
	VObj *self;
{
	TFStruct *tf = updateEStrUser(self);
        TFLineNode *ln;
	TFChar *tfcp;
	int rootx, rooty, mx, my, px, py, cx, cy, scr_off_y;
	int i;
	char *url;
	TagInfo *ti;

	if (!tf) return 0;
	if (!TFWINDOW) return 0;

	/* dude, this could be faster */

	GLQueryMouse(TFWINDOW, &rootx, &rooty, &mx, &my);
	mapFromPixelToCharPosition(tf, mx, my, &cx, &scr_off_y, &px, &py);
/*	cy = scr_off_y + tf->screen_row_offset;
*/
/*	printf("tfed mouse movement: row,col: %d(%d),%d\n", cy, scr_off_y, cx);
*/
	ln = tf->offsetp;
	for (i = 0; i < scr_off_y; i++) {
		ln = ln->next;
		if (ln == NULL) return NULL;
	}
	tfcp = ln->linep + cx;

	if (!(TFCFlags(tfcp) & MASK_BUTTON)) return NULL;

	/* search forward to find an tag */
	for (; TFCChar(tfcp); tfcp++)
		if (TFCTagID(tfcp)) {
			url = (char*)ln->tagInfo[TFCTagID(tfcp)].info;
			if (url) return decodeURL(url);
			return NULL;
		}
/*
	ti = ln->tagInfo;
	for (i = 1; i < ln->tagInfoCount; i++) {
		printf("tagInfo[%d] begin=%d end=%d valid=%d url={%s}\n",
			i, ti->x_begin, ti->x_end, ti->valid,
			ti->info ? ti->info : "NULL");
		if (px >= ti->x_begin && px <= ti->x_end) {
			printf("BINGO tagInfo[%d]: info={%s}\n",
				ti->info ? ti->info : "NULL");
			break;
		}
	}
*/
	return NULL;
}

int tfed_processMouseInput(self)
	VObj *self;
{
	TFStruct *tf = updateEStrUser(self);
	int rootx, rooty;
	int mx, my;
	int px, py;
	int prev_cx = -1, prev_cy = -1;
	int init_cx, init_cy;
	int cx, cy;
	XEvent e;
	int stat;
	int initColor = 1;
	int doHighLighting;

	if (!tf) return 0;
	if (!TFWINDOW) return 0;

	/* Scott */
	/*
	 * This avoids locking up the server in case something else in
	 * the application has the pointer grabbed  (like menu widgets).
	 */
	if (GrabSuccess != XGrabPointer(display, TFWINDOW, TRUE, 0,
					GrabModeAsync, GrabModeAsync,
					None, None, CurrentTime))
	    return 1;
	XUngrabPointer(display, CurrentTime);

	doHighLighting = GET_cursor(self);

	GLQueryMouse(TFWINDOW, &rootx, &rooty, &mx, &my);
	mapFromPixelToCharPosition(tf, mx, my, &init_cx, &init_cy, &px, &py);
 	init_cy += tf->screen_row_offset;

	tfed_placeCursor(self, mx, my);

	for (;;) {
		XNextEvent(display, &e);

		switch (eventType(e)) {
		case ButtonPress:
/*never happens. event already caught before coming here.
*/			process_event(&e, ACTION_TOOL);

		/*tfed_searchAndHighlightStringFromPoint(tf, "viola");*/
			return stat;
		break;
		case ButtonRelease:

			if (findWindowObject(e.xany.window))
			    process_event(&e, ACTION_TOOL);

			mapFromPixelToCharPosition(tf, mx, my, 
					&cx, &cy, &px, &py);
			cy += tf->screen_row_offset;

/*			printf("clip row,col: %d,%d - %d,%d\n",
				init_cy, init_cx, cy, cx);
*/
			tf->highLiteFrom_cx = init_cx;
			tf->highLiteFrom_cy = init_cy;
			tf->highLiteTo_cx = cx;
			tf->highLiteTo_cy = cy;

			GLSetSelection(self, NULL);

			stat = tfed_placeCursor(self, mx, my);

			return stat;
		break;
		case MotionNotify:
/*		if (((XMotionEvent*)&e)->is_hint != NotifyHint) break;*/

			GLQueryMouse(TFWINDOW, &rootx, &rooty, &mx, &my);
			mapFromPixelToCharPosition(tf, mx, my, 
							&cx, &cy, &px, &py);
			cy += tf->screen_row_offset;

#ifdef NOT
			if (initColor) {
				GLClearSelection();

				GLPrepareObjColor(self);
				if (BDPixel == BGPixel) {
				XSetForeground(display, 
						gc_mesh, FGPixel);
					XSetBackground(display, 
						gc_mesh, BGPixel);
					XSetForeground(display, 
						gc_bd, FGPixel);
				} else {
					XSetForeground(display, 
						gc_mesh, BDPixel);
				}
				initColor = 0;
			}
#endif NOT
			if (cx != prev_cx || cy != prev_cy) {
/*				printf("range row,col: %d,%d - %d,%d\n",
					init_cy, init_cx, cy, cx);
*/
				if (prev_cx != -1 && prev_cy != -1) 
				        updateHilite(tf, init_cx, init_cy,
						     prev_cx, prev_cy, 0);
				updateHilite(tf, init_cx, init_cy, cx, cy, 0);
				prev_cx = cx;
				prev_cy = cy;
			}
		/*tfed_searchAndHighlightStringFromPoint(tf, "viola");*/
		break;
		default:
			if (findWindowObject(e.xany.window))
				process_event(&e, ACTION_TOOL);
		break;
		}
	}
}

char *tfed_getSelection(self)
	VObj *self;
{
	TFStruct *tf = GET__TFStruct(self);
	extern VObj *xselectionObj;

	if (tf) return rangeOperation(tf, 
				tf->highLiteFrom_cx, tf->highLiteFrom_cy,
				tf->highLiteTo_cx, tf->highLiteTo_cy,
				0, 0, 1);
	return NULL;
}

int tfed_clearSelection(self)
	VObj *self;
{
	TFStruct *tf = GET__TFStruct(self);

	if (tf->highLiteFrom_cx != -1) {
		GLPrepareObjColor(self);
		xselectionObj = NULL;
		rangeOperation(tf, 
				tf->highLiteFrom_cx, tf->highLiteFrom_cy,
				tf->highLiteTo_cx, tf->highLiteTo_cy,
				1, -1, 0);
		tf->highLiteFrom_cx = 
		tf->highLiteFrom_cy =
		tf->highLiteTo_cx = 
		tf->highLiteTo_cy = -1;
	}
	return 1;
}

int tfed_placeCursor(self, mx, my)
	VObj *self;
	int mx, my;
{
	TFStruct *tf = updateEStrUser(self);
	TFChar *tfcp;
	int doDrawCursor = GET_cursor(self);

	if (!tf) {
/*		printf("Internal error: tfed_placeCursor(): tf == NULL\n");*/
		return 0;
	}
	if (mx > tf->xUL && mx < tf->xLR) {
		if (my > tf->yUL && my < tf->yLR) {
			int cx, cy, actual_row;

			if (doDrawCursor) TimedEraseCursor(self, NULL, NULL);

			if (tf->editableP)
			  replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);

			mapFromPixelToCharPosition(tf, mx, my, &cx, &cy, 
						&(tf->csr_px), &(tf->csr_py));

			tf->currentp = tf->offsetp;
			for (actual_row = 0; actual_row < cy; actual_row++) {
				if (tf->currentp->next)
					tf->currentp = tf->currentp->next;
				else
					break; /* stop at final line */
			}
			tf->current_col_sticky = tf->current_col = cx;
			tf->current_row = tf->screen_row_offset + actual_row;

			if (tf->editableP)
			  replaceNodeLine(theEditLN, tf->currentp, 0, NULL);

			setCurrentFontID(tf);

/*			if (cursorWithinField(tf)) placeCursorWithinStr(tf);*/

			if (doDrawCursor) TimedDrawCursor(self, NULL, NULL);
/*
			if (helper_txtDisp_updateShownInfo(tf)) {
				VObjList *objl;
				for (objl = GET__shownDepend(self); 
					objl; objl = objl->next) {
				  	if (objl->o) {
						sendMessage1N2int(objl->o,
						    "shownInfoV", 
						    GET_shownPositionV(self),
						    GET_shownSizeV(self));
					}
				}
			}
*/
			return 1;
		}
	}
	return 0;
}

int tfed_processKeyEvent(self, w, c)
	VObj *self;
	Window w;
	char c;
{
	TFStruct *tf = updateEStrUser(self);
	int doDrawCursor = GET_cursor(self);
	int stat;

	if (!w) doDrawCursor = 0;
	refreshMode = CHAR;
	updateShown = 0;

	if (doDrawCursor) TimedEraseCursor(self, NULL, NULL);

	if (kbflookup[c]) {
		stat = kbflookup[c](tf);
		if (stat == 0) return 0;
	} else if (tf->esc_toggle == 1) {
		switch (c) {
		case CTRL_v:
			/* scroll down one page */
			kbf_scroll_down(tf);
		break;
		case '0':
			kbf_useFont_fixed(tf);
		break;
		case '1':
			kbf_useFont_normal(tf);
		break;
		case '2':
			kbf_useFont_normal_large(tf);
		break;
		case '3':
			kbf_useFont_normal_largest(tf);
		break;
		case '4':
			kbf_useFont_bold(tf);
		break;
		case '5':
			kbf_useFont_bold_large(tf);
		break;
		case '6':
			kbf_useFont_bold_largest(tf);
		break;
		case '9':
			kbf_useFont_context(tf);
		break;
		}
		tf->esc_toggle = tf->esc_toggle ? 0 : 1;

		/* because font change might affect lineHeight,
		 * refresh screen
		 */
		replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
		refreshMode = SCREEN;
		updateShown = 1;
		tf->bufferUsed = 1;

	} else {
		if (c == ESC) {
			/* record escape key "toggling" */
			((tf->esc_toggle == 0) ? 
				(tf->esc_toggle = 1) : (tf->esc_toggle = 0));
		} else if (isprint(c)) {
/*			printf("(%c),%d\n",c,(int)c);*/
			insertKey(tf, c, tf->currentFontID);
		}
		tf->bufferUsed = 1;
	}
/*printf("csr_px=%d csr_py=%d current_row=%d screen_row_offset=%d \n", 
	tf->csr_px, tf->csr_py, tf->current_row, tf->screen_row_offset);*/

	if (refreshMode == LINE) {
		updateShown = 1;
		if (w && cursorWithinField(tf)) {
			drawLineOffset(tf, 
				       tf->current_row - tf->screen_row_offset,
				       1);
		}
	} else if (refreshMode == SCREEN) {
		updateShown = 1;
		if (w && cursorWithinField(tf)) {
			replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
			renderTF(tf);
		}
	}
	if (updateShown) {
		if (helper_txtDisp_updateShownInfo(tf)) {
			VObjList *objl;
			for (objl = GET__shownDepend(self); 
				objl; objl = objl->next) {
				if (objl->o) {
					sendMessage1N1int(objl->o, 
						"shownPositionV", 
						GET_shownPositionV(self));
					sendMessage1N1int(objl->o, 
						"shownSizeV", 
						GET_shownSizeV(self));
				}
			}
		}
	}
	if (doDrawCursor) TimedDrawCursor(self, NULL, NULL);
/*
printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL, 
	theEditLn->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
*/
	refreshMode = 0;
	return 1;
}

int insertKey(tf, c, fontID)
	TFStruct *tf;
	char c;
	int fontID;
{
	static TFChar tfc;
	int width = FontWidths(fontID)[c];

	TFCShiftStr(theEditLN->linep, tf->current_col, 1);
	tfc.c = c;
	tfc.fontID = fontID;
	tfc.tagID = 0; 
	tfc.flags = 0; 
	TFCCopy(theEditLN->linep + tf->current_col, &tfc);

	if (theEditLN->maxFontHeight < FontMaxHeight(fontID))
		theEditLN->maxFontHeight = FontMaxHeight(fontID);
	theEditLN->length++;
	tf->current_col++;
	tf->current_col_sticky = tf->current_col;

	if ((tf->csr_px + width) < tf->xLR) {
		scrollLineForward(tf, 1, tf->current_col);
		tf->csr_px += width;
	} else {
		tf->csr_px = tf->xUL;
		tf->csr_px += width;
		if (tf->currentp) {
		  tf->csr_py += tf->currentp->maxFontHeight;
		  scrollLineForward(tf, 1, tf->current_col);
		} else {
		  fprintf(stderr, 
			"Internal error: insertKey() tf->currentp == NULL.\n");
		}
		refreshMode = SCREEN;
		updateShown = 1;
	}

	return 1;
}

/* insert tab character 
 */
int kbf_ident(tf)
	TFStruct *tf;
{
	/*XXX*/
	insertKey(tf, ' ', tf->currentFontID);
	insertKey(tf, ' ', tf->currentFontID);
	insertKey(tf, ' ', tf->currentFontID);
	insertKey(tf, ' ', tf->currentFontID);

	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* start of line 
 */
int kbf_beginning_of_line(tf)
	TFStruct *tf;
{
	tf->current_col_sticky = tf->current_col = 0;
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	placeCursorWithinStr(tf);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* end of line 
 */
int kbf_end_of_line(tf)
	TFStruct *tf;
{
	tf->current_col_sticky = tf->current_col = theEditLN->length;
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	placeCursorWithinStr(tf);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* back_space 
 */
int kbf_backward_char(tf)
	TFStruct *tf;
{
	tf->current_col -= 1;
	tf->current_col_sticky = tf->current_col;
	if (tf->current_col < 0) {
		placeCursor(tf);
	} else {
		tf->csr_px -= TFCWidth(theEditLN->linep + tf->current_col);
		if (tf->csr_px < tf->xUL) placeCursor(tf);
	}
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* forward char 
 */
int kbf_forward_char(tf)
	TFStruct *tf;
{
	tf->current_col++;
	tf->current_col_sticky = tf->current_col;
	if (tf->current_col > theEditLN->length) {
		placeCursor(tf);
	} else {
		tf->csr_px += TFCWidth(theEditLN->linep + tf->current_col - 1);
		if ((tf->csr_px + TFCWidth(theEditLN->linep + tf->current_col)) >
			tf->xLR) {
			placeCursor(tf);
		}
	}
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* pull in from right 
 */
int kbf_delete_char(tf)
	TFStruct *tf;
{
	if (theEditLN->length > 0) {
		--(theEditLN->length);
/*
		printf("kbf_delete_char(): px=%d c=[%d]'%c'\n",
		       tf->csr_px,
		       tf->current_col,
		       TFCChar(theEditLN->linep + tf->current_col));
*/
		scrollLineBackward(tf, 1, tf->current_col);
		TFCShiftStr(theEditLN->linep, tf->current_col+1, -1);
		
		/* update maxFontHeight */
/*	
	if (FontMaxHeight(TFCFontID(theEditLN->linep + tf->current_col)) 
*/

	}
/*	refreshMode = LINE;*/
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* kill rest of line, and put it in buffer 
 */
#define VERBOSE_KBF_KILL_LINE verbose
int kbf_kill_line(tf)
	TFStruct *tf;
{
	int lnA_breakc, lnB_breakc, lnAB_breakc;
	int lnB_maxFontHeight;
	int d, i, shiftSpan, eraseSpan, limit, py, probepy;
	TFLineNode *currentp;
	int offset, upper = 0;

	if (!tf->currentp) {
		SLBell();
		return 0;
	}

	lnA_breakc = countBreaks(theEditLN->linep);

	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}
	if (tf->bufferUsed) {
		TFCClear(theYankLN->linep); 
		theYankLN->length = 0;
		tf->bufferUsed = 0;
	}
	if (tf->current_col == theEditLN->length) {
		/* join current and next line, 
		 * since there's nothing to yank */
		/* printf("yank rest of line\n");*/

		if (tf->current_row >= tf->lineNodeCount - 1) {
			SLBell();
			return 0;
		}
		if (!tf->currentp->next) {
			return 0;
		}
		lnB_maxFontHeight = tf->currentp->next->maxFontHeight;
		lnB_breakc = tf->currentp->next->breakc;

		joinLine(tf);

if (VERBOSE_KBF_KILL_LINE) {
printf("@@@ theYankLN -- length=%d---\n", theYankLN->length);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theYankLN->linep, NULL,
	theYankLN->tagInfoCount);
printf("---\n");
}
		TFCstrcat(theYankLN->linep, TFC_ARRAY_NEWLINE);
		theYankLN->length++;

if (VERBOSE_KBF_KILL_LINE) {
printf("@@@ theYankLN -- length=%d---\n", theYankLN->length);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theYankLN->linep, NULL,
	theYankLN->tagInfoCount);
printf("---\n");
}
		replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
		tf->currentp->breakc = theEditLN->breakc;
		if (lnB_maxFontHeight != tf->currentp->maxFontHeight) {
			refreshMode = SCREEN;
			return 1;
		}

		lnAB_breakc = theEditLN->breakc;
		d = lnA_breakc + lnB_breakc - lnAB_breakc;

if (VERBOSE_KBF_KILL_LINE) {
printf("***** lnA_breakc = %d   lnB_breakc = %d  lnAB_breakc = %d   d=%d\n",
		lnA_breakc, lnB_breakc, lnAB_breakc, d);
}
		shiftSpan = d * theEditLN->maxFontHeight;
		eraseSpan = d * theEditLN->maxFontHeight;
		offset = tf->current_row;

if (VERBOSE_KBF_KILL_LINE) {
printf("***** shiftSpan = %d  eraseSpan = %d  offset = %d\n", 
	shiftSpan, eraseSpan, offset);
}
		for (currentp = tf->offsetp, i = tf->screen_row_offset; 
		     i < offset; i++) {
			if (!currentp) break;
			upper += currentp->maxFontHeight * currentp->breakc;
			currentp = currentp->next;
		}
		upper += lnA_breakc * theEditLN->maxFontHeight;

		if (shiftSpan > 0) {
			int safety = tf->height - upper - shiftSpan;
			if (safety > 0) {
				XCopyArea(display, TFWINDOW, TFWINDOW,
					  gc_copy, 
					  tf->xUL, tf->yUL + upper + shiftSpan,
					  tf->width, safety,
					  tf->xUL, tf->yUL + upper);
			}
			if (eraseSpan) {
if (verbose) fprintf(stdout, "tfed: 1 XClearArea w=%x %d %d %d %d\n", TFWINDOW, 
	tf->xUL, tf->yLR - eraseSpan,
	tf->width, eraseSpan);
				XClearArea(display, TFWINDOW, 
					tf->xUL, tf->yLR - eraseSpan,
					tf->width, eraseSpan, False);

			}
		}
		drawLineOffset(tf, tf->current_row - tf->screen_row_offset, 1);

	} else {
		TFChar *tfcp;
		int yoffset;

		lnB_maxFontHeight = tf->currentp->maxFontHeight;
		lnB_breakc = 0;

		/* beause tagInfo is not transfered, need to clear
		 * out tag bits when a line is copied
		 * (lest we're referencing garbage tagInfo...)
		 */

if (VERBOSE_KBF_KILL_LINE) {
	printf(">>> current_col=%d\n", tf->current_col);
	printf(">>> theYankLN->length=%d\n", theYankLN->length);
	printf(">>> theEditLN->length=%d\n", theEditLN->length);
}
		TFCstrcpy(theYankLN->linep + theYankLN->length,
				theEditLN->linep + tf->current_col);
		for (tfcp = theYankLN->linep + theYankLN->length;
		     TFCChar(tfcp); tfcp++)
			TFCFlags(tfcp) = TFCFlags(tfcp) & MASK_REVERSE;
		TFCClear(theEditLN->linep + tf->current_col);
		theYankLN->length += theEditLN->length - tf->current_col;
		theEditLN->length = tf->current_col;
		setBreaks(tf, theEditLN);

if (VERBOSE_KBF_KILL_LINE) {
	printf(">>>> current_col=%d\n", tf->current_col);
	printf(">>>> theYankLN->length=%d\n", theYankLN->length);
	printf(">>>> theEditLN->length=%d\n", theEditLN->length);
	printf(">>>> currentp->breakc=%d\n", tf->currentp->breakc);
	printf(">>>> theEditLN->breakc=%d\n", theEditLN->breakc);
}
		replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
		setBreaks(tf, tf->currentp);

		drawLineOffset(tf, tf->current_row - tf->screen_row_offset, 1);

		lnAB_breakc = theEditLN->breakc;
		d = lnA_breakc + lnB_breakc - lnAB_breakc;

if (VERBOSE_KBF_KILL_LINE) {
printf("***** lnA_breakc = %d   lnB_breakc = %d  lnAB_breakc = %d   d=%d\n",
		lnA_breakc, lnB_breakc, lnAB_breakc, d);
}
		shiftSpan = d * theEditLN->maxFontHeight;
		eraseSpan = d * theEditLN->maxFontHeight;
		offset = tf->current_row;

if (VERBOSE_KBF_KILL_LINE) {
printf("***** shiftSpan = %d  eraseSpan = %d  offset = %d\n", 
	shiftSpan, eraseSpan, offset);
}
		for (currentp = tf->offsetp, i = tf->screen_row_offset; 
		     i < offset; i++) {
			if (!currentp) break;
			upper += currentp->maxFontHeight * currentp->breakc;
			currentp = currentp->next;
		}
		upper += lnAB_breakc * theEditLN->maxFontHeight;

		if (shiftSpan > 0) {
			int safety = tf->height - upper - shiftSpan;

			if (safety > 0) {
				XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
					tf->xUL, tf->yUL + upper + shiftSpan,
					tf->width, safety,
					tf->xUL, tf->yUL + upper);
			}
			if (eraseSpan) {
if (verbose) fprintf(stdout, "tfed: 2 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
	tf->xUL, tf->yLR - eraseSpan,
	tf->width, eraseSpan);
				XClearArea(display, TFWINDOW, 
					tf->xUL, tf->yLR - eraseSpan,
					tf->width, eraseSpan, False);
			}
		}
	}

	if (shiftSpan > 0) {

		py = tf->yUL;
		i = 0;
		for (currentp = tf->offsetp; ; currentp = currentp->next) {
			if (!currentp) return 0;
			if (currentp == tf->currentp) break;
			py += currentp->maxFontHeight * currentp->breakc;
			++i;
		}
		probepy = 0;
		limit = tf->height - py - eraseSpan;

		if (VERBOSE_KBF_KILL_LINE) {
			printf("***** limit=%d  yLR=%d\n", limit, tf->yLR);
		}

		for (; ; currentp = currentp->next) {
			if (!currentp) break;
			d = currentp->maxFontHeight * currentp->breakc;
			probepy += d;

			if (VERBOSE_KBF_KILL_LINE) {
				printf("***** d=%d probepy=%d\n", d, probepy);
			}
			if (probepy >= limit) break;
			py += d;
			++i;
		}
		for (; currentp; currentp = currentp->next) {
			if (py >= tf->yLR) break;
			if (VERBOSE_KBF_KILL_LINE) {
			   printf("***** drawing line %d, py=%d\n", i, py);
			}
			drawLineOffset(tf, i, 1);
			i++;
			py += currentp->maxFontHeight * currentp->breakc;
		}
	}
	setCurrentFontID(tf);

	return 1;
}

/* insert buffer */
int kbf_insert_yank(tf)
	TFStruct *tf;
{
	insertStr(tf, tf->current_col, theYankLN);
	tf->bufferUsed = 1;
	return 1;
}

/* delete to left and pull 
 * YANKED INTO tfed2.c
 */

/* join current and the next lines 
 */
int kbf_join_line(tf)
	TFStruct *tf;
{
	joinLine(tf);
	tf->bufferUsed = 1;
	return 1;
}

/* delete current line 
 */
int kbf_delete_line(tf)
	TFStruct *tf;
{
	int col;

	if (tf->lineNodeCount <= 0) return 0;
	if (!tf->currentp) return 0;

	tf->lineNodeCount--;
	tf->lineVisibleCount -= tf->currentp->breakc;
	deleteLineNode(tf);
	if (tf->currentp) 
		if (tf->currentp->linep)
			TFCstrcpy(theEditLN->linep, tf->currentp->linep);
	col = TFCstrlen(theEditLN->linep);
	theEditLN->length = col;

	refreshMode = SCREEN;
	tf->bufferUsed = 1;

	setCurrentFontID(tf);

	return 1;
}
		
/* insert line above current line, after pushing the current line 
 * down one line 
 */
int kbf_open_line(tf)
	TFStruct *tf;
{
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	if (!tf->currentp) {
		SLBell();
		return 0;
	}
	tf->currentp = insertLineNode(tf, tf->currentp, 1);
	TFCstrcpy(theEditLN->linep, tf->currentp->linep);
	theEditLN->length = TFCstrlen(theEditLN->linep);
	tf->current_col_sticky = tf->current_col = 0;

	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	placeCursorWithinStr(tf);
	setCurrentFontID(tf);

	refreshMode = SCREEN;
	tf->bufferUsed = 1;
	return 1;
}

/* insert line below cursor
 */
int kbf_open_line_below(tf)
	TFStruct *tf;
{
	insertBelowLineNode(tf, tf->currentp, 1);
	moveLine(tf, 1);
	tf->screen_col_offset = 0;
	tf->current_col_sticky = tf->current_col = 0;
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	refreshMode = SCREEN;
	return 1;
}

/* previous line 
 */
int kbf_previous_line(tf)
	TFStruct *tf;
{
	moveLine(tf, -1);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	updateShown = 1;
	return 1;
}

/* next line 
 */
int kbf_next_line(tf)
	TFStruct *tf;
{
	moveLine(tf, 1);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	updateShown = 1;
	return 1;
}

/* scroll down one page 
 */
int kbf_scroll_down(tf)
	TFStruct *tf;
{
	moveLine(tf, -(tf->num_of_lines + 1));
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	updateShown = 1;
	return 1;
}

/* scroll up one page 
 */
int kbf_scroll_up(tf)
	TFStruct *tf;
{
	moveLine(tf, tf->num_of_lines + 1);
	setCurrentFontID(tf);
	tf->bufferUsed = 1;
	updateShown = 1;
	return 1;
}

/* scroll page up one line 
 */
int kbf_scroll_up_line(tf)
	TFStruct *tf;
{
	moveOffset(tf, -1, &buffi);
	setCurrentFontID(tf);
	tf->csr_py += buffi;
	moveLine(tf, -1);
	refreshMode = SCREEN;
	updateShown = 1;
	tf->bufferUsed = 1;
	return 1;
}

/* scroll up down one line 
 */
int kbf_scroll_down_line(tf)
	TFStruct *tf;
{
	moveOffset(tf, 1, &buffi);
	setCurrentFontID(tf);
	tf->csr_py += buffi;
	moveLine(tf, 1);
	refreshMode = SCREEN;
	updateShown = 1;
	tf->bufferUsed = 1;
	return 1;
}

/* refresh
 */
int kbf_refresh(tf)
	TFStruct *tf;
{
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	refreshMode = SCREEN;
	updateShown = 1;
	tf->bufferUsed = 1;
	return 1;
}

/* dump lines 
 */
int kbf_dump(tf)
	TFStruct *tf;
{
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	refreshMode = SCREEN;
	updateShown = 1;
	tf->bufferUsed = 1;
	dumpNodeLines(tf);
	return 1;
}

int kbf_newline(tf)
	TFStruct *tf;
{
	int length, start = tf->current_col;
	int delta;
	TFChar *tfcp;
	int orig_breakc, after1_breakc, after2_breakc;
	int origLineNumber;
	TFLineNode *breakLinep;

/*NO GOOD orig_breakc = theEditLN->breakc; */

	/* should not be needed here */
	orig_breakc = countBreaks(theEditLN->linep); 
/*	printf("***2 orig_breakc = %d\n", orig_breakc);*/

	/* this is not correct... loosing tag infomation */
	for (tfcp = theEditLN->linep + start; TFCChar(tfcp); tfcp++)
		if (TFCTagID(tfcp)) {
			TFCTagID(tfcp) = 0;
		}

	/* store left side in tf->currentp */
	replaceNodeLine2(theBuffLN, theEditLN, 0, start, 0, NULL);
	replaceNodeLine(tf->currentp, theBuffLN, 1, tf->mg);

	TFCFlags(tf->currentp->linep + tf->currentp->length) |= MASK_NL;

	if (tf->currentp) {
		after1_breakc = tf->currentp->breakc = 
				countBreaks(tf->currentp->linep);
	}
/*
printf("*** after1_breakc = %d\n", after1_breakc);
*/
	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}
/*
printf("current_row=%d\n",tf->current_row);
printf("1--in editp -------------------------------------------\n");
dumpTFCArray(DUMP_CHAR, theEditLN->linep, NULL,	theEditLN->tagInfoCount);
printf("\n1--in currentp ---------\n");
dumpTFCArray(DUMP_CHAR, tf->currentp->linep, NULL, tf->currentp->tagInfoCount);
printf("\n1--in buff -------------\n");
dumpTFCArray(DUMP_CHAR, theBuffLN->linep, NULL, theBuffLN->tagInfoCount);
*/
	/* remember what the original current_row is, because moveLineNode()
	 * may modify it 
	 */
	origLineNumber = tf->current_row - tf->screen_row_offset;
	breakLinep = tf->currentp;

	insertBelowLineNode(tf, tf->currentp, 1);
	if (!moveLineNode(tf, 1)) return 0;

	delta = rowAdjustOffset(tf, 1);

	length = theEditLN->length - start;
	if (length > 0) {
		replaceNodeLine2(tf->currentp, theEditLN, start, length, 
					1, tf->mg);
	} else if (length < 0) {
		fprintf(stderr, "kbf_newline length== -1!\n");
		length = 0;
	}
	if (tf->currentp) {
		TFCClear(tf->currentp->linep + length);
		if (length == 0) {
			tf->currentp->maxFontHeight = 
				theEditLN->maxFontHeight;
			tf->currentp->maxFontDescent = 
				theEditLN->maxFontDescent;
		}
	}

	TFCShiftStr(theEditLN->linep, 0, -start);
	theEditLN->length -= start;
	theEditLN->breakc = setBreaks(tf, theEditLN);
	after2_breakc = theEditLN->breakc;
/*
printf("*** after2_breakc = %d\n", after2_breakc);
printf("current_row=%d\n",tf->current_row);
printf("2--in editp ----------------------- breakc=%d\n", theEditLN->breakc);
dumpTFCArray(DUMP_CHAR, theEditLN->linep, NULL,	theEditLN->tagInfoCount);
printf("\n2--in currentp -----------\n");
dumpTFCArray(DUMP_CHAR, tf->currentp->linep, NULL, tf->currentp->tagInfoCount);
printf("\n2--in buff ---------------\n");
dumpTFCArray(DUMP_CHAR, theBuffLN->linep, NULL, theBuffLN->tagInfoCount);
printf("\n");
*/
	if (delta == 0) {
		int diff_breakc = after1_breakc + after2_breakc - orig_breakc;
		int offset, upper = 0, lower, shiftSpan, eraseSpan;
		TFLineNode *currentp;

/*		printf("******* diff_breakc = %d\n", diff_breakc);*/

		shiftSpan = theEditLN->maxFontHeight * diff_breakc;
		eraseSpan = theEditLN->maxFontHeight * 
				(after1_breakc + after2_breakc);

		tf->currentp->breakc = theEditLN->breakc;
		tf->currentp->maxFontHeight = theEditLN->maxFontHeight;

		/* upper is where the line-being-split starts 
		 * lower is the original line-being-split ends
		 */
		for (currentp = tf->offsetp; currentp; 
			currentp = currentp->next) {
			if (currentp == breakLinep) break;
			upper += currentp->maxFontHeight * currentp->breakc;
		}
		lower = upper + orig_breakc * currentp->maxFontHeight;

		/* shift down screen to make room for the two split line */
		if (shiftSpan)
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
				  tf->xUL, tf->yUL + lower,
				  tf->width, tf->height - lower - shiftSpan,
				  tf->xUL, tf->yUL + lower + shiftSpan);

		/* clear space for right/lower half of the split line to be */
		if (eraseSpan) {
if (verbose) fprintf(stdout, "tfed: 3 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
				tf->xUL, tf->yUL + upper,
				tf->width, eraseSpan);

			XClearArea(display, TFWINDOW,
				tf->xUL, tf->yUL + upper,
				tf->width, eraseSpan, False);
		}

		/* draw the left/upper half of the split line */
		drawLineOffset(tf, origLineNumber, 1);

		/* draw the right/lower half of the split line */
		drawLineOffset(tf, origLineNumber + 1, 1);
		refreshMode = 0;
		updateShown = 1;
	} else {
		refreshMode = SCREEN;
		updateShown = 1;
	}
	tf->screen_col_offset = 0;
	tf->current_col_sticky = tf->current_col = 0;
	placeCursor(tf);

	tf->bufferUsed = 1;

	return 1;
}

int kbf_useFont_fixed(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_fixed;
	return 1;
}

int kbf_useFont_normal(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_normal;
	return 1;
}

int kbf_useFont_normal_large(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_normal_large;
	return 1;
}

int kbf_useFont_normal_largest(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_normal_largest;
	return 1;
	
}

int kbf_useFont_bold(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_bold;
	return 1;
}

int kbf_useFont_bold_large(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_bold_large;
	return 1;
}

int kbf_useFont_bold_largest(tf)
	TFStruct *tf;
{
	tf->currentFontID = fontID_bold_largest;
	return 1;
}

int kbf_useFont_context(tf)
	TFStruct *tf;
{
	setCurrentFontID(tf);
	return 1;
}


int tfed_get_currentChar(tf, c)
	TFStruct *tf;
	char *c;
{
	if (tf->currentp->linep) {
		*c = TFCChar(theEditLN->linep + tf->current_col);
		return 1;
	} else {
		*c = '\0';
		return 0;
	}
}

int tfed_get_currentLine(tf, strBuff)
	TFStruct *tf;
	char *strBuff;
{
	TFChar *tfcp, *tfcArray;
	int i = 0, j = 0;

	if (tf->editableP) tfcArray = theEditLN->linep;
	else tfcArray = tf->currentp->linep;

	while (tfcp = &(tfcArray[i++])) {
		if (!TFCChar(tfcp)) break;
		strBuff[j++] = TFCChar(tfcp);
	}
	strBuff[j++] = '\0';

	return j;
}

/* returns the length of the word 
 */
int tfed_get_currentWord(tf, strBuff)
	TFStruct *tf;
	char *strBuff;
{
	TFChar *tfcp, *tfcArray;
	int i = 0, j = 0;

	if (tf->editableP) tfcArray = theEditLN->linep;
	else tfcArray = tf->currentp->linep;

	/* add code to convert shift in/outs... */
	for (i = tf->current_col; i >= 0; i--) {
		tfcp = &(tfcArray[i]);
		if (!isalpha(TFCChar(tfcp))) break;
	}
	while (tfcp = &(tfcArray[++i])) {
		if (!isalpha(TFCChar(tfcp))) break;
		else if (TFCChar(tfcp) == '\0') break;
		strBuff[j++] = TFCChar(tfcp);
	}
	strBuff[j++] = '\0';

	return j;
}

int tfed_get_charMask(tf)
	TFStruct *tf;
{
	TFChar *tfcp;

	if (tf->editableP) tfcp = theEditLN->linep + tf->current_col;
	else {
		if (!tf->currentp) return 0;
		tfcp = tf->currentp->linep + tf->current_col;
	}
	return (int)TFCFlags(tfcp);
}

char *tfed_get_currentTag(tf)
	TFStruct *tf;
{
	TFChar *tfcp;

	if (tf->editableP) tfcp = theEditLN->linep + tf->current_col;
	else tfcp = tf->currentp->linep + tf->current_col;

	if (TFCTagID(tfcp))
		return (char*)tf->currentp->tagInfo[TFCTagID(tfcp)].info;

	return NULL;
}

char *tfed_get_previousTag(tf)
	TFStruct *tf;
{
	TFChar *tfcp;
	int i = tf->current_col;

	if (tf->editableP) tfcp = theEditLN->linep + tf->current_col;
	else tfcp = tf->currentp->linep + tf->current_col;

	
	/* search backward to find an tag */
	for (; i >= 0; tfcp--, i--) {
		if (!TFCChar(tfcp)) break;
		if (TFCTagID(tfcp))
		    return (char*)tf->currentp->tagInfo[TFCTagID(tfcp)].info;
	}
	return NULL;
}

char *tfed_get_nextTag(tf)
	TFStruct *tf;
{
	TFChar *tfcp;

	if (tf->editableP) tfcp = theEditLN->linep + tf->current_col;
	else tfcp = tf->currentp->linep + tf->current_col;
/*
printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, tfcp, tf->currentp->tagInfo,
		tf->currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
*/
	/* search forward to find an tag */
	for (; TFCChar(tfcp); tfcp++) {
		if (TFCTagID(tfcp))
		    return (char*)tf->currentp->tagInfo[TFCTagID(tfcp)].info;
	}
	return NULL;
}

int tfed_get_numberOfLinesDisplayed(tf)
	TFStruct *tf;
{
	return tf->num_of_lines;
}

int tfed_get_totalLineCount(tf)
	TFStruct *tf;
{
	return tf->lineNodeCount;
}

int tfed_get_cursorColumn(tf)
	TFStruct *tf;
{
	return tf->current_col;
}

int tfed_get_cursorRow(tf)
	TFStruct *tf;
{
	return tf->current_row;
}

int tfed_get_lineRowOffset(tf)
	TFStruct *tf;
{
	return tf->screen_row_offset;
}

int tfed_set_wrap(tf, wrap)
	TFStruct *tf;
	int wrap;
{
	tf->wrap = wrap;
	return tf->wrap;
}

void TFCInsertChar(tfcArray, col, tfcp)
	TFChar tfcArray[];
	int col;
	TFChar *tfcp;
{
	TFCShiftStr(tfcArray, col, 1);
	TFCCopy(tfcArray + col, tfcp);
}

int tfed_setBuffer(tf, str)
	TFStruct *tf;
	char *str;
{
	int fontID = 0; /* XXX */

	if (str) return str2EBuff(tf, str, &fontID);
	return str2EBuff(tf, "", &fontID);
}

int tfed_getBuffer(str)
	char *str;
{
	if (str) return TFC2StrStrcpy(str, theBuffLN->linep);
	return 0;
}

int tfed_drawCursor(self)
	VObj *self;
{
	TFStruct *tf = updateEStrUser(self);

	if (tf)
		if (tf->isRenderAble) {
			GLPrepareObjColor(self);
			TimedDrawCursor(self, NULL, NULL);
			return 1;
		}
	return 0;
}

/*
int tfed_internalShownPosition(self)
	VObj *self;
{
  return tf->shownPosition;
}
*/

int tfed_setCursorBlinkDelay(self, delay)
	VObj *self;
	long delay;
{
	TFStruct *tf = updateEStrUser(self);

	if (tf) {
		tf->cursorBlinkDelay = delay;
		return 1;
	}
	return 0;
}

int tfed_eraseCursor(self)
	VObj *self;
{
	TFStruct *tf = updateEStrUser(self);
	if (tf)
		if (tf->isRenderAble) {
			GLPrepareObjColor(self);
			TimedEraseCursor(self, NULL, NULL);
		}
	return 1;
}

int tfed_jumpToOffsetLine(self, destLine)
	VObj *self;
	int destLine;
{
	int delta;
	TFStruct *tf = updateEStrUser(self);
	Window w = GET_window(self);

	if (!tf) return 0;
	if (tf->editableP) replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
/*	placeCursorWithinStr(tf);*/
	delta = moveOffset(tf, destLine - tf->screen_row_offset, &buffi);
	tf->csr_py += buffi;

	if (w) {
		if (!tfed_scroll_delta(tf, delta)) renderTF(tf);
	}
	return delta;
}

int tfed_jumpToLine(self, destLine)
	VObj *self;
	int destLine;
{
/*
	TFStruct *tf = updateEStrUser(self);
	int i = 0;
	Window w = GET_window(self);
	int fontID = GET__font(self);

	if (tf) i = jumpLine(tf, w, fontID, destLine);
	return i;
*/
}
#define TolowerChar(x) (isupper(x) ? tolower(x) : x)

TFLineNode *get_nth_line(tf, n)
        TFStruct *tf;
        int n;
{
        TFLineNode *p;

	p = tf->firstp;
        while (n--) p = p->next;
	return p;
}

int tfed_findString(tf, s, patlen, row, col, row_ret, col_ret)
        TFStruct *tf;
        char *s;
	int patlen;
        int row, col;
        int *row_ret, *col_ret;
{
        TFLineNode *lp;
	int maxlen, i, j, k;
	char buf[BUFSIZ];    /* icky -- breaks if patlen > bufsiz */
	char *t;
	TFChar *carray;

	i = 0;
	while (i < patlen) {
	    buf[i++] = TolowerChar(*s);
	    s++;
	}
	buf[patlen] = '\0';

	lp = get_nth_line(tf, row);
	i = col;

    while (lp != NULL) {
	carray = lp->linep;
	while (i < lp->length - patlen) {
	    j = 0, k = i;
	    while ((j < patlen) &&
		   (buf[j] == TolowerChar(TFCChar(carray + k)))) {
		j++, k++;
	    }
	    if (j == patlen) {
		*row_ret = row;
		*col_ret = i;
		return 1;
	    }
	    i++;
	}
        i = 0;
	row++;
        lp = lp->next;
    }
    printf("NO DICE!\n");
    return 0;
}

int tfed_searchAndHighlightStringFromPoint(tf, s)
	TFStruct *tf;
	char *s;
{
	int patlen;
	int startcol, startrow;
	int col_ret, row_ret;

	col_ret = -1;
	patlen = strlen(s);

	if (tf->search_len) {
		startrow = tf->search_y;
		startcol = tf->search_x + 1;
	} else {
		startrow = 0;
		startcol = 0;
	}

	tfed_findString(tf, s, patlen, startrow, startcol, &row_ret, &col_ret);

    /* Remove old search hilite/record etc */

	update_tf_color(tf);

	if (tf->search_len) {
		rangeOperation(tf, tf->search_x, tf->search_y,
				tf->search_x + tf->search_len - 1,
				tf->search_y, 1, -1, 0);
	}

	if (col_ret == -1) /* No match found! */ {
	    tf->search_len = 0;
	    return 0;
	}

	rangeOperation(tf, col_ret, row_ret, col_ret + patlen - 1, row_ret,
			1, 1, 0);
	tf->search_len = patlen;
	tf->search_x = col_ret;
	tf->search_y = row_ret;	

	return 1;
}


/*****************************************************************************
 * internal routines
 */
int TFCstrlen(tfcArray)
	TFChar tfcArray[];
{
	int i = 0;

	while (TFCChar(tfcArray + i)) ++i;

	return i;
}

/* 
 * returns length of tfcAttarTo 
 */
int TFCstrcat(tfcArrayTo, tfcArrayFrom)
	TFChar tfcArrayTo[];
	TFChar tfcArrayFrom[];
{
	int i = 0, j = 0;

	while (TFCChar(tfcArrayTo + i)) ++i;
	while (TFCChar(tfcArrayFrom + j)) {
		TFCCopy(tfcArrayTo + i, tfcArrayFrom + j);
		++i; ++j;
	}
	TFCClear(tfcArrayTo + i);

	return i;
}

int str2EBuff(tf, str, fontID)
	TFStruct *tf;
	char *str;
	int fontID;
{
	return NULL; /*XXX*/
}

/* returns length of tfcAttarTo */
int TFC2StrStrcpy(strTo, tfcArrayFrom)
	char *strTo;
	TFChar *tfcArrayFrom;
{
	int i;

	for (i = 0; strTo[i] = TFCChar(tfcArrayFrom + i); i++);
	strTo[i] = '\0';

	return i;
}

/* returns length of tfcAttarTo */
int TFCstrcpy(tfcArrayTo, tfcArrayFrom)
	TFChar *tfcArrayTo;
	TFChar *tfcArrayFrom;
{
	TFChar *tfcArrayTo_orig = tfcArrayTo;

	while (TFCChar(tfcArrayFrom)) {
		TFCCopy(tfcArrayTo, tfcArrayFrom);
		++tfcArrayFrom;
		++tfcArrayTo;
	}
	TFCClear(tfcArrayTo);

	return tfcArrayTo - tfcArrayTo_orig;
}

int TFCstrncpy(tfcArrayTo, tfcArrayFrom, n)
	TFChar *tfcArrayTo;
	TFChar *tfcArrayFrom;
	int n;
{
	TFChar *tfcArrayTo_orig = tfcArrayTo;

	while (n-- > 0) {
		TFCCopy(tfcArrayTo, tfcArrayFrom);
		++tfcArrayFrom;
		++tfcArrayTo;
	}
	return tfcArrayTo - tfcArrayTo_orig;
}

int joinLine(tf)
	TFStruct *tf;
{
	if (!tf->currentp) return 0;

	tf->lineNodeCount--;
	tf->lineVisibleCount--;
	deleteLineNode(tf);
	TFCstrcpy(theEditLN->linep + theEditLN->length, tf->currentp->linep);
	theEditLN->length += tf->currentp->length;
	setBreaks(tf, theEditLN);

	return 1;
}

#define VERBOSE_INSERTSTR__
int insertStr(tf, split, source)
	TFStruct *tf;
	int split;
	TFLineNode *source;
{
	int i = 0, j = 0, multipleInserts = 0;
	TFChar *tfcp, *tfcLocalBuff;
	TFLineNode *beginp, *endp, *currentp, *newLN;
	int yoffset = tf->csr_py, offset;
	int d, orig_breakc, after_breakc;
	char c;

	orig_breakc = countBreaks(theEditLN->linep);

	/* save right half in theBuffLN
	 */
	theBuffLN->length = theEditLN->length - split;
	replaceNodeLine2(theBuffLN, theEditLN, split, theBuffLN->length, 
				0, NULL);

	TFCClear(theEditLN->linep + split);
	theEditLN->length = split;

#ifdef VERBOSE_INSERTSTR
printf("split = %d\n", split);
printf("--source --\n>");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, source->linep, NULL, 
	theEditLN->tagInfoCount);
printf("<\n--buff --\n>");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theBuffLN->linep, NULL, 
	theBuffLN->tagInfoCount);
printf("<\n--edit --\n>");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL,
	theEditLN->tagInfoCount);
printf("<\n");
#endif

	/* find line delimeters (i, j) in source
	 */
	for (tfcp = source->linep; c = TFCChar(tfcp); tfcp++)
		if (c == '\n' || c  == '\r') break;
	j = tfcp - source->linep;

	/* append first source-line to the left half of the edit-line 
	 */
	currentp = tf->currentp;
	offset = tf->current_row;

	if (!currentp) return 0;

	if (j == source->length) {
		/* the source is a one liner, so just insert it into
		 * the current buffer, and be done with.
		 */

		TFCstrncpy(theEditLN->linep + split, source->linep, j);
		theEditLN->length = split + j;
		TFCClear(theEditLN->linep + theEditLN->length);

#ifdef VERBOSE_INSERTSTR
printf("\n--edit --\n>");
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL, 
		theEditLN->tagInfoCount);
printf("<\n");
#endif
		tf->current_col_sticky = tf->current_col = theEditLN->length;

		TFCstrncpy(theEditLN->linep + theEditLN->length, 
				theBuffLN->linep, theBuffLN->length);
		theEditLN->length += theBuffLN->length;
		TFCClear(theEditLN->linep + theEditLN->length);
		setBreaks(tf, theEditLN);
		replaceNodeLine(currentp, theEditLN, 1, tf->mg);

		d = orig_breakc - currentp->breakc;

#ifdef VERBOSE_INSERTSTR
printf("\n--edit -- length=%d\n>", theEditLN->length);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL,
		theEditLN->tagInfoCount);
printf("<\n");
printf("### orig_breakc=%d  currentbreakc=%d  d=%d\n",
	orig_breakc, currentp->breakc, d);
#endif

		if (d < 0) {
			if (!scrollDownLowerPartNEW(tf, 
				offset - tf->screen_row_offset, 
				currentp->maxFontHeight * -d)) {
				renderTF(tf);
			}
		} else if (d > 0) {
			if (!scrollUpLowerPart(tf, 
				offset - tf->screen_row_offset, 
				currentp->maxFontHeight * d)) {
				renderTF(tf);
			}
		}
		drawLineOffset(tf, offset - tf->screen_row_offset, 1);

	} else {
		/* the source has atleast one carriage return.
		 */
		int diffSpan, oldSpan, newSpan, finish = 0, length;
		int upper = 0, linesToDraw = 0;
#ifdef VERBOSE_INSERTSTR
printf("\n--edit -- length=%d\n>", theEditLN->length);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL,
		theEditLN->tagInfoCount);
printf("<\n");
#endif
		oldSpan = currentp->maxFontHeight * orig_breakc;
		newSpan = 0;
		beginp = currentp;
		offset = tf->current_row - tf->screen_row_offset;

#ifdef VERBOSE_INSERTSTR
		fprintf(stderr, "** i=%d j=%d \n", i, j);
#endif
		while (j <= source->length) {
			/* find line delimeter in source */
			for (tfcp = source->linep + i; c = TFCChar(tfcp); 
				tfcp++) 
				if (c == '\n' || c  == '\r') break;
			j = tfcp - source->linep;

#ifdef VERBOSE_INSERTSTR
			fprintf(stderr, "**** i=%d j=%d \n", i, j);
#endif
			/*
			** Compute total length of line, and make line
			*/
			if (j >= source->length) {
				finish = 1;
			}
			if (!linesToDraw) {		/* first line */
				newLN = tf->currentp;
				if (currentp->linep) 
					Vfree(tf->mg, currentp->linep);
				length = theEditLN->length + j - i;
			} else {
				newLN = insertBelowLineNode(tf, currentp, 0);
				tf->current_row++;
				length = j - i;
			}
			if (finish) {			/* last line */
				if (theBuffLN->length)
					length += theBuffLN->length;
			}
			newLN->length = length;
			newLN->linep = (TFChar*)Vmalloc(tf->mg, 
							sizeof(struct TFChar) *
							(newLN->length + 1));
/*MSTAT_tfed+=newLN->length;*/
			if (!newLN->linep) {
				fprintf(stderr, "malloc failed\n");
				return 0;
			}

			/*
			** Copy data into line
			*/
			if (!linesToDraw) {		/* first line */
				TFCstrcpy(newLN->linep, theEditLN->linep);
				TFCstrncpy(newLN->linep + theEditLN->length,
						source->linep + i, 
						j - i);
				newLN->length = theEditLN->length + j - i;
				TFCClear(newLN->linep + newLN->length);
			} else {
				TFCstrncpy(newLN->linep, 
					source->linep + i, j - i);
				newLN->length = j - i;
				TFCClear(newLN->linep + newLN->length);
			}
			if (finish) {			/* last line */
				tf->current_col_sticky = tf->current_col 
					= newLN->length;
				if (theBuffLN->length) {
					TFCstrcat(newLN->linep, 
						theBuffLN->linep);
					newLN->length += theBuffLN->length;
				}
				setBreaks(tf, newLN);
				replaceNodeLine(theEditLN, newLN, 0, NULL);
			} else {
				setBreaks(tf, newLN);
			}
#ifdef VERBOSE_INSERTSTR
if (VERBOSE_INSERTSTR) {
fprintf(stderr, "i=%d j=%d \n", i, j);
printf("\n--newLN----length=%d breakc=%d---\n>", newLN->length, newLN->breakc);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, newLN->linep, NULL,
		newLN->tagInfoCount);
printf("<\n");
#endif
			linesToDraw++;
			currentp = newLN;
			tf->currentp = newLN;
			newSpan += currentp->maxFontHeight * currentp->breakc;

			c = TFCChar(source->linep + j);
			if (c == '\0') {
				if (finish) break;
			}
			i = ++j;
		}
		endp = currentp;

		/* Make space for drawing 
		 */
		currentp = tf->offsetp;
		for (i = 0; i < offset; i++) {
			if (!currentp) break;
			upper += currentp->maxFontHeight * currentp->breakc;
			currentp = currentp->next;
		}
		diffSpan = newSpan - oldSpan;
		if (diffSpan > 0) {
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
				tf->xUL, tf->yUL + upper,
				tf->width, tf->height - upper - diffSpan,
				tf->xUL, tf->yUL + upper + diffSpan);
		}
		/* Draw the new lines
		 */
		for (; linesToDraw; linesToDraw--, i++) {
#ifdef VERBOSE_INSERTSTR
				printf("DRAWING line offset=%d\n", i);
#endif
			drawLineOffset(tf, i, 1);
		}
	}
	placeCursorWithinStr(tf);

#ifdef VERBOSE_INSERTSTR
printf("--3 dump:\n");
dumpNodeLines(tf);
printf("\n--edit----breakc=%d---\n>", theEditLN->breakc);
dumpTFCArray(DUMP_CHAR|DUMP_FONT, theEditLN->linep, NULL,
		theEditLN->tagInfoCount);
printf("<\n");
#endif

	return 1;
}

/*
 * change cursor's visual-related variables in reflection of the cursor's line
 * movement.
 *
 * RETURN: lines moved in the signed direction.
 * span accumulates pixel span of scrolle lines
 */
int moveOffset(tf, dir, span)
	TFStruct *tf;
	int dir;
	int *span;
{
	int lineTraversed = 0, linesToTraverse;

	*span = 0;

	if (dir == 0) return 0;
	linesToTraverse = abs(dir);

	if (dir < 0) {
		/* traverse backward */
		while (lineTraversed < linesToTraverse) {
			if (tf->offsetp == NULL) {
				/* whoops! offsetp is NULL! */
				break; 
			}
			if (tf->offsetp->prev == NULL) {
				/* the offsetp line is the first line */
				break;
			} else {
				/* there exists a previous line node. 
				 * move to previous line node 
				 */
				tf->offsetp = tf->offsetp->prev;
				*span += tf->offsetp->maxFontHeight *
						tf->offsetp->breakc;
				--(tf->screen_row_offset);
			}
			++lineTraversed;
		}
		return -lineTraversed;
	} else {
		/* traverse forward */
		while (lineTraversed < linesToTraverse) {
			if (tf->offsetp == NULL) {
				/* whoops! offsetp is NULL! */
				break; 
			}
			if (tf->offsetp->next == NULL) {
				/* the offsetp line is the last line */
				break;
			} else {
				/* there exists a next line node. 
				 * move to next line node 
				 */
				*span -= tf->offsetp->maxFontHeight *
						tf->offsetp->breakc;
				tf->offsetp = tf->offsetp->next;
				tf->screen_row_offset++;
			}
			lineTraversed++;
		}
		return lineTraversed;
	}
}

int jumpLine(tf, w, fontID, destLine)
	TFStruct *tf;
	Window w;
	int fontID;
	int destLine;
{
	int delta = moveLine(tf, destLine - tf->current_row);
	return delta;
}

/*
 * copy node line
 */
int replaceNodeLine(to, from, freeOldSpaceP, mg)
	TFLineNode *to;
	TFLineNode *from;
	int freeOldSpaceP;
	MemoryGroup *mg;
{
	if (!from) {
/*		fprintf(stderr, "*** replaceNodeLine: FROM == NULL\n");*/
		return 0;
	}
	if (!from->linep) {
/*		fprintf(stderr, "*** replaceNodeLine: FROM->linep == NULL\n");*/
		return 0;
	}
	if (!to) {
/*		fprintf(stderr, "*** replaceNodeLine: TO == NULL\n");*/
		return 0;
	}
	if (freeOldSpaceP) {
		if (to->linep) {
			if (mg) {
				Vfree(mg, to->linep);
				Vfree(mg, to->tagInfo);
			} else {
				Rfree(to->linep);
				Rfree(to->tagInfo);
			}
		}
		if (mg) to->linep = (TFChar*)Vmalloc(mg, sizeof(struct TFChar) 
					 * (TFCstrlen(from->linep) + 1));
		else  to->linep = (TFChar*)Rmalloc(sizeof(struct TFChar) 
					 * (TFCstrlen(from->linep) + 1));
		if (!to->linep) {
			fprintf(stderr, "malloc failed\n");
			return 0;
		}
	} else {
		if (mg) {
			Vfree(mg, to->tagInfo);
		} else {
			Rfree(to->tagInfo);
		}
	}
	TFCstrcpy(to->linep, from->linep);
	to->length = from->length;
	to->breakc = from->breakc;
	to->maxFontHeight = from->maxFontHeight;
	to->maxFontDescent = from->maxFontDescent;

/*YYY	bcopy(from->tagInfo, to->tagInfo,   sizeof(int) * TAGINFO_SIZE);*/

	to->tagInfoCount = from->tagInfoCount;
	to->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					* to->tagInfoCount);
	bcopy(from->tagInfo, to->tagInfo,
		sizeof(struct TagInfo) * to->tagInfoCount);

	return 1;
}

/*
 * copy node line
 */
int replaceNodeLine2(to, from, start, length, freeOldSpaceP, mg)
	TFLineNode *to;
	TFLineNode *from;
	int start, length;
	int freeOldSpaceP;
	MemoryGroup *mg;
{
	if (!from->linep) {
/*		fprintf(stderr, "*** replaceNodeLine2: FROM->linep == NULL\n");*/
		return 0;
	}
	if (!to) {
/*		fprintf(stderr, "*** replaceNodeLine2: TO == NULL\n");*/
		return 0;
	}
	if (freeOldSpaceP) {
		if (to->linep) {
			if (mg) Vfree(mg, to->linep);
			else Rfree(to->linep);
		}
		if (mg) to->linep = (TFChar*)Vmalloc(mg, sizeof(struct TFChar) 
					 * (length + 1));
		else  to->linep = (TFChar*)Rmalloc(sizeof(struct TFChar) 
					 * (length + 1));
		if (!to->linep) {
			fprintf(stderr, "malloc failed\n");
			return 0;
		}
	}
	TFCstrncpy(to->linep, from->linep + start, length);
	TFCClear(to->linep + length);
/*	TFCFlags(to->linep) = TFCFlags(from->linep + length); trouble is, the null char's flag info is significant, because MASK_NL is stored there.. bug*/
	to->length = length;
	to->maxFontHeight = from->maxFontHeight;
	to->maxFontDescent = from->maxFontDescent;
/*YYY	bcopy(from->tagInfo, to->tagInfo, sizeof(int) * TAGINFO_SIZE); */

	to->tagInfoCount = from->tagInfoCount;
	to->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo)
					* to->tagInfoCount);
	bcopy(from->tagInfo, to->tagInfo,
		sizeof(struct TagInfo) * to->tagInfoCount);

	return 1;
}

/* assumes string pointed to be 'cp' is delimiter escapped-- no '(' and ')'
 */
char *probeTagBody(cp)
	char *cp;
{
	int paran = 0;
/*	char lc = '\0', llc = '\0';*/

	while (cp) {
		if (*cp == STAG_OPEN) {
			++paran;
		} else if (*cp == STAG_CLOSE) {
			--paran;
			if (paran == 0) return ++cp;
		}
/*		llc = lc;
		lc = *cp;  */
		++cp;
	}
	return 0;
}

int addCtrlChar(buildInfo)
	TFCBuildInfo *buildInfo;
{
	char c, *s, *cp;
	int i, j, stat, done = 0, length;
	TFChar *tfcp = buildInfo->tbuff + buildInfo->tbuffi;
	TagInfo *ti;

	if (buildInfo->verbatim) {
		TFCChar(tfcp) = *(buildInfo->str);
		TFCFontID(tfcp) = buildInfo->fontID;
		TFCFlags(tfcp) = buildInfo->flags;
		if (++(buildInfo->tbuffi) > TBUFFSIZE) {
			TFCChar(tfcp) = '\0';
			splitLine(buildInfo, buildInfo->breaki, 1);
			tfcp = buildInfo->tbuff + buildInfo->tbuffi;
		}
		++tfcp;
		TFCTagID(tfcp) = 0;
		buildInfo->flags = NULL;
		++(buildInfo->str);
		return 1;
	}

	switch (*(++(buildInfo->str))) {

		/*
		 * picture insert
		 *
		 *  Introduced MASK_PIC flag. fontID contains picture ID,
		 *  to look up in textfield picture list. SGML built process
		 *  had created and put the pictures in list. 
		 *  XXXX made me do it.
		 */
		case 'p':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			int picID;
			TFPic *pic = NULL, *picp, *pics;

			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			strncpy(sbuff, buildInfo->str + 1, length);
			sbuff[length] = '\0';

/*			picID = atoi(sbuff);*/

			pic = tfed_addPicFromFile(&(buildInfo->pics), 
						  sbuff/*URL*/,
						  NULL/*no file*/);

/*XXXXXX
	trouble here.... figure out why buildInfo->pics is null
	even for picID == 2.
*/
/*			for (picp = buildInfo->pics; picp; picp = picp->next) {
				if (picp->id == picID) {
					pic = picp;
					break;
				}
			}
*/
			if (!pic) pic = dunselPic;

			if (buildInfo->maxFontHeight < pic->height)
				buildInfo->maxFontHeight = pic->height;

			TFCChar(tfcp) = '@';
			TFCFlags(tfcp) = buildInfo->flags | MASK_PIC;
			TFCFontID(tfcp) = pic->id;

			buildInfo->px += pic->width;

			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				TFCChar(tfcp) = '\0';
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			}
			++tfcp;
			TFCTagID(tfcp) = 0;
			++(buildInfo->str);
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		/* embedded object
		 */
		case 'o':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			VObj *inset;
			int insetHeight;
			int xLR = GET__TFStruct(buildInfo->self)->xLR - 2; /* 2=fudgeFactor*/

			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			cp = (char*)Rmalloc(sizeof(char) * length + 1);
			strncpy(cp, buildInfo->str + 1, length);
			cp[length] = '\0';

			inset = getObject(cp);
			if (!inset) {
				printf("Inset object '%s' not found.\n", cp);
				break;
			}

			if (buildInfo->px + GET_width(inset) + 2 >= xLR) {
				if (buildInfo->tbuffi > 1) {
					TFCChar(tfcp) = '\0';
					splitLine(buildInfo, 
						buildInfo->breaki, 1);
					tfcp = buildInfo->tbuff + 
							buildInfo->tbuffi;
				}
			}

			if (buildInfo->tagID < TAGINFO_SIZE) {
			    ++(buildInfo->tagID);
			    TFCTagID(tfcp) = buildInfo->tagID;
			    TFCFlags(tfcp) = buildInfo->flags;

			    ti = &(buildInfo->buffTagInfo[buildInfo->tagID]);
			    ti->info = cp;
/*
			    ti->x_begin = 0;
			    ti->x_end = 0;
			    ti->valid = 0;
*/
			} else {
				if (TFCChar(tfcp) == '\0') {
				  TFCFlags(tfcp - 1) = buildInfo->flags;
				} else {
				  TFCFlags(tfcp) = buildInfo->flags;
				}
				fprintf(stderr, "exceeded tag limit\n");
			}
			if (tfed_anchorPatchIdx > 0) {
				int i;
				for (i = 0; i < tfed_anchorPatchIdx; i++) {
					*(tfed_anchorPatch[i]) 
						= (char*)saveString(cp);
				}
				tfed_anchorPatchIdx = 0;
			}

			if (GET__parent(inset) != buildInfo->self) {
				VObjList *olist;
				olist = GET__children(buildInfo->self);
				SET__children(buildInfo->self, 
					appendObjToList(olist, inset));
				SET__parent(inset, buildInfo->self);
			}
			SET_x(inset, buildInfo->px);
			SET_y(inset, buildInfo->vspan);

			buildInfo->px += GET_width(inset) + 2;
			insetHeight = GET_height(inset) + 2;

			if (buildInfo->maxFontHeight < insetHeight)
				buildInfo->maxFontHeight = insetHeight;

			TFCChar(tfcp) = '@';
			TFCFlags(tfcp) = buildInfo->flags | MASK_OBJ;
			TFCFontID(tfcp) = buildInfo->fontID;

			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				TFCChar(tfcp) = '\0';
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			}
			++tfcp;
			TFCTagID(tfcp) = 0;
			++(buildInfo->str);/*useless*/
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		case 'q':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			int picID;
			TFPic *pic = NULL, *picp, *pics;

			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			strncpy(sbuff, buildInfo->str + 1, length);
			sbuff[length] = '\0';

			picID = atoi(sbuff);

			picp = (TFPic*)GET__content(buildInfo->self);
			for (; picp; picp = picp->next) {
				if (picp->id == picID) {
					pic = picp;
					break;
				}
			}

			if (!pic) pic = dunselPic;

			if (!tfed_addPic(&(buildInfo->pics), pic)) {
				fprintf(stderr, "tfed_addPic() failed\n");
			}
			/* XXX remember to free pic list in __content(self)!!! 
			 */

			if (buildInfo->maxFontHeight < pic->height)
				buildInfo->maxFontHeight = pic->height;

			TFCChar(tfcp) = '@';
			TFCFlags(tfcp) = buildInfo->flags | MASK_PIC;
			TFCFontID(tfcp) = pic->id;

			buildInfo->px += pic->width;

			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				TFCChar(tfcp) = '\0';
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			}
			++tfcp;
			TFCTagID(tfcp) = 0;
			++(buildInfo->str);
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		case 'e':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
/*			cp = (char*)Vmalloc(GET__TFStruct(buildInfo->self)->mg, 
						sizeof(char) * length + 1);
*/
			cp = (char*)malloc(sizeof(char) * (length + 1));
/*MSTAT_tfed+=length;*/
			if (!cp) {
				fprintf(stderr, "malloc failed\n");
				return 0;
			}
			strncpy(cp, buildInfo->str + 1, length);
			cp[length] = '\0';
			if (buildInfo->tagID < TAGINFO_SIZE) {
			    ++(buildInfo->tagID);
/*
			    if (TFCChar(tfcp) == '\0') {
				  TFCTagID(tfcp - 1) = buildInfo->tagID;
				  TFCFlags(tfcp - 1) = buildInfo->flags;
			    } else {
				  TFCTagID(tfcp) = buildInfo->tagID;
				  TFCFlags(tfcp) = buildInfo->flags;
			    }
*/
			    TFCTagID(tfcp) = buildInfo->tagID;
			    TFCFlags(tfcp) = buildInfo->flags;

			    ti = &(buildInfo->buffTagInfo[buildInfo->tagID]);
			    ti->info = cp;
/*			    ti->x_begin = 0;
			    ti->x_end = 0;
			    ti->valid = 0;
*/
			} else {
				if (TFCChar(tfcp) == '\0') {
				  TFCFlags(tfcp - 1) = buildInfo->flags;
				} else {
				  TFCFlags(tfcp) = buildInfo->flags;
				}
				fprintf(stderr, "exceeded tag limit\n");
			}
			if (tfed_anchorPatchIdx > 0) {
				int i;
				for (i = 0; i < tfed_anchorPatchIdx; i++) {
					*(tfed_anchorPatch[i]) 
						= (char*)saveString(cp);
				}
				tfed_anchorPatchIdx = 0;
			}

/*
			TFCChar(tfcp) = '*';
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)['*'];
			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				TFCChar(tfcp) = '\0';
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff 
						+ buildInfo->tbuffi;
			}
			++tfcp;
*/
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		/*
		 * Script result insert
		 */
		case 's':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			Packet result;
			TFCBuildInfo buildInfo2;

			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 1;
			cp = (char*)Rmalloc(sizeof(char) * (length + 1));
			if (!cp) {
				fprintf(stderr, "malloc failed\n");
				return 0;
			}
			strncpy(cp, buildInfo->str + 1, length);
			cp[length] = '\0';

			result.info.s = NULL;
			execScript(buildInfo->self, &result, cp);
			Rfree(cp);
			if (!result.info.s) break;

			cp = PkInfo2Str(&result);

			buildInfo2.beginp = buildInfo->beginp;
			buildInfo2.str = cp;
			buildInfo2.makeLinep = 0;
			buildInfo2.vspan = 0;

/*			bcopy(buildInfo2, buildInfo, 
				sizeof(struct TFCBuildInfo));
*/
 			memcpy((void*)&buildInfo2, 
 				(void*)buildInfo, 
 				(size_t)sizeof(struct TFCBuildInfo));

			tfed_buildLines(&buildInfo2);

			buildInfo->vspan += buildInfo2.vspan;

/*XXXX something needs to happen here */
			tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		/*
		 * font change
		 */
		case 'f':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			int newFontID;
			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			strncpy(sbuff, buildInfo->str + 1, length);
			sbuff[length] = '\0';
			newFontID = atoi(sbuff);
			if (newFontID <= numberOfFontIDs) {
				buildInfo->fontID = newFontID;
				buildInfo->spaceWidth = 
						FontWidths(newFontID)[' '];

				if (buildInfo->maxFontHeight < 
					FontMaxHeight(newFontID)) 
					buildInfo->maxFontHeight = 
					    FontMaxHeight(newFontID);
				if (buildInfo->maxFontDescent < 
					FontDescent(newFontID))
 					buildInfo->maxFontDescent = 
					    FontDescent(newFontID);
				buildInfo->str = s;
			} else {
				fprintf(stderr, 
					"buildLine: fontID unknown.\n");
			}
		} else {
			goto foobar;
		}
		break;

		/*
		 * highlight
		 */
		case 'h':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			cp = buildInfo->str + 1;
			buildInfo->flags |= MASK_REVERSE;
			while (length--) {
				TFCChar(tfcp) = *cp;
				TFCFontID(tfcp) = buildInfo->fontID;
				TFCFlags(tfcp) = buildInfo->flags;
				buildInfo->px += 
					FontWidths(buildInfo->fontID)[*cp];
				++cp;
				++tfcp;
				++buildInfo->tbuffi;
				TFCTagID(tfcp) = 0;
			}
			TFCFlags(tfcp) = NULL;
			buildInfo->flags &= ~MASK_REVERSE;
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;

		/*
		 * Underline
		 */
/*
		case 'u':
		if (*(++(buildInfo->str)) == STAG_OPEN) {
			s = probeTagBody(buildInfo->str);
			if (!s) {
				printf("tag error.\n");
				break;
			}
			length = s - buildInfo->str - 2;
			cp = buildInfo->str + 1;
			buildInfo->flags |= MASK_UNDER;
			while (length--) {
				TFCChar(tfcp) = *cp;
				TFCFontID(tfcp) = buildInfo->fontID;
				TFCFlags(tfcp) = buildInfo->flags;
				buildInfo->px += 
					FontWidths(buildInfo->fontID)[*cp];
				++cp;
				++tfcp;
				++(buildInfo->tbuffi);
				TFCTagID(tfcp) = 0;
			}
			buildInfo->flags &= ~MASK_UNDER;
			buildInfo->str = s;
		} else {
			goto foobar;
		}
		break;
*/
		/*
		 * Italic 
		 */
		case 'i':
			if (*(++(buildInfo->str)) == STAG_OPEN) {
				/*
				 * italic
				 */
				s = probeTagBody(buildInfo->str);
				if (!s) {
					printf("tag error.\n");
					break;
				}
				length = s - buildInfo->str - 2;
				cp = buildInfo->str + 1;
				buildInfo->flags |= MASK_ITALIC;
				while (length--) {
					TFCChar(tfcp) = *cp;
					TFCFontID(tfcp) = buildInfo->fontID;
					TFCFlags(tfcp) = buildInfo->flags;
					buildInfo->px += 
					    FontWidths(buildInfo->fontID)[*cp];
					++cp;
					++tfcp;
					++(buildInfo->tbuffi);
					TFCTagID(tfcp) = 0;
				}
				buildInfo->flags &= ~MASK_ITALIC;
				buildInfo->str = s;
			} else {
				goto foobar;
			}
		break;

		/*
		 * Button
		 */
		case 'b':
			c = *(++(buildInfo->str));
			if (c == STAG_OPEN) {
				buildInfo->flags |= MASK_BUTTON;
				++(buildInfo->str);
			} else if (c == STAG_CLOSE) {
				buildInfo->flags &= ~MASK_BUTTON;
				++(buildInfo->str);
			} else {
				goto foobar;
			}
		break;

		/*
		 * xrule 
		 */
/*
		case 'x':
			if (*(++(buildInfo->str)) == STAG_OPEN) {
				TFCChar(tfcp) = ' ';
				TFCFlags(tfcp) = buildInfo->flags;
				TFCFontID(tfcp) = buildInfo->fontID;
				TFCFlags(tfcp) = buildInfo->flags | MASK_XRULE;
				++tfcp;
				TFCTagID(tfcp) = 0;
				buildInfo->str += 2;
			} else {
				buildInfo->str -= 2;
			}
		break;
*/
		case '\\':
			TFCChar(tfcp) = '\\';
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)['\\'];
			++tfcp;
			++(buildInfo->tbuffi);
			TFCTagID(tfcp) = 0;
			++(buildInfo->str);
		break;

		case 'n':
			TFCChar(tfcp) = ' ';
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)[' '];
			++tfcp;
			++(buildInfo->tbuffi);
			TFCTagID(tfcp) = 0;
			++(buildInfo->str);
		break;

		default:
foobar:		/* unknown escape */
			TFCChar(tfcp) = '\\';
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)['\\'];
			++tfcp;
			++(buildInfo->tbuffi);
			TFCTagID(tfcp) = 0;

			TFCChar(tfcp) = *(buildInfo->str - 1);
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)[
							*(buildInfo->str - 1)];
			++tfcp;
			++(buildInfo->tbuffi);
			TFCTagID(tfcp) = 0;

			TFCChar(tfcp) = *(buildInfo->str);
			TFCFlags(tfcp) = buildInfo->flags;
			TFCFontID(tfcp) = buildInfo->fontID;
			buildInfo->px += FontWidths(buildInfo->fontID)[
							*(buildInfo->str)];
			++tfcp;
			++(buildInfo->tbuffi);
			TFCTagID(tfcp) = 0;

			++(buildInfo->str);
		break;
	}
/*	TFCTagID(tfcp) = 0;*/
	return 1;
}

/*
 * Recrusively convert str into TFLineNodes and insert at currentp
 * fontID = current font id
 * retun lineCount
 */
int tfed_buildLines(buildInfo)
	TFCBuildInfo *buildInfo;
{
	TFChar *tfcp;
	char c, *strp;
	char *s, *cp;
	int i, j, stat, keepGoing = 1, length;
	int xLR = GET__TFStruct(buildInfo->self)->xLR - 2; /* 2=fudgeFactor*/
	BOOL format;
	int fontID, *widths, flags;

	if (!buildInfo->str) return 0;

	if (buildInfo->tbuffi == 0) {
		tfcp = buildInfo->tbuff;
		TFCTagID(tfcp) = buildInfo->tagID;
	}

	format = buildInfo->format;
	fontID = buildInfo->fontID;
	widths = FontWidths(fontID);

	strp = buildInfo->str;
	tfcp = buildInfo->tbuff + buildInfo->tbuffi;
	flags = buildInfo->flags;
	TFCTagID(tfcp) = 0;

	while (keepGoing) {
/*		printf("[%c]\n", *strp);*/

	  	c = *strp;

		if ((c > ' ') && (c != '\\')) {
			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				/* error. lossing info... */
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			}
			buildInfo->px += widths[c];

			TFCChar(tfcp) = c;
			TFCFontID(tfcp) = fontID;
			TFCFlags(tfcp) = flags;

			if (format && (buildInfo->px >= xLR)) {
				if (buildInfo->breaki > 0) {
					TFCTagID(buildInfo->tbuff +
						 buildInfo->tbuffi) = 0;
					splitLine(buildInfo,
					    buildInfo->breaki, 1/*newln*/); 
				} else {
					TFCTagID(buildInfo->tbuff +
						 buildInfo->tbuffi) = 0;
					splitLine(buildInfo, 
					    buildInfo->tbuffi, 1/*newln*/);
				}
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			} else {
				++tfcp;
			}
			TFCTagID(tfcp) = 0;
			++strp;

		} else if (c == ' ') {
			if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				/* error. lossing info... */
				splitLine(buildInfo, buildInfo->breaki, 1);
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			}
			buildInfo->px += buildInfo->spaceWidth;

			TFCChar(tfcp) = ' ';
			TFCFontID(tfcp) = fontID;
			TFCFlags(tfcp) = flags;

			if (format && (buildInfo->px >= xLR)) {
				if (buildInfo->breaki > 0) {
					TFCTagID(buildInfo->tbuff +
						 buildInfo->tbuffi) = 0;
					splitLine(buildInfo,
					    buildInfo->breaki, 1/*newln*/); 
				} else {
					TFCTagID(buildInfo->tbuff +
						 buildInfo->tbuffi) = 0;
					splitLine(buildInfo, 
					    buildInfo->tbuffi, 1/*newln*/);
				}
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
			} else {
				buildInfo->breaki = buildInfo->tbuffi;
				++tfcp;
			}
			TFCTagID(tfcp) = 0;
			++strp;

		} else {
			switch (c) {
			case '\\':
				if (buildInfo->verbatim) {
					buildInfo->px += widths[c];
					TFCChar(tfcp) = c;
					TFCFontID(tfcp) = fontID;
					TFCFlags(tfcp) = flags;
					if (++(buildInfo->tbuffi) > 
						TBUFFSIZE) {
						/* error. lossing info... */
						splitLine(buildInfo, 
							buildInfo->breaki, 1);
						tfcp = buildInfo->tbuff 
							+ buildInfo->tbuffi;
					}
					++tfcp;
					TFCTagID(tfcp) = 0;
					++strp;
				} else {
					buildInfo->str = strp;
					addCtrlChar(buildInfo);

					/* in case they changed */
					flags = buildInfo->flags;
					if (fontID != buildInfo->fontID) {
						fontID = buildInfo->fontID;
						widths = FontWidths(fontID);
					}
					tfcp = buildInfo->tbuff 
						+ buildInfo->tbuffi;
					strp = buildInfo->str;
					if (buildInfo->px >= xLR) {
					  /* split the line only if
					   * there's > one glyph
					   * on the line 
					   */	
					  if (buildInfo->tbuffi > 1) {
					    if (!strncmp(strp, "\\b)\\e(",6)) {

					      TFCFlags(tfcp-1) |= MASK_BUTTON;

					      TFCTagID(tfcp) = 0;
					      TFCChar(tfcp) = ' ';

					      buildInfo->flags &= ~MASK_BUTTON;

					      TFCFontID(tfcp) = fontID;
					      TFCFlags(tfcp) = buildInfo->flags;
					      buildInfo->str += 3;
					      addCtrlChar(buildInfo);
					      buildInfo->breaki++;
					    }
					    splitLine(buildInfo, 
					        buildInfo->breaki, 1/*newln*/);
					    tfcp = buildInfo->tbuff 
							  + buildInfo->tbuffi;
					  } else if (buildInfo->tbuffi == 1) {
					    if (!strncmp(strp, "\\b)\\e(",6)) {
char *url;
/* HIGH KLUDGE
   \b(\o(HTML__img8)\b)\e(%2Fhtbin%2Fimagemap%2Fhome-europe) 
*/
					      TFCFlags(tfcp-1) |= MASK_BUTTON;

					      TFCTagID(tfcp) = 0;
					      TFCChar(tfcp) = ' ';

					      buildInfo->flags &= ~MASK_BUTTON;
					      flags = buildInfo->flags;

					      TFCFontID(tfcp) = fontID;
					      TFCFlags(tfcp) = buildInfo->flags;

					      buildInfo->str += 3;

					      addCtrlChar(buildInfo);

					      tfcp = buildInfo->tbuff 
							+ buildInfo->tbuffi;
/*
					      TFCFlags(tfcp) = 0;
					      TFCTagID(tfcp) = 0;
*/
/*
url = buildInfo->tagInfo[1].info;
buildInfo->tagInfo[1].info = NULL;
buildInfo->tagID--;
*/
					      splitLine(buildInfo, 1,
							1/*newln*/);

					      tfcp = buildInfo->tbuff
							 + buildInfo->tbuffi;
					      strp = buildInfo->str;
					    } else {
					      splitLine(buildInfo, 1, 
						      1/*newln*/);
					      tfcp = buildInfo->tbuff 
							  + buildInfo->tbuffi;
					    }
					  }
					  TFCFlags(tfcp) = 0;
					  TFCTagID(tfcp) = 0;
					}
				}
			break;
			case '\n':
				if (buildInfo->verbatim
				    || buildInfo->format == FORMAT_NONE) {

					if (!TFCTagID(tfcp)) {
					  /* to prevent overwriting embedded
					   * information in flags, by using 
					   * a dummy space character (not seen)
					   * to keep place.
					   */
					  TFCChar(tfcp) = ' ';
					  TFCFlags(tfcp) = flags;
					  TFCFontID(tfcp) = fontID;
					  ++tfcp;
					  ++buildInfo->tbuffi;
					  TFCTagID(tfcp) = 0;
					  buildInfo->breaki = 
					    buildInfo->tbuffi;
					} 
					TFCChar(tfcp) = '\0';
					TFCFlags(tfcp) = flags;
					TFCFontID(tfcp) = fontID;

					splitLine(buildInfo, 
						buildInfo->tbuffi, 1);
					tfcp = buildInfo->tbuff 
						+ buildInfo->tbuffi;
					TFCTagID(tfcp) = 0;
					++strp;
				} else {
					buildInfo->breaki = buildInfo->tbuffi;
					TFCChar(tfcp) = ' ';
					TFCFlags(tfcp) = flags;
					TFCFontID(tfcp) = fontID;
					buildInfo->px += buildInfo->spaceWidth;
					if (++(buildInfo->tbuffi) > 
						TBUFFSIZE) {
						/* error. lossing info... */
						TFCChar(tfcp) = '\0';
						splitLine(buildInfo, 
							buildInfo->breaki, 1);
						tfcp = buildInfo->tbuff 
							+ buildInfo->tbuffi;
					} else {
						++tfcp;
					}
					++strp;
				}
				TFCTagID(tfcp) = 0;
			break;
	
			case '\r':
				/* ignore carriage return characters. */
				++strp;
			break;

			case '\t':
			  {
				int mark;
				int spaceWidth = buildInfo->spaceWidth;
				int tabSpan = spaceWidth * TFC_TAB_SPACES;

				mark = (tabSpan - (buildInfo->px % tabSpan))
					+ buildInfo->px;

				buildInfo->breaki = buildInfo->tbuffi;

				while (buildInfo->px < mark) {
					TFCChar(tfcp) = ' ';
					TFCFlags(tfcp) = flags;
					TFCFontID(tfcp) = fontID;
					buildInfo->px += spaceWidth;
					++(buildInfo->tbuffi);
					++tfcp;
					TFCTagID(tfcp) = 0;
				}
				++strp;
				if (buildInfo->px >= xLR) {
					splitLine(buildInfo, buildInfo->breaki,
						  1/*newln*/);
				}
			  }
			break;

			case '\0': {
				if (!buildInfo->makeLinep) {
					keepGoing = 0;
					continue;
				}
				/* special case */
				if (TFCTagID(tfcp)) {
					TFCChar(tfcp) = ' ';
					buildInfo->px += buildInfo->spaceWidth;
					TFCFontID(tfcp) = fontID;
					TFCFlags(tfcp) = flags;
					++(buildInfo->tbuffi);
					++tfcp;
					TFCTagID(tfcp) = 0;
				}
				buildInfo->breaki = buildInfo->tbuffi;
				buildInfo->px += widths['\0'];
				TFCChar(tfcp) = '\0';
				TFCFontID(tfcp) = fontID;
				TFCFlags(tfcp) = flags;

				if (buildInfo->verbatim) {
					splitLine(buildInfo, 
						buildInfo->tbuffi, 1);
				} else {
					int oldbreaki;
					oldbreaki = buildInfo->breaki;
					if (buildInfo->px >= xLR) {
						splitLine(buildInfo, 
							buildInfo->breaki, 1);
					} else {
						splitLine(buildInfo, 
							buildInfo->tbuffi, 1);
					}
				}
				tfcp = buildInfo->tbuff + buildInfo->tbuffi;
				keepGoing = 0;
			}
			break;
			default:
				if (++(buildInfo->tbuffi) > TBUFFSIZE) {
				  /* error. lossing info... */
				  splitLine(buildInfo, buildInfo->breaki, 1);
				  tfcp = buildInfo->tbuff + buildInfo->tbuffi;
				}
				buildInfo->breaki = buildInfo->tbuffi;

				/* in case *strp is an ISOLatin1 char (sigh)*/
				buildInfo->px +=
			XTextWidth(fontInfo[fontID].fontStruct, strp, 1);

				TFCChar(tfcp) = c;
				TFCFontID(tfcp) = fontID;
				TFCFlags(tfcp) = flags;
				++tfcp;
				TFCTagID(tfcp) = 0;
				++strp;
			break;
			}
		}
	}
	return 1;
}

#define VERBOSE_SPLITLINE___
/* this function sucks
 */
int splitLine(buildInfo, split, tagCarriage)
	TFCBuildInfo *buildInfo;
	int split;
	int tagCarriage;
{
	TFChar tfc, *tfcp;
	TFLineNode *newp;
	int size, i, j, k, n, patch = 0;
	char *tagInfoBuff[TAGINFO_SIZE];
	int tagInfoBuffi, tagID;
	int currentFontID, prevFontID = -1;
	TagInfo *ti;
	int newTagID;

	/* special case: don't check font height if the whole line
	 * is a picture.
	 */
	if (!(split == 1 && (TFCFlags(buildInfo->tbuff) & MASK_PIC))) {
	  if (buildInfo->maxFontHeight < FontMaxHeight(buildInfo->fontID)) 
		buildInfo->maxFontHeight = FontMaxHeight(buildInfo->fontID);
	  if (buildInfo->maxFontDescent < FontDescent(buildInfo->fontID))
		buildInfo->maxFontDescent = FontDescent(buildInfo->fontID);
	}
/*
printf("///(now font=%d) buildInfo->maxFontHeight = %d ", buildInfo->fontID, buildInfo->maxFontHeight);
printf(",  buildInfo->maxFontDescent = %d\n", buildInfo->maxFontDescent);
*/
/*  dangerous to use... need to zero @ split if to use...*/

#ifdef VERBOSE_SPLITLINE
	{
	  int i;
	printf(">>>\n");
	  for (i = 0; i < split; i++) 
	    printf("%c", TFCChar(buildInfo->tbuff + i));
	  printf("<<\n");
/*
	dumpTFCArray(DUMP_CHAR, buildInfo->tbuff, buildInfo->buffTagInfo,
				buildInfo->tagInfoCount);
*/
	printf("<<\n");
	}
#endif
	newp = (TFLineNode*)Vmalloc(GET__memoryGroup(buildInfo->self),
					sizeof(struct TFLineNode));
	if (!newp) {
		fprintf(stderr, "Vmalloc failed\n");
		return NULL;
	}

	newp->maxPixelExtentX = 0;
	newp->maxPixelExtentY = 0;
	newp->maxFontHeight = buildInfo->maxFontHeight;
	newp->maxFontDescent = buildInfo->maxFontDescent;
	newp->breakc = 1;
	newp->breakcIsValid = 0;
	newp->length = split;

	tfcp = buildInfo->tbuff + split;
	if (TFCTagID(tfcp)) newp->length++;

	size = sizeof(struct TFChar) * (newp->length + 1);
/*MSTAT_tfed+=size;*/
	newp->linep = (TFChar*)malloc(size);
	if (!newp->linep) {
		fprintf(stderr, "malloc failed\n");
		return NULL;
	}
	bcopy(buildInfo->tbuff, newp->linep, size);
	if (newp->length > 0) {
		/* try to get rid of extraneous space char at end of line,
		 * BUT don't if it's used to hold embedded info 
		 */
		tfcp = newp->linep + newp->length - 1;
		if (TFCChar(tfcp) == ' ' && !TFCTagID(tfcp)) newp->length--;
	}

	TFCClear(newp->linep + newp->length);

	if (newp->length > 0) {
		if (tagCarriage)
			TFCFlags(newp->linep + newp->length-1) |= MASK_NL;
		if (TFCFlags(newp->linep + newp->length - 1) & MASK_BUTTON)
			patch = 1;
	} else {
		if (tagCarriage)
			TFCFlags(newp->linep + newp->length) |= MASK_NL;
	}

	if (buildInfo->tagID + patch > 0) {
		newp->tagInfoCount = buildInfo->tagID + 1;/*wrong*/
		newp->tagInfo = (TagInfo*)malloc(sizeof(struct TagInfo) 
					   * (newp->tagInfoCount + patch));
		for (i = newp->tagInfoCount - 1; i > 0; i--) {
			ti = &(newp->tagInfo[i]);
/*should not malloc	ti->info = saveString(buildInfo->buffTagInfo[i].info);*/
			ti->info = buildInfo->buffTagInfo[i].info;
/*
			ti->x_begin = 0;
			ti->x_end = 0;
			ti->valid = 0;
			printf("## tagID=%d [%s]\n", i, ti->info);
*/
		}
		ti = &(newp->tagInfo[0]); /* old artifact*/
		ti->info = NULL;
/*
		ti->x_begin = 0;
		ti->x_end = 0;
		ti->valid = 0;
*/
	} else {
		newp->tagInfoCount = 0;
		newp->tagInfo = NULL;
	}
	if (patch) {
		newp->tagInfoCount++;
		ti = &(newp->tagInfo[++(buildInfo->tagID)]);
		ti->info = NULL;
/*
		ti->x_begin = 0;
		ti->x_end = 0;
		ti->valid = 0;
*/
		tfed_anchorPatch[tfed_anchorPatchIdx++] = 
			&(newp->tagInfo[buildInfo->tagID].info);
		TFCTagID(newp->linep + newp->length - 1) = buildInfo->tagID;
	}

#ifdef VERBOSE_SPLITLINE
	printf("made line: tagID=%d>> ", buildInfo->tagID);
	dumpTFCArray(DUMP_CHAR|DUMP_WRAP|DUMP_NL, newp->linep, newp->tagInfo,
			newp->tagInfoCount);
	printf("<<\n");
#endif

	/*
	 * Link the new line to tf struct
	 */
	if (buildInfo->currentp) {
		if (buildInfo->currentp) {
			if ((buildInfo->currentp)->next) 
				(buildInfo->currentp)->next->prev = newp;
			newp->prev = buildInfo->currentp;
			newp->next = buildInfo->currentp->next;
			buildInfo->currentp->next = newp;
			buildInfo->currentp = newp;
		} else {
			buildInfo->currentp = newp;
			buildInfo->beginp = newp;
			newp->prev = NULL;
			newp->next = NULL;
		}
	} else {
/*		fprintf(stderr, "Error: splitLine(): currentp = NULL\n");*/
		newp->prev = NULL;
		newp->next = NULL;
		buildInfo->beginp = newp;
		buildInfo->currentp = newp;
	}
	buildInfo->lineNodeCountp++;
	buildInfo->lineVisibleCountp++;
/*	buildInfo->tagID = 0;*/
	buildInfo->vspan += buildInfo->maxFontHeight;
	buildInfo->maxFontHeight = 0;
	buildInfo->maxFontDescent = 0;
	buildInfo->px = GET__TFStruct(buildInfo->self)->xUL;

	k = buildInfo->tbuffi;
	for (n = 0, j = split; j < k; j++)
		if (TFCChar(buildInfo->tbuff + j) != ' ') break;

	for (; j < k; n++, j++) {
		TFPic *picp, *pic = NULL;
		int picID;

		TFCCopy(&tfc, buildInfo->tbuff + j);
		TFCCopy(buildInfo->tbuff + n, &tfc);
		buildInfo->px += FontWidths(buildInfo->fontID)[TFCChar(&tfc)];

		currentFontID = TFCFontID(&tfc);
		if (currentFontID != prevFontID) {

		    if (TFCFlags(&tfc) & MASK_PIC) {
			picID = currentFontID;
			picp = (TFPic*)GET__content(buildInfo->self);
			if (!picp)
				picp = (TFPic*)GET__content(
					GET__parent(buildInfo->self));

			for (; picp; picp = picp->next)
				if (picp->id == picID) {
					pic = picp;
					break;
				}
			if (!pic) pic = dunselPic;
			if (buildInfo->maxFontHeight < pic->height)
				buildInfo->maxFontHeight = pic->height;

		    } else if (TFCFlags(&tfc) & MASK_OBJ) {
			VObj *inset;
			VObjList *olist;
			char *insetName;
			int insetHeight;

		insetName = buildInfo->buffTagInfo[TFCTagID(&tfc)].info;
			if (insetName) {
			  inset = getObject(insetName);
			  if (inset) {
			    insetHeight = GET_height(inset) + 2;
			    if (buildInfo->maxFontHeight < insetHeight)
				buildInfo->maxFontHeight = insetHeight;
			    /* make sure this object is stuck within self.
			     * Tis a problem with collected text.
			     */
			    olist = GET__children(buildInfo->self);
			    SET__children(buildInfo->self,
					appendObjToList(olist, inset));
			    SET__parent(inset, buildInfo->self);
			  }
			}
		    } else {
			if (buildInfo->maxFontHeight < 
			    FontMaxHeight(currentFontID))
				buildInfo->maxFontHeight = 
						FontMaxHeight(currentFontID);
			if (buildInfo->maxFontDescent < 
			    FontDescent(currentFontID))
				buildInfo->maxFontDescent = 
						FontDescent(currentFontID);
		    }
		}
	}

	/* collect tagInfo for split remainder-- this sucks */
	buildInfo->tagID = 0;
	for (i = 0; i < n; i++) {
		tagID = TFCTagID(buildInfo->tbuff + i);
		if (tagID) {
			if (tagID >= TAGINFO_SIZE) {
				fprintf(stderr, 
			"Distress: tagID=%d, exceeds tagInfoCount=%d.\n", 
					tagID, TAGINFO_SIZE);
			} else {
				buildInfo->tagID++;
				buildInfo->buffTagInfo[buildInfo->tagID].info =
					newp->tagInfo[tagID].info;
				buildInfo->buffTagID[buildInfo->tagID] = tagID;
/*
				printf("@@ tagID=%d [%s]\n", 
					tagID, newp->tagInfo[tagID].info);
*/
				newp->tagInfo[tagID].info = NULL;
			}
		}
	}
	/* re-number all tagID to start from 1. fix tagInfo offset 
	 */
/*
	newTagID = 0;
	for (tfcp = buildInfo->tbuff, j = 0; j < n; tfcp++, j++) {
		tagID = TFCTagID(tfcp);
		if (tagID) {
			for (i = buildInfo->tagID; i > 0; i--) {
				if (tagID == buildInfo->buffTagID[i]) {
					TFCTagID(tfcp) = i;
					newTagID++;
					break;
				}
			}
		}
	}
*/
	newTagID = 1;
	for (tfcp = buildInfo->tbuff, j = 0; j < n; tfcp++, j++) {
		tagID = TFCTagID(tfcp);
/*
printf(">>> char='%c'  tagID=%d \n", TFCChar(tfcp), tagID);
*/
		if (tagID) {
/* value not aways char* 
printf(">>>     from [%d]={%s} to [%d]\n",
       tagID, buildInfo->buffTagID[tagID],newTagID);
*/	
			buildInfo->buffTagID[newTagID] =
				buildInfo->buffTagID[tagID];

			TFCTagID(tfcp) = newTagID;
			newTagID++;
		}
	}
	buildInfo->tagID = newTagID;

#ifdef VERBOSE_SPLITLINE
	TFCClear(buildInfo->tbuff + n);
	printf("$$$ TagID=%d>> ", buildInfo->tagID);
	dumpTFCArray(DUMP_CHAR|DUMP_WRAP|DUMP_NL, 
		buildInfo->tbuff, buildInfo->buffTagInfo,
		TAGINFO_SIZE);
	printf("<<\n");
#endif

	buildInfo->tbuffi = n;
	buildInfo->breaki = n;

	return 1;
}

int deleteLineNode(tf)
	TFStruct *tf;
{
	TFLineNode *lp; 
	int ret = 0;

	if (tf->currentp) {
		if (tf->currentp->prev) {
			tf->currentp->prev->next = tf->currentp->next;
		} else {
			/* delete first line */
			/* printf("delete >	first line\n");*/
			tf->firstp = tf->currentp->next;
			tf->offsetp = tf->currentp->next;
		}
		if (tf->currentp->next)
			tf->currentp->next->prev = tf->currentp->prev;
		
		lp = tf->currentp;

		if (tf->currentp->next) {
			tf->currentp = tf->currentp->next;
			ret = 0;
		} else {
			tf->currentp = tf->currentp->prev;
			--(tf->current_row);
			ret = -1;
		}
		if (lp == tf->offsetp) tf->offsetp = tf->currentp;

		if (lp->linep) Vfree(tf->mg, lp->linep);

		Vfree(tf->mg, lp);
	}
/*
printf("refreshMode = SCREEN delteLineNode\n");
	refreshMode = SCREEN;
*/
	return ret;
}

/*
 * insert line above current line, 
 * then set current line pointing to the new line
 */
TFLineNode *insertLineNode(tf, currentp, initLineP)
	TFStruct *tf;
	TFLineNode *currentp;
	int initLineP;
{
	TFLineNode *newp = (TFLineNode *)Vmalloc(tf->mg, 
						sizeof(struct TFLineNode));

	if (!newp) {
		fprintf(stderr, "Vmalloc failed\n");
		return 0;
	}
	newp->prev = currentp->prev;
	newp->next = currentp;
	if (currentp->prev) {
/*		printf("insert before line\n");*/
		currentp->prev->next = newp;
	} else {
/*		printf("insert before first line\n");*/
		tf->currentp = newp;
		tf->firstp = newp;
		tf->offsetp = newp;
	}
	currentp->prev = newp;

	/* the new line has only one character (terminating character) */
	if (initLineP) {
/*Vfree() gets it: newp->linep = (TFChar*)Vmalloc(tf->mg, sizeof(struct TFChar));*/
		newp->linep = (TFChar*)Rmalloc(sizeof(struct TFChar));
		if (!newp->linep) {
			fprintf(stderr, "Rmalloc failed\n");
			return 0;
		}
		TFCClear(newp->linep);
		TFCFontID(newp->linep) = tf->currentFontID;
		newp->length = 0;
	}
	newp->tagInfo = (TagInfo*)Rmalloc(sizeof(struct TagInfo) 
					  * TAGINFO_SIZE);
	bzero(newp->tagInfo, sizeof(int) * TAGINFO_SIZE);
	newp->tagInfoCount = TAGINFO_SIZE;
	newp->maxFontHeight = FontMaxHeight(tf->currentFontID);
	newp->maxFontDescent = FontDescent(tf->currentFontID);
	newp->breakc = 1;
	tf->lineNodeCount++;
	tf->lineVisibleCount += tf->currentp->breakc;

	return newp;
}

TFLineNode *insertBelowLineNode(tf, currentp, initLineP)
	TFStruct *tf;
	TFLineNode *currentp;
	int initLineP;
{
	TFLineNode *newp = (TFLineNode *)Vmalloc(tf->mg, sizeof(TFLineNode));

	if (!newp) {
		fprintf(stderr, "Vmalloc() failed\n");
		return 0;
	}
	newp->prev = currentp;
	if (currentp) {
		newp->next = currentp->next;
		if (currentp->next) {
			currentp->next->prev = newp;
/*			printf("insertBelowLineNode(): insert.\n");*/
		}
		currentp->next = newp;
/*		printf("insertBelowLineNode(): appending to end.\n");*/
	} else {
/*		printf("insertBelowLineNode(): first&only line.\n");*/
		tf->currentp = newp;
		tf->firstp = newp;
		tf->offsetp = newp;
		newp->next = NULL;
	}
	if (initLineP) {
		newp->linep = (TFChar*)Rmalloc(sizeof(struct TFChar));
		if (!newp->linep) {
			fprintf(stderr, "Rmalloc failed\n");
			return 0;
		}
		TFCClear(newp->linep);
		TFCFontID(newp->linep) = tf->currentFontID;
		newp->length = 0;
	}
	newp->tagInfo = (TagInfo*)Rmalloc(sizeof(struct TagInfo) 
					  * TAGINFO_SIZE);
	bzero(newp->tagInfo, sizeof(int) * TAGINFO_SIZE);
	newp->tagInfoCount = TAGINFO_SIZE;
	newp->maxFontHeight = FontMaxHeight(tf->currentFontID);
	newp->maxFontDescent = FontDescent(tf->currentFontID);
	newp->breakc = 1;
	newp->breakcIsValid = 1;
	tf->lineNodeCount++;
	tf->lineVisibleCount += tf->currentp->breakc;

	return newp;
}


char *convertNodeLinesToStr(self, headp)
	VObj *self;
	TFLineNode *headp;
{
	TFLineNode *savep;
	char *newStrp, *cp, c;
	TFChar *tfcp, *tfcline;
	int cci, flag;
	short oldState, state;
	char fontID = (char)GET__font(self);
	char *buffp = buff;
	char s[10];
	
	if (!headp) return NULL;

	/* printf("convertNodeLinesToStr : \n");*/
/*
	savep = headp;
	while (headp) {
		totalLength += headp->length + 1;
		printf("%d,%d\n",totalLength, headp->length);
		headp = headp->next;
	}
	headp = savep;
*/
/*	oldState = TFCFlags(headp->linep);*/
	oldState = 0;

	while (headp) {
/*
		printf("%d>", headp->length);
		dumpTFCArray(DUMP_CHAR|DUMP_WRAP|DUMP_NL, 
				headp->linep, headp->tagInfo,
				headp->tagInfoCount);
		printf("<\n");
*/
		cci = 0;

		tfcp = headp->linep;
		while (tfcp) {
			cci++;
			if (cci >= BUFF_SIZE) {
				fprintf(stderr,
			"convertNodeLinesToStr: buff size exceeded.\n");
				break;
			}

			c = TFCChar(tfcp);
			if (c && (TFCFontID(tfcp) != fontID)) {
				fontID = TFCFontID(tfcp);
				*buffp++ = '\\';
				*buffp++ = 'f';
				*buffp++ = '(';
				if ((int)fontID < 10) {
					*buffp++ = '0' + (int)fontID;
				} else {
					valToStr((int)fontID, s);
					for(cp = s; *cp;)
						*buffp++ = *cp++;
				}
				*buffp++ = ')';
			}
			state = TFCFlags(tfcp) & MASK_BUTTON;
			if (state != oldState) {
				if (oldState & MASK_BUTTON) {
					*buffp++ = '\\';
					*buffp++ = 'b';
					*buffp++ = ')';
				} else if (oldState & MASK_REVERSE) {
					*buffp++ = SHIFT_OUT;
				}
				if (state & MASK_BUTTON) {
					*buffp++ = '\\';
					*buffp++ = 'b';
					*buffp++ = '(';
				} else if (state & MASK_REVERSE) {
					*buffp++ = SHIFT_IN;
				} 
				oldState = state;
			} 
			flag = TFCFlags(tfcp);
			if (flag & MASK_PIC) {
				valToStr((int)TFCFontID(tfcp), s);
				*buffp++ = '\\';
				*buffp++ = 'q';
				*buffp++ = '(';
				for(cp = s; *cp;) *buffp++ = *cp++;
				*buffp++ = ')';
				goto next;
			} 
			if (flag & MASK_OBJ) {
				if (headp->tagInfo) {
				  cp = (char*)headp->tagInfo[
					  TFCTagID(tfcp)].info;
				  if (cp) {
				    *buffp++ = '\\';
				    *buffp++ = 'o';
				    *buffp++ = '(';
				    while (*cp) *buffp++ = *cp++;
				    *buffp++ = ')';
				  }
				} else {
				  fprintf(stderr, "INTERNAL ERROR OCCURED.\n");
				}
				goto next;
			}
			c = TFCChar(tfcp);
			if (c && (TFCFontID(tfcp) != fontID)) {
				fontID = TFCFontID(tfcp);
				*buffp++ = '\\';
				*buffp++ = 'f';
				*buffp++ = '(';
				if ((int)fontID < 10) {
					*buffp++ = '0' + (int)fontID;
				} else {
					valToStr((int)fontID, s);
					for(cp = s; *cp;)
						*buffp++ = *cp++;
				}
				*buffp++ = ')';
			}
			if (TFCTagID(tfcp)) {
				if (headp->tagInfo) {
				  cp = (char*)headp->tagInfo[
					        TFCTagID(tfcp)].info;
				  if (cp) {
				    *buffp++ = '\\';
				    *buffp++ = 'e';
				    *buffp++ = '(';
				    while (*cp) *buffp++ = *cp++;
				    *buffp++ = ')';
				  }
				} else {
				  fprintf(stderr, "INTERNAL ERROR OCCURED.\n");
				}
			}
/*			if (TFCTagID(tfcp)) {
				char abuff[16];
				if (TFCFlags(tfcp) & MASK_PTR) {
					int val = (long)headp->tagInfo
							[TFCTagID(tfcp)].info;
					cp = valToStr(val, abuff);
				} else if (TFCFlags(tfcp) & MASK_STR) {
					cp = (char*)headp->tagInfo
						[TFCTagID(tfcp)].info;
				}
				if (cp) {
					*buffp++ = '\\';
					*buffp++ = 'v';
					*buffp++ = '(';
					while (*cp) *buffp++ = *cp++;
					*buffp++ = ')';
				}
			}
*/			if (c) *buffp++ = c;
			else break;

		next:
			if (flag & MASK_NL) break;
			++tfcp;
		}
		if (headp)
			if (TFCFlags(tfcp) & MASK_NL) *buffp++ = '\n';
			else *buffp++ = ' ';/*?? without this, lines can be jointed together without a space*/
		headp = headp->next;
#ifdef lakjlkjlkjkj
		if (headp) {

/* XXX This is a bug, in that extraneous '\n' *may* be generated. 
 * The solution is to be able to distinquish line break as the text source
 * intended or because it was result of formating line-breaks. 
 * Use a flag...
 */
			*buffp++ = '\n';

/*			if (cci == 2 && (TFCFlags(tfcp - 1) & MASK_PIC)) {
*/
			/* exception: don't add newline if this is 
			 * a very wide picture... 
			 */
		}
#endif
	}
	if (state & MASK_BUTTON) *buffp++ = ')';
	if (state & MASK_REVERSE) *buffp++ = SHIFT_OUT;
	*buffp++ = '\0';

/*printf("buff={%s}\n", buff);*/

	newStrp = (char *)Vmalloc(GET__memoryGroup(self), 
				sizeof(char) * (buffp - buff));
/*MSTAT_tfed_convertNodeLinesToStr+=buffp - buff;*/
	if (!newStrp) {
		fprintf(stderr, "malloc failed\n");
		return 0;
	}
	strcpy(newStrp, buff);

	return newStrp;
}


void freeNodeLines(tf)
	TFStruct *tf;
{
	TFLineNode *nodep, *nextp;

	nodep = tf->firstp;
	while (nodep) {
		nextp = nodep->next;
/*
	printf("freeNodeLine >>");
	dumpTFCArray(DUMP_CHAR, nodep->linep, nodep->tagInfo,
		nodep->tagInfoCount);
	printf("<<\n");
*/
		if (nodep->linep) Vfree(tf->mg, nodep->linep);
		/*XXX need to free tagInfo as well!!! */
		Vfree(tf->mg, nodep);
		nodep = nextp;
	}

	TFCClear(theEditLN->linep);
	theEditLN->length = 0;

	tf->firstp = NULL;
	tf->currentp = NULL;
	tf->offsetp = NULL;
}

void dumpNodeLines(tf)
	TFStruct *tf;
{
	int length;
	TFLineNode *currentp;

printf("current_row=%d current_col=%d\n", tf->current_row, tf->current_col);
printf("screen_row_offset=%d\n", tf->screen_row_offset);

	currentp = tf->offsetp;
/*	lines = tf->num_of_lines;*/

	printf("--offset->tail-------------------\n");
	while (currentp) {

/*	while (currentp && (lines >= 0)) {	
	  	--lines;
*/
		printf("%3d %2d %2d>", 
		       currentp->length, currentp->breakc,
		       currentp->maxFontHeight);

		dumpTFCArray(DUMP_CHAR | DUMP_NL | DUMP_WRAP,
				currentp->linep + tf->screen_col_offset,
				currentp->tagInfo,
				currentp->tagInfoCount);

		printf("<\n");
		currentp = currentp->next;
	}
	printf("-------------------------------\n");
}

int setBreaks(tf, currentp)
	TFStruct *tf;
	TFLineNode *currentp;
{
	int segpx = tf->xUL;
	TFChar *tfcp = currentp->linep;
	int pwidthlimit = tf->xLR;
	TFPic *picp, *pic = NULL;
	int picID;

	currentp->breakc = 0;
	for (tfcp = currentp->linep; TFCChar(tfcp); tfcp++) {
		segpx += glyphWidth(tf, tfcp);
/*
		if (TFCFlags(tfcp) & MASK_PIC) {
			picID = TFCFontID(tfcp);
			picp = (TFPic*)GET__content(tf->self);
			if (!picp)
				picp = (TFPic*)GET__content(
					GET__parent(tf->self));

			for (; picp; picp = picp->next)
				if (picp->id == picID) {
					pic = picp;
					break;
				}
			if (!pic) pic = dunselPic;
			segpx += pic->width;
		} else {
			segpx += TFCWidth(tfcp);
		}
*/
		TFCFlags(tfcp) &= ~MASK_WRAP;
		if (segpx > pwidthlimit) {
			if (TFCFlags(tfcp) & MASK_PIC) {
				TFCFlags(tfcp) |= MASK_WRAP;
			} else if (tfcp > currentp->linep) {
				TFCFlags(tfcp - 1) |= MASK_WRAP;
				currentp->breakc++;
			}
			segpx = tf->xUL;
		}
	}
	currentp->breakc++;
	return currentp->breakc;
}

int setCurrentFontID(tf)
	TFStruct *tf;
{
	TFChar *tfcp;

	tfcp = theEditLN->linep + tf->current_col;
	if (tf->current_col > 0) {
		if (TFCChar(tfcp) == '\0') {
			if (tf->current_col > 0) {
				tf->currentFontID = TFCFontID(tfcp-1);
			} else {
				if (tf->currentp) {
					if (tf->currentp->prev) {
					    tf->currentFontID = 
						TFCFontID(
						   tf->currentp->prev->linep + 
						   tf->currentp->prev->length);
					} else if (tf->currentp->next) {
					    tf->currentFontID = 
						TFCFontID(
						   tf->currentp->prev->linep);
					} else {
						tf->currentFontID = 
							fontID_normal;
					}
				} else {
					tf->currentFontID = fontID_normal;
				}
			}
		} else if (TFCChar(tfcp) == ' ') {
			tf->currentFontID = TFCFontID(tfcp-1);
		} else {
			tf->currentFontID = TFCFontID(tfcp);
		}
	} else {
		tf->currentFontID = TFCFontID(tfcp);
	}
	return tf->currentFontID;
}

int TimedDrawCursor(self, argv, argc)
	VObj *self;
	Packet argv[];		/* dummy */
	int argc;		/* dummy */
{
	TFStruct *tf = updateEStrUser(self);

	if (!tf) return 0;
/*	if (!cursorWithinField(tf)) return 1;*/

	if (tf->cursorTimeInfo) cancelEvent(tf->cursorTimeInfo);
	if (tf->cursorIsVisible == 0) drawCursor(tf);
	tf->cursorIsVisible = 1;
	if (GET_cursor(self) && tf->cursorBlinkDelay != -1)
		tf->cursorTimeInfo = 
			scheduleEvent(tf->cursorBlinkDelay, TimedEraseCursor, 
					self, NULL, NULL);
	else 
		tf->cursorTimeInfo = 0;

	return 1;
}

int TimedEraseCursor(self, argv, argc)
	VObj *self;
	Packet argv[];		/* dummy */
	int argc;		/* dummy */
{
	TFStruct *tf = updateEStrUser(self);

	if (!tf) return 0;
/*	if (!cursorWithinField(tf)) return 1;*/

	if (tf->cursorTimeInfo) cancelEvent(tf->cursorTimeInfo);
	if (tf->cursorIsVisible == 1) eraseCursor(tf);
	tf->cursorIsVisible = 0;
	if (GET_cursor(self) && tf->cursorBlinkDelay != -1)
		tf->cursorTimeInfo = 
			scheduleEvent(tf->cursorBlinkDelay, TimedDrawCursor,
					self, NULL, NULL);
	else 
		tf->cursorTimeInfo = 0;

	return 1;
}

void invertCursor(tf)
	TFStruct *tf;
{
/*printf(">>>>>>> invertCursor(): col=%d row=%d px=%d py=%d row_offset=%d...\n",
	tf->current_col, tf->current_row, 
	tf->csr_px, tf->csr_py,
	tf->screen_row_offset);
*/
	if (!tf) return;
	if (!cursorWithinField(tf)) {
/*printf("invertCursor(): not within field\n");*/
		return;
	}
	if (!tf->currentp) return;
	setCurrentFontID(tf);

	if (FontMaxHeight(tf->currentFontID) == tf->currentp->maxFontHeight) {
		XDrawLine(display, TFWINDOW, gc_invert, 
			tf->csr_px, tf->csr_py, tf->csr_px, 
			tf->csr_py + tf->currentp->maxFontHeight);
	} else {
		int px, py1, py2;

		px = tf->csr_px;
		py1 = tf->csr_py;
		py2 = py1 + tf->currentp->maxFontHeight -
			FontMaxHeight(tf->currentFontID) -
			tf->currentp->maxFontDescent +
			FontDescent(tf->currentFontID);
		XDrawLine(display, TFWINDOW, gc_invert_dash, px, py1, px, py2);
		py1 = py2 + 1;
		py2 += FontMaxHeight(tf->currentFontID);
		XDrawLine(display, TFWINDOW, gc_invert, px, py1, px, py2);
		py1 = py2 + 1;
		py2 += FontDescent(tf->currentFontID);
		XDrawLine(display, TFWINDOW, gc_invert_dash, px, py1, px, py2);
	}
	XFlush(display);
}

void drawCursor(tf)
	TFStruct *tf;
{
	if (!tf) return;
	if (!cursorWithinField(tf)) return;
	if (!tf->currentp) return;
/*	setCurrentFontID(tf);*/

	GLPrepareObjColor(tf->self);

	if (tf->isRenderAble) {
	  XDrawLine(display, TFWINDOW, gc_cr,
		    tf->csr_px - 0, tf->csr_py, 
		    tf->csr_px - 0, 
		    	tf->csr_py + tf->currentp->maxFontHeight - 1);
	}
}

void eraseCursor(tf)
	TFStruct *tf;
{
	if (!tf) return;
	if (!cursorWithinField(tf)) return;
	if (!tf->currentp) return;
/*	setCurrentFontID(tf);*/

	if (tf->isRenderAble) {
		XDrawLine(display, TFWINDOW, gc_bg,
			tf->csr_px, tf->csr_py, 
			tf->csr_px, 
				tf->csr_py + tf->currentp->maxFontHeight - 1);
	}
}

void drawChar(tf, tfcp, px, py)
	TFStruct *tf;
	TFChar *tfcp;
	int px, py;
{
	xcharitem.font = FontFont(TFCFontID(tfcp));
	xcharitem.chars[0] = TFCChar(tfcp);
	XDrawText(display, TFWINDOW, gc_fg, px, py, &xcharitem, 1);
	XFlush(display);
}

TFStruct *updateEStrUser(self)
	VObj *self;
{
	static VObj *currentUserObj = NULL;
	static TFStruct *tf = NULL;

	if (self == NULL) {
		currentUserObj = NULL;
		tf = NULL;
		return NULL;
	} if (self == currentUserObj) {
		return GET__TFStruct(self);
	} else {
		TFStruct *selfTF = GET__TFStruct(self);
		if (currentUserObj) {
			if (tf->editableP)
				replaceNodeLine(tf->currentp, theEditLN,
						1, tf->mg);
		}
		if (selfTF) {
			if (selfTF->currentp) {
				if (selfTF->editableP)
					replaceNodeLine(theEditLN, 
							selfTF->currentp,
							0, NULL);
			}
			currentUserObj = self;
			tf = selfTF;
			return selfTF;
		}
	}
	return NULL;
}

int TFCShiftStr(tfcArray, starti, shift)
	TFChar tfcArray[];
	int starti, shift;
{
	int length, shifts = 0;
	int i, j = 0;

	length = TFCstrlen(tfcArray);
	if (shift > 0) {
		TFCClear(tfcArray + length + shift);
		for (i = length - starti; i > 0; i--) {
			++j;
			TFCCopy(tfcArray + length - j + shift, 
				tfcArray + length - j);
			++shifts;
		}
	} else {
		i = length-starti;
/*		TFCClear(tfcArray + length - shift);*/
		while (i >= 0) {
			TFCCopy(tfcArray + starti + j + shift,
				tfcArray + starti + j);
			--shifts;
			++j;
			--i;
		}
	}
	return shifts;
}

void dumpTFCArray(mask, tfcbuff, tagInfo, tagInfoCount)
	int mask;
	TFChar tfcbuff[];
	TagInfo *tagInfo;
	int tagInfoCount;
{
	TFChar *tfcp;
	int j = 0, tagID;

	for (;;) {
		tfcp = tfcbuff + j;
		if (!tfcp) {
			fprintf(stderr, 
				"ierror: malformed tfc array: tfcp = NULL\n");
			break;
		}
		if (!TFCChar(tfcp)) break;
/*
		printf("%c", TFCChar(tfcp));
*/
		if (TFCTagID(tfcp)) {
			if (TFCTagID(tfcp) > tagInfoCount) {
				printf("Error: tagID > tagInfoCount(%d)!\n",
					tagInfoCount);
			} else {
				if (tagInfo)
					printf("[%d:%s]", 
						TFCTagID(tfcp), 
						tagInfo[TFCTagID(tfcp)].info);
				else
					printf("[%d:?]", TFCTagID(tfcp));
			}
		}
		if (TFCFlags(tfcp) & MASK_REVERSE ||
			TFCFlags(tfcp) & MASK_BUTTON) {
			printf("%s%c%s", 
				enterReverse_vt100, 
				TFCChar(tfcp), 
				leaveReverse_vt100);
		} else {
			if (mask & DUMP_CHAR) putchar(TFCChar(tfcp));
			if (mask & DUMP_FONT) printf("(%d)", TFCFontID(tfcp));
		}
		if (mask & DUMP_WRAP) 
			if (TFCFlags(tfcp) & MASK_WRAP)
				printf("<WRAP>");
		if (mask & DUMP_NL) 
			if (TFCFlags(tfcp) & MASK_NL)
				printf("<NL>");
/*
		printf("%d", TFCFontID(tfcp));
		printf("fontID=%d, ", TFCFontID(tfcp));
		printf("width=%d, ", TFCWidth(tfcp));
		if (TFCFlags(tfcp) & MASK_WRAP) printf("wrap, ");
		printf("\n");
*/
		++j;
	}
}

int translateCol2Px(tfcp, col)
	TFChar *tfcp;
	int col;
{
	int px = 0;

	for (; col; col--, tfcp++) {
		if (TFCChar(tfcp)) px += TFCWidth(tfcp);
		else break;
	}
	return px;
}

int translatePx2Col(tfcp, px)
	TFChar *tfcp;
	int px;
{
	int pi = 0, col = 0;

	for (; TFCChar(tfcp); tfcp++) {
		pi += TFCWidth(tfcp);
		if (pi >= px) break;
		++col;
	}
	return col;
}

void placeCursorWithinStr(tf)
	TFStruct *tf;
{
/*	printf("pcws: %d %d\n",tf->current_col,tf->screen_col_offset);*/
	/* make sure current_col is within the string range */
	if (tf->current_col != tf->current_col_sticky) {
		tf->current_col = tf->current_col_sticky;
	}
	if (tf->current_col > theEditLN->length)
		tf->current_col = theEditLN->length;

	if (cursorWithinField(tf)) placeCursor(tf);
}

/* make use of wrap info 
 */
int countBreaks(tfcp)
	TFChar *tfcp;
{
	int n = 0;

	do {
		if (!TFCChar(tfcp)) return ++n;
		if (TFCFlags(tfcp) & MASK_WRAP) ++n;
	} while (++tfcp);
	return n;
}

void placeCursor(tf)
	TFStruct *tf;
{
	TFLineNode *currentp;
	TFChar *tfcp;
	int i, xpos = tf->xUL, ypos = tf->yUL;

	if (tf->current_row == tf->screen_row_offset) {
		currentp = tf->offsetp;
	} else if (tf->current_row < tf->screen_row_offset) {
		for (currentp = tf->offsetp; ; currentp = currentp->prev) {
			if (!currentp) return;
			if (currentp == tf->currentp) break;
			if (!currentp->prev) break;
			i = currentp->maxFontHeight * currentp->breakc;
/*			printf("*****> ypos=%d i=%d\n", ypos, i);*/
			if (ypos - i < tf->yUL) {
				/* cursor is partially obscured at top, try
				 * to scroll page down */
/*			printf("************> ypos=%d ypos+y=%d\n", ypos, i);*/
				moveOffset(tf, -1, &buffi);
				tfed_scroll_delta(tf, -1);
				updateShown = 1;
				placeCursor(tf);
				return;
			}
			ypos -= i;
		}
	} else if (tf->current_row > tf->screen_row_offset) {
		for (currentp = tf->offsetp; ; currentp = currentp->next) {
			if (!currentp) return;
/*			printf("*****> ypos=%d i=%d\n", ypos, i);*/
			i = currentp->maxFontHeight * currentp->breakc;
			if (ypos + i > tf->yLR) {
				/* cursor is partially obscured at bottom, try
				 * to scroll page up */
/*			printf("************> ypos=%d ypos+y=%d\n", ypos, i);*/
				moveOffset(tf, 1, &buffi);
				tfed_scroll_delta(tf, 1);
				updateShown = 1;
				placeCursor(tf);
				return;
			}
			if (currentp == tf->currentp) break;
			if (!currentp->next) break;
			ypos += i;
		}
	}
	if (currentp) {
		for (i = 0, tfcp = currentp->linep; TFCChar(tfcp); 
		     i++, tfcp++) {
			if (i >= tf->current_col) break;
			if (TFCFlags(tfcp) & MASK_WRAP) {
				xpos = tf->xUL;
				ypos += currentp->maxFontHeight;
			} else {
				xpos += glyphWidth(tf, tfcp);
			}
		}
	}
/*	printf("@@@@@ i=%d xpos=%d ypos=%d \n", i, xpos, ypos);*/

	tf->csr_px = xpos;
	tf->csr_py = ypos;
}


int moveLine(tf, dir)
	TFStruct *tf;
	int dir;
{
	int delta, actual_delta;
	int cursorWasWithinField = cursorWithinField(tf);

	refreshMode = 0;

#ifdef MOVELINE_VERBOSE
	printf("moveLine cursor=%d\n", cursorWasWithinField);
#endif
	replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
	delta = moveLineNode(tf, dir);

	if (tf->editableP)
	  replaceNodeLine(theEditLN, tf->currentp, 0, NULL);

/*	col = translatePx2Col(theEditLN->linep, tf->current_px_sticky); */
	if (tf->current_col < tf->current_col_sticky)
		tf->current_col = tf->current_col_sticky;

	/* if the cursor was within the field before the move, make sure cursor
	 * is remains visible. Move offset if necessary.
	 */
#ifdef MOVELINE_VERBOSE
	printf("moveLine delta=%d\n", delta);
#endif
	if (cursorWasWithinField) {
#ifdef MOVELINE_VERBOSE
	  printf("moveLine 1\n");
#endif
		if (tf->csr_py < 0) {
#ifdef MOVELINE_VERBOSE
		  printf("moveLine 2\n");
#endif			moveOffset(tf, delta, &buffi);
		} else if (tf->csr_py > tf->yLR) {
#ifdef MOVELINE_VERBOSE
		  printf("moveLine 3\n");
#endif
			moveOffset(tf, delta, &buffi);
		} else {
#ifdef MOVELINE_VERBOSE
		  printf("moveLine inside param.\n");
#endif
		}
	} else {
#ifdef MOVELINE_VERBOSE
	  printf("moveLine 5\n");
#endif
	}
	placeCursorWithinStr(tf);

	return delta;
}

int rowAdjustOffset(tf, delta)
	TFStruct *tf;
	int delta;
{
	int actual_offset_delta = 0;

	/* if the cursor was within the field before the move, make sure cursor
	 * is remains visible. Move offset if necessary.
	 */
/*	printf("rowPlaceCursor delta=%d\n", delta);*/
	if (tf->csr_py < 0) {
		actual_offset_delta = moveOffset(tf, delta, &buffi);
	} else if (tf->csr_py > tf->yLR) {
		actual_offset_delta = moveOffset(tf, delta, &buffi);
	}
	return actual_offset_delta;
}

int rowAdjustLine(tf, delta)
	TFStruct *tf;
	int delta;
{
	if (tfed_scroll_delta(tf, delta)) {
		refreshMode = 0;
	} else {
		refreshMode = SCREEN;
	}
	return delta;
}

int moveLineNode(tf, dir)
	TFStruct *tf;
	int dir;
{
	int lineTraversed = 0, linesToTraverse;

	linesToTraverse = abs(dir);
	if (dir == 0) return 0;
	if (dir < 0) {
		/* traverse backward */
		while (lineTraversed < linesToTraverse) {
			if (tf->currentp == NULL) {
				/* whoops! currentp is NULL! */
				break; 
			}
			if (tf->currentp->prev == NULL) {
				/* the current line is the first line.
				 * stop here. */
				break;
			}
			/* there exists a previous line node. 
			 * move to previous line node */
			tf->currentp = tf->currentp->prev;
			if (tf->currentp == tf->firstp) {
				/* reached the first line. */
				tf->firstp = tf->currentp;
			}
			tf->current_row--;
			lineTraversed++;
		}
	} else {

		/* traverse forward */
		while (lineTraversed < linesToTraverse) {
			if (tf->currentp == NULL) {
				/* whoops! currentp is NULL! */
				break; 
			}
			if (tf->currentp->next == NULL) {
				/* the current line is the last line.
				 * stop here. */
				break;
			}
			/* there exists a next line node. 
			 * move to next line node */
			tf->currentp = tf->currentp->next;
			tf->current_row++;
			lineTraversed++;
		}
	}
/*
	if ((tf->current_row < tf->screen_row_offset) ||
	    (tf->current_row > (tf->screen_row_offset + tf->num_of_lines))) {
		refreshMode = SCREEN;
	}
*/
	return dir < 0 ? -lineTraversed : lineTraversed;
}

/*
 * draws a line (string), using the text-field info 
 */
void drawLine(tf, currentp, yoffset)
	TFStruct *tf;
	TFLineNode *currentp;
	int *yoffset;
{
	int fontyoffset, length;

	if (!TFWINDOW || !GET_visible(tf->self)) return;
	if (!cursorWithinField(tf)) {
		/*printf("drawLine: cursor not within field... abort\n");*/
		return;
	}
	fontyoffset = *yoffset + currentp->maxFontHeight - 
			currentp->maxFontDescent;

if (verbose) fprintf(stdout, "tfed: 4 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
		tf->xUL, *yoffset, tf->width,
		*yoffset + currentp->maxFontHeight * currentp->breakc - 1);

	XClearArea(display, TFWINDOW, 
		tf->xUL, *yoffset, tf->width,
		*yoffset + currentp->maxFontHeight * currentp->breakc - 1,
		False);
/*
huh?
	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}
*/
}

int drawLineSeg(tf, currentp, yoffset, fontyoffset)
	TFStruct *tf;
	TFLineNode *currentp;
	int *yoffset;
	int *fontyoffset;
{
	TFChar *segheadtfcp = currentp->linep;
	TFChar *tfcp = currentp->linep;
	int segendpx, segpx;
	int pwidthlimit = tf->xLR;
	int stat, nosegdrawn = 0;
	XTextItem item;
	Window w = TFWINDOW;
	int x1, y1, x2, y2;
	short prevFontID, prevVideo;
	char *buffp;
	int flags;
	GC usegc = gc_fg;
	int localYOffset = *yoffset;

	if (tf->align == PANE_CONFIG_CENTER) {
		segpx = (tf->xLR - tf->xUL - lineSegWidth(tf, currentp)) / 2;
	} else if (tf->align == PANE_CONFIG_E2W) {
		segpx = tf->xLR - tf->xUL - lineSegWidth(tf, currentp);
	} else {
		segpx = tf->xUL;
	}

	item.delta = 0;
	item.chars = buff;

	while (TFCChar(tfcp)) {

		/* grab the next segment 
		 */
		segheadtfcp = tfcp;
		segendpx = segpx;

		prevFontID = TFCFontID(tfcp);
		prevVideo = TFCFlags(tfcp) & 
			(MASK_REVERSE | MASK_RELIEF | MASK_BUTTON |
			 MASK_PIC | MASK_OBJ);

		buffp = buff;
		for (;;) {
			if (prevFontID == TFCFontID(tfcp) &&
			     prevVideo == (TFCFlags(tfcp) & 
				(MASK_REVERSE | MASK_RELIEF | MASK_BUTTON | 
				 MASK_PIC | MASK_OBJ))) {

				flags = TFCFlags(tfcp);
				flags &= ~MASK_WRAP;
				if (flags & MASK_PIC) {
					segendpx += glyphWidth(tf, tfcp);
				} else if (flags & MASK_OBJ) {
					VObj *inset;
					char *insetName;
					insetName = 
					    currentp->tagInfo[TFCTagID(tfcp)].info;
					if (insetName) {
					    inset = getObject(insetName);
					    if (inset) {
					      segendpx += GET_width(inset) + 2;
					    }
					}
					++tfcp;
					stat = 2;
					break;
				} else {
					segendpx += glyphWidth(tf, tfcp);
				}
				if (segendpx > pwidthlimit &&
				    tfcp > segheadtfcp) {
 					*buffp = '\0';
					TFCFlags(tfcp - 1) |= MASK_WRAP;
					stat = 1;
					break;
				} else {
					if (!(*buffp = TFCChar(tfcp))) {
						stat = 0;
						break;
					}
					buffp++;
					tfcp++;
				}
			} else {
				stat = 2;
				break;
			}
		}

		if (!*buff) {
			if (TFCWidth(tfcp) > pwidthlimit) {
				/* hey, what's the deal? looks like 
				 * problem is a tiny window. so, 
				 * instead of not drawing anything, 
				 * draw obscured character, in the
				 * hope of hinting the user to
				 * enlarges the window.
				 */
				segendpx += glyphWidth(tf, tfcp);
				buff[0] = TFCChar(tfcp);
				tfcp++;
				stat = 1;
			}
		}

/*
		buff[tfcp - segheadtfcp] = '\0';
		printf(">>drawLineSeg: stat=%d, segment = [%s]\n", stat, buff);
*/

		/* drawing the segment 
		 */
		if (*buff) {
			item.font = FontFont(TFCFontID(segheadtfcp));
			item.nchars = tfcp - segheadtfcp;

			flags = TFCFlags(segheadtfcp);
			if (!flags) {
				XDrawText(display, w, gc_fg, 
					segpx, *fontyoffset, &item, 1);
			} else {
				if (!(flags & (MASK_PIC | MASK_OBJ))) {
				    if (flags & MASK_REVERSE) {
					XFillRectangle(display, 
						w, gc_fg,
						segpx, 
						localYOffset,
						segendpx - segpx, 
						currentp->maxFontHeight);
					usegc = gc_invert;
				    } else usegc = gc_fg;
				    XDrawText(display, w, usegc, segpx, 
					*fontyoffset, &item, 1);
				}
				if (flags & MASK_BUTTON) {
					x1 = segpx;
					y1 = localYOffset;
					x2 = segendpx; 
					y2 = localYOffset+currentp->maxFontHeight;

/*					if (lookAndFeel == LAF_BOX_TXT){
						XDrawRectangle(display, w, 
							gc_mesh, x1, y2-2, 
							x2-x1-2, 1);
						XDrawRectangle(display, w, 
							gc_mesh, x2-2, y1, 
							1, y2-y1-2);
						XDrawLine(display, w, gc_bd, 
							x1, y1, x2-1, y1);
						XDrawLine(display, w, gc_bd, 
							x1, y1, x1, y2-1);
						XDrawLine(display, w, gc_bd, 
							x1+1, y1+1, x1+1,y2-2);
					}
*/
					if (flags & MASK_RELIEF) usegc = gc_cr;
					else usegc = gc_mesh;
					XDrawRectangle(display, w, usegc,
							x1, y2-2, 
							x2-x1-2, 1);
				}

/*				if (flasg & MASK_UNDER) {
					XDrawLine(display, w, gc_fg,
						segpx, *fontyoffset + PYY,
						segendpx, *fontyoffset + PYY);
					usegc = gc_fg;
				}
*/
/*				if (flags & MASK_XRULE) {
					int y = localYOffset 
						+ currentp->maxFontHeight / 2;
					XDrawLine(display, w, gc_fg,
						tf->xUL, y,
						tf->xLR, y);
					usegc = gc_fg;
				}
*/
				if (flags & MASK_PIC) {
					TFPic *picp, *pic = NULL;
					int picID;

					picID = TFCFontID(segheadtfcp);
/*
 * because of the round about way of gather data for <P> objects,
 * in HTML the pic list would be found in the parent object, not
 * the Paragraph object itself. Stored in parent's __content slot.
 * quite a hack.
 */
					picp = (TFPic*)GET__content(tf->self);
					if (!picp)
						picp = (TFPic*)GET__content(
							GET__parent(tf->self));

					for (; picp; picp = picp->next)
						if (picp->id == picID) {
							pic = picp;
							break;
						}
					if (!pic) pic = dunselPic;

/*					segendpx += pic->width;
*/

					if (pic->type == TFPic_XBML) {
						GLDisplayXBM(w, 
							segpx+2,
							localYOffset+1,
							pic->width, 
							pic->height, 
							pic->data);

					} else if (pic->type == TFPic_GIF ||
					           pic->type == TFPic_XBM) {
						GLGIFDraw(w, pic->data,
							segpx+2,
							localYOffset+1,
							pic->width, 
							pic->height);
					}
					if (flags & MASK_BUTTON) {
					    if (flags & MASK_RELIEF) {
						usegc = gc_cr;
					    } else {
						usegc = gc_mesh;
					    }
					    XDrawRectangle(display, w, 
							usegc,
							segpx+1,
							localYOffset+1, 
							pic->width-1,
							pic->height-2); 
					    XDrawRectangle(display, w, 
							usegc,
							segpx,
							localYOffset, 
							pic->width+1, 
							pic->height); 
					}
					goto near;
				}
				if (flags & MASK_OBJ) {
					VObj *inset;
					char *insetName;
					insetName = 
			  currentp->tagInfo[TFCTagID(segheadtfcp)].info;
					if (insetName) {
						inset = getObject(insetName);

		if (inset) {
			int x = GET_x(inset), y = GET_y(inset);
			if (x != segpx || y != localYOffset) {
				SET_x(inset, segpx);
				SET_y(inset, localYOffset);
			}
			if (GET_window(inset)) {
/*
fprintf(stderr, "updating win for obj %s  insetName=%s w=%x %d %d\n", 
GET_name(inset), insetName, GET_window(inset),segpx, localYOffset);
*/
				GLUpdatePosition(!GET_parent(inset),
					GET_window(inset),
					segpx, localYOffset);
			}
			if (!GET_window(inset))
				sendMessage1N1int(inset, "visible", 1);
		}
					}
				}
			}
		} else {
			/* just in case */
			if (++nosegdrawn > 2) break;
		}
	near:
		if (tf->editableP)
		  if (*fontyoffset + currentp->maxFontHeight > tf->height) {
			int x1 = tf->xUL-1;
			int x2 = tf->xLR;
			int y2 = tf->yLR+1;
		      	/* bottom edge */
			XDrawLine(display, w, gc_bg_lighter, x1+1, y2, x2, y2);
			XDrawLine(display, w, gc_bg_lighter, x1+2, y2-1, x2, y2-1);
		  }

		if (stat == 0) {
			/* end of line */
			break;
		} else if (stat == 1) {
			/* carriage break to next line */
			segpx = tf->xUL;
			*fontyoffset += currentp->maxFontHeight;
			localYOffset += currentp->maxFontHeight;
			if (tf->wrap == 0) break;
		} else {
			/* continue */
			segpx = segendpx;
		}
	}
	return 1;
}

/* find out total width */
int lineSegWidth(tf, currentp)
	TFStruct *tf;
	TFLineNode *currentp;
{
	TFChar *segheadtfcp = currentp->linep;
	TFChar *tfcp = currentp->linep;
	int segendpx, segpx = tf->xUL;
	int pwidthlimit = tf->xLR;
	int stat, nosegdrawn = 0;
	XTextItem item;
	int x1, y1, x2, y2;
	short prevFontID, prevVideo;
	char *buffp;
	int flags;
	GC usegc = gc_fg;

	item.delta = 0;
	item.chars = buff;

	while (TFCChar(tfcp)) {

		/* grab the next segment 
		 */
		segheadtfcp = tfcp;
		segendpx = segpx;

		prevFontID = TFCFontID(tfcp);
		prevVideo = TFCFlags(tfcp) & 
			(MASK_REVERSE | MASK_RELIEF | MASK_BUTTON |
			 MASK_PIC | MASK_OBJ);

		buffp = buff;
		for (;;) {
			if (prevFontID == TFCFontID(tfcp) &&
			     prevVideo == (TFCFlags(tfcp) & 
				(MASK_REVERSE | MASK_RELIEF | MASK_BUTTON | 
				 MASK_PIC | MASK_OBJ))) {

				flags = TFCFlags(tfcp);
				flags &= ~MASK_WRAP;
				if (flags & MASK_PIC) {
					segendpx += glyphWidth(tf, tfcp);
				} else if (flags & MASK_OBJ) {
					VObj *inset;
					char *insetName;
					insetName = 
					    currentp->tagInfo[TFCTagID(tfcp)].info;
					if (insetName) {
					    inset = getObject(insetName);
					    if (inset) {
					      segendpx += GET_width(inset) + 2;
					    }
					}
					++tfcp;
					stat = 2;
					break;
				} else {
					segendpx += glyphWidth(tf, tfcp);
				}
				if (segendpx > pwidthlimit &&
				    tfcp > segheadtfcp) {
					TFCFlags(tfcp - 1) |= MASK_WRAP;
					stat = 1;
					break;
				} else {
					if (!(*buffp = TFCChar(tfcp))) {
						stat = 0;
						break;
					}
					buffp++;
					tfcp++;
				}
			} else {
				stat = 2;
				break;
			}
		}

		if (!*buff) {
			if (TFCWidth(tfcp) > pwidthlimit) {
				/* hey, what's the deal? looks like 
				 * problem is a tiny window. so, 
				 * instead of not drawing anything, 
				 * draw obscured character, in the
				 * hope of hinting the user to
				 * enlarges the window.
				 */
				segendpx += glyphWidth(tf, tfcp);
				tfcp++;
				stat = 1;
			}
		}
	near:
		if (stat == 0) {
			/* end of line */
			break;
		} else if (stat == 1) {
			/* carriage break to next line */
			if (tf->wrap == 0) break;
		} else {
			/* continue */
			segpx = segendpx;
		}
	}
	return segendpx;
}

/* draws a line(string), using the text-field info 
 * line linesOffset is offset from first line on screen
 */
int drawLineOffset(tf, linesOffset, clearBG)
	TFStruct *tf;
	int linesOffset;
	int clearBG;
{
	int i, yoffset, fontyoffset;
	TFLineNode *currentp;

	if (!TFWINDOW || !GET_visible(tf->self)) return 0;

	yoffset = tf->yUL;

	/* get i'th line from offset */
	currentp = tf->offsetp;
	for (i = 0; i < linesOffset; i++) {
		yoffset += currentp->maxFontHeight * currentp->breakc;
		currentp = currentp->next;
		if (!currentp) return 0;
	}

	fontyoffset = yoffset + currentp->maxFontHeight - 
			currentp->maxFontDescent;

	if (clearBG && tf->isRenderAble) {
/*
fprintf(stdout, "tfed: 5 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
	tf->xUL, yoffset, tf->width, 
	currentp->maxFontHeight * currentp->breakc);
*/
		XClearArea(display, TFWINDOW, tf->xUL, yoffset, tf->width, 
			   currentp->maxFontHeight * currentp->breakc, False);
	}
	drawLineSeg(tf, currentp, &yoffset, &fontyoffset);

	return 1;
}

int renderTF(tf)
	TFStruct *tf;
{
	TFLineNode *currentp;
	TFChar *linep, *tfcp;
	int yoffset, fontyoffset, maxFontHeight, maxFontDescent, pwidthlimit;
	int yoffset_s, fontyoffset_s;
	int segpx;
	int fontID, lfontID = -1;
	FontInfo *fip;

	if (tf->self == NULL) {
	  fprintf(stderr, 
		  "Hey, what's the idea. renderTF(): tf->self==NULL\n");
	  return 0;
	}
	if (!TFWINDOW || !GET_visible(tf->self)) return 0;

	pwidthlimit = tf->xLR;
	currentp = tf->offsetp;
	yoffset = tf->yUL;
	fontyoffset = tf->yUL;

	GLPrepareObjColor(tf->self);

	XClearWindow(display, TFWINDOW);

	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}

	while (currentp) {
		linep = currentp->linep;

		if (!linep || (yoffset > tf->height)) break;
/*
printf("renderTF: {");
dumpTFCArray(DUMP_CHAR|DUMP_WRAP|DUMP_NL, 
	&(currentp->linep[tf->screen_col_offset]), currentp->tagInfo,
	currentp->tagInfoCount);
printf("}\n");

*/
		/* scan the line to find maxFontHeight */
		segpx = tf->xUL;

		if (currentp->breakcIsValid == 0) {
			setBreaks(tf, currentp);
			currentp->breakcIsValid = 1;
		}

		if (!TFCChar(linep)) {
			fip = &fontInfo[TFCFontID(linep)];
			maxFontHeight = currentp->maxFontHeight = 
				fip->maxheight;
			maxFontDescent = currentp->maxFontDescent = 
				fip->descent;
		} else {
			maxFontHeight = 0;
			maxFontDescent = 0;
			lfontID = -1;
			for (tfcp = linep; TFCChar(tfcp); ++tfcp) {
				if (segpx > pwidthlimit) break;
				if (TFCFlags(tfcp) & MASK_PIC) {
					TFPic *pic = NULL, *picp, *pics;
					int picID;

					picID = TFCFontID(tfcp);
					picp = (TFPic*)GET__content(tf->self);
					if (!picp)
						picp = (TFPic*)GET__content(
							GET__parent(tf->self));
					for (; picp; picp = picp->next)
						if (picp->id == picID) {
							pic = picp;
							break;
						}
					if (!pic) pic = dunselPic;
					segpx += pic->width;
					if (maxFontHeight < pic->height)
						maxFontHeight = pic->height;
				} else if (TFCFlags(tfcp) & MASK_OBJ) {
					VObj *inset;
					char *insetName;
					int insetHeight;

					insetName = 
				currentp->tagInfo[TFCTagID(tfcp)].info;
					if (insetName) {
					  inset = getObject(insetName);
					  if (inset) {
					    segpx += GET_width(inset) + 2;
					    insetHeight = GET_height(inset)+2;
					    if (maxFontHeight < insetHeight)
						maxFontHeight = insetHeight;
					  }
					}
				} else {
					segpx += TFCWidth(tfcp);
					if (TFCFontID(tfcp) != lfontID) {
					    fontID = TFCFontID(tfcp);
					    fip = &fontInfo[fontID];
					    if (maxFontHeight < fip->maxheight)
						maxFontHeight = fip->maxheight;
					    if (maxFontDescent < fip->descent)
						maxFontDescent = fip->descent;
					    lfontID = fontID;
					}
				}
			}
			currentp->maxFontHeight = maxFontHeight;
			currentp->maxFontDescent = maxFontDescent;
		}

		/* this can't be right. what if this is the first line,
		 * and breakc > 1? in that case, fontyoffset starts NOT
		 * at the top of the window!
		 */
		fontyoffset = yoffset + (maxFontHeight - maxFontDescent);
/*
print("BEFORE2 fy=%d y=%d bc=%d\n", fontyoffset, yoffset, currentp->breakc);
*/
		/*
		 * extract and draw segments in the current line
		 */
		drawLineSeg(tf, currentp, &yoffset, &fontyoffset);
/*
print("AFTER1  fy=%d y=%d bc=%d\n", fontyoffset, yoffset, currentp->breakc);
print("AFTER2  fy_s=%d\n", fontyoffset_s);
*/

/*		tf->num_of_lines += currentp->breakc;
*/
		yoffset += maxFontHeight * currentp->breakc;
/*
		if (doWraping) {
			segpx = tf->xUL;
			yoffset += FontMaxHeight(fontID);
			fontyoffset += FontMaxHeight(fontID);
			doWraping = 0;
		}
*/
		currentp = currentp->next;
	}
	if (tf->currentp) {
	  if (tf->editableP)
	    replaceNodeLine(theEditLN, tf->currentp, 0, NULL);
	} else {
/*		printf("Internal error: tf->currentp == NULL.\n");*/
	}

	GLDrawBorder(TFWINDOW, 0, 0, 
		     GET_width(tf->self)-1, GET_height(tf->self)-1,
		     GET_border(tf->self), 1);

	return 1;
}
 
/*
 * scroll lower half page downward,
 * push open `delta' number of lines, beginning at `offset' rows
 */
int scrollDownLowerPart(tf, offset, span)
	TFStruct *tf;
	int offset;
	int span;
{
	TFLineNode *currentp;
	int i, upper = 0;

	currentp = tf->offsetp;
	for (i = 0; i <= offset; i++) {
		if (!currentp) break;
		upper += currentp->maxFontHeight * currentp->breakc;
		currentp = currentp->next;
	}
	XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
		tf->xUL, tf->yUL + upper,
		tf->width, tf->height - upper - span,
		tf->xUL, tf->yUL + upper + span);
if (verbose) fprintf(stdout, "tfed: 6 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, span, False);
	XClearArea(display, TFWINDOW, 
			tf->xUL, tf->yUL + upper,
			tf->width, span, False);
	return 1;
}

int scrollDownLowerPartNEW(tf, offset, span)
	TFStruct *tf;
	int offset;
	int span;
{
	TFLineNode *currentp;
	int i, upper = 0;

	currentp = tf->offsetp;
	for (i = 0; i < offset; i++) {
		if (!currentp) break;
		upper += currentp->maxFontHeight * currentp->breakc;
		currentp = currentp->next;
	}
	XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
		tf->xUL, tf->yUL + upper,
		tf->width, tf->height - upper - span,
		tf->xUL, tf->yUL + upper + span);
if (verbose) fprintf(stdout, "tfed: 7 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, span, False);
	XClearArea(display, TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, span, False);
	return 1;
}

int scrollUpLowerPart(tf, offset, span)
	TFStruct *tf;
	int offset;
	int span;
{
	TFLineNode *currentp = tf->offsetp;
	int i, upper = 0;
	int py = tf->yUL, probepy, limit, d;

	for (i = 0; i <= offset; i++) {
		if (!currentp) break;
		upper += currentp->maxFontHeight * currentp->breakc;
		currentp = currentp->next;
	}
	if (span > 0) {
		int safety = tf->height - upper - span;
		if (safety > 0) {
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
				tf->xUL, tf->yUL + upper + span,
				tf->width, safety,
				tf->xUL, tf->yUL + upper);
		}
if (verbose) fprintf(stdout, "tfed: 8 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yLR - span,
			tf->width, span);
		XClearArea(display, TFWINDOW, 
			tf->xUL, tf->yLR - span,
			tf->width, span, False);
	} else {
if (verbose) fprintf(stdout, "tfed: 9 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, tf->height - upper);
		XClearArea(display, TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, tf->height - upper, False);
	}
	py = upper;
	probepy = tf->yUL;
	limit = tf->height - py - span;
	for (; ; currentp = currentp->next) {
		if (!currentp) break;
		d = currentp->maxFontHeight * currentp->breakc;
		probepy += d;

/*		printf("***** d=%d probepy=%d\n", d, probepy);*/

		if (probepy >= limit) break;
		py += d;
		++i;
	}
	for (; currentp; currentp = currentp->next) {
		if (py >= tf->yLR) break;

/*		   printf("***** drawing line %d, py=%d\n", i, py);*/

		drawLineOffset(tf, i, 0);
		i++;
		py += currentp->maxFontHeight * currentp->breakc;
	}
	return 1;
}

int scrollUpLowerPartNEW(tf, offset, span)
	TFStruct *tf;
	int offset;
	int span;
{
	TFLineNode *currentp = tf->offsetp;
	int i, upper = 0;

	for (i = 0; i < offset; i++) {
		if (!currentp) break;
		upper += currentp->maxFontHeight * currentp->breakc;
		currentp = currentp->next;
	}
	if (span > 0) {
		int safety = tf->yLR - upper - span;
		if (safety > 0) {
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
				tf->xUL, tf->yUL + upper + span,
				tf->width, safety,
				tf->xUL, tf->yUL + upper);
		}
if (verbose) fprintf(stdout, "tfed: 10 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, tf->height - upper);
		XClearArea(display, TFWINDOW, 
			tf->xUL, tf->yLR - span,
			tf->width, span, False);
	} else {
if (verbose) fprintf(stdout, "tfed: 12 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, tf->height - upper);
		XClearArea(display, TFWINDOW,
			tf->xUL, tf->yUL + upper,
			tf->width, tf->height - upper, False);
	}
	return 1;
}

#ifdef nofnfnfn
/*
 * collect characters to shift to next line
 *
 * start counting characters to shift down to next visible line,
 * all chars until the one with wrap tag (the old wrap sentury) or EOL.
 * results: col, pwidth
 *
 * Returns: 1 if not the end-of-line.
 *          0 if end-of-line is encountered.
 */
int collectCharsToShift(tf, currentp, colp, pwidthp, delta)
	TFStruct *tf;
	TFLineNode *currentp;
	int *colp;
	int *pwidthp;
	int *delta;
{
	TFChar *tfcp = currentp->linep;

	*pwidthp = 0;
	*delta = 0;

	for (tfcp += *colp; TFCChar(tfcp); tfcp++) {
/*		printf("{%c}", TFCChar(tfcp));*/
		*pwidthp += TFCWidth(tfcp);
		++(*delta);
		++(*colp);
		if (TFCFlags(tfcp) & MASK_WRAP) {
			TFCFlags(tfcp) &= ~MASK_WRAP;
			/*printf(" is old sentury\n");*/
			return 1;
		}
	}

	/* push down other lines */
	if (!scrollDownLowerPart(tf, 
		tf->current_row - tf->screen_row_offset + PYY,
		currentp->maxFontHeight)){
		renderTF(tf);
		return 0;
	}
	currentp->breakc++;
	return 1;
}
#endif

/* travere to right-side edge of field 
 * move probepx to just one char before crossing the right edge
 */
int traverseToRightEdge(tfcArray, pwidthlimit, pxp, colp)
	TFChar *tfcArray;
	int pwidthlimit;
	int *pxp;
	int *colp;
{
	TFChar *tfcp;
	int probepx = *pxp;
	int probecol = *colp;

	tfcp = tfcArray + probecol;
/*
	printf("*****************************************************\n");
	printf("(%c]", TFCChar(tfcp));
	printf("col=%d px=%d pwidthlimit=%d TFCWidth=%d\n", 
		*colp, *pxp, pwidthlimit, TFCWidth(tfcArray + *colp));
*/
	tfcp = tfcArray + probecol;
	probepx = *pxp;
	for (;;) {
		++probecol;
/*		printf("[%c] tagID=%d  ", TFCChar(tfcp), TFCTagID(tfcp));*/

		probepx += TFCWidth(tfcp);
/*		printf("probecol=%d probepx=%d pwidthlimit=%d TFCWidth=%d\n", 
			probecol, probepx, pwidthlimit, TFCWidth(tfcp));
*/
		if (TFCChar(tfcp) == '\0') {
/*			printf("no need to wrap (EOL).\n");*/
			return 0;
		}
		if (probepx >= pwidthlimit) {
			/* move this and following characters, until the
			 * previous wrapper.
			 * the character right before this is the new 
			 * breaker.
			 */
/*			printf(">>>> new breaker [%c]\n", TFCChar(tfcp - 1));
			printf(">>>>[%c]", TFCChar(tfcp));
			printf("col=%d px=%d pwidthlimit=%d TFCWidth=%d\n", 
				*colp, *pxp, pwidthlimit, TFCWidth(tfcp));
*/
			return 1;
		}
		if (TFCFlags(tfcp) & MASK_WRAP) {
			/*printf("no need to wrap (WRAP).\n");*/
			return 0;
		}
		*pxp = probepx;
		*colp = probecol;
		tfcp = tfcArray + probecol;
	}
}

#define VERBOSE_SCROLLINEFORWARD__
int scrollLineForward(tf, delta, col)
	TFStruct *tf;
	int delta;
	int col;
{
	int sy;
	int pwidth = 0;
	TFChar *tfcp, *linep;
	int px, py, i, hasMore = 1;
	int exitAfterPatch = 0;
	TFLineNode *currentp = theEditLN;

	linep = currentp->linep;

	if (!linep) return 0;/*???*/

	px = tf->csr_px;
	py = tf->csr_py;
	sy = py + currentp->maxFontHeight - currentp->maxFontDescent;
	currentp->breakc = 0;

	for (i = 0; i < delta; i++)
		pwidth += TFCWidth(linep + col + i - delta);

	for (i = 0; i < col; i++)
		if (TFCFlags(linep + i) & MASK_WRAP) currentp->breakc++;
	currentp->breakc++;

	while (hasMore) {

		/* shift line forward 
		 */
		if (pwidth == 0) return 1; /* hmm... */
#ifdef VERBOSE_SCROLLINEFORWARD
		printf("** displace: pwidth=%d px=%d \n", pwidth, px);
#endif
		if (TFWINDOW) {
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy0, 
				  px, py,
				  tf->xLR - tf->xUL - px - pwidth + 1,
				  currentp->maxFontHeight, 
				  px + pwidth, py);

#ifdef VERBOSE_SCROLLINEFORWARD
fprintf(stdout, "tfed: 33 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
				px, py,
				pwidth, currentp->maxFontHeight, False);
#endif
			XClearArea(display, TFWINDOW, 
				   px, py,
				   pwidth, currentp->maxFontHeight, 
				   False);
		}
		/* patch the void created by shifting line forward
		 */
#ifdef VERBOSE_SCROLLINEFORWARD
		printf("@@@@@ col=%d delta=%d\n", col, delta);
#endif
		for (i = col - delta; i < col; i++) {
#ifdef VERBOSE_SCROLLINEFORWARD
			printf("## (%c) i=%d px=%d sy=%d\n", 
				TFCChar(linep + i), i, px, sy);
#endif

			drawChar(tf, linep + i, px, sy);
			px += TFCWidth(linep + i);
		}
		if (exitAfterPatch) {
#ifdef VERBOSE_SCROLLINEFORWARD
			printf("****** exitAfterPatch\n");
#endif
			return 1;
		}
		hasMore = traverseToRightEdge(linep, tf->xLR, 
					      &px, &col);
		if (!hasMore) break;
		exitAfterPatch = !hasMore;

#ifdef VERBOSE_SCROLLINEFORWARD
		printf("~~~~ exitAfterPatch=%d\n", exitAfterPatch);
		printf("**2 px=%d col=%d hasMore=%d\n", px, col, hasMore);
#endif
		/* set the new wrap sentury
		 */
		if (col > 0) {
#ifdef VERBOSE_SCROLLINEFORWARD
			printf("### <%c> col=%d is new sentury\n",
				TFCChar(linep + col - 1), col - 1);
#endif
			TFCFlags(linep + col - 1) |= MASK_WRAP;
			currentp->breakc++;

#ifdef VERBOSE_SCROLLINEFORWARD
			printf("chars to move to next line:\n");
			dumpTFCArray(DUMP_CHAR|DUMP_FONT, linep + col, NULL,
				currentp->tagInfoCount);
			printf("\n");
#endif
			if (col > 1) {
				if (TFCFlags(linep + col - delta - 1) &
					MASK_WRAP) {
					TFCFlags(linep + col - delta - 1) &=
						~MASK_WRAP;
/*					printf("XXXXXXX no more\n");*/
					return 1;
				}
			}
		} else {
/*			printf("internal error: tfed.c: scroll_line().\n");*/
			return 0;
		}
		
		/* count characters to shift down to next visible line
		 */
		currentp->maxFontHeight = currentp->maxFontHeight;

		if (!tf->currentp) return 0;

		tf->currentp->maxFontHeight = currentp->maxFontHeight;

		tfcp = currentp->linep;
		pwidth = 0;
		delta = 0;

		for (tfcp += col; TFCChar(tfcp); tfcp++) {
#ifdef VERBOSE_SCROLLINEFORWARD
	printf("{%c}", TFCChar(tfcp));
#endif
			pwidth += TFCWidth(tfcp);
			++delta;
			++col;
			if (TFCFlags(tfcp) & MASK_WRAP) {
				TFCFlags(tfcp) &= ~MASK_WRAP;
				/*printf(" is old sentury\n");*/
				goto done;
			}
		}
		/* push down other lines */
		if (!scrollDownLowerPart(tf, 
			tf->current_row - tf->screen_row_offset,
			currentp->maxFontHeight)){
			renderTF(tf);
		}
		currentp->breakc++;
		tf->currentp->breakc++;
		if (tf->currentp == tf->offsetp) 
			tf->offsetp->breakc = tf->currentp->breakc;
done:

		/* erase the characters that was moved to the next visible line
		 */
#ifdef VERBOSE_SCROLLINEFORWARD
		printf("*** px=%d py=%d pwidth=%d\n", px, py, pwidth);
		printf("*** paramdx=%d px=%d\n", tf->xLR, px);
#endif
		i = tf->xLR - px;
		if (i > 1 && TFWINDOW) {
#ifdef VERBOSE_SCROLLINEFORWARD
fprintf(stdout, "tfed: 13 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
					px, py, 
					i, currentp->maxFontHeight);
#endif
			XClearArea(display, TFWINDOW, 
					px, py, 
					i, currentp->maxFontHeight, False);
		}
		px = tf->xUL;
		py += currentp->maxFontHeight;
		sy += currentp->maxFontHeight;

#ifdef VERBOSE_SCROLLINEFORWARD
		printf(">> col=%d px=%d sy=%d pwidth=%d delta=%d\n",
			col, px, sy, pwidth, delta);
#endif
	}
	return 1;
}


/*in tfed2.c: int scrollLineBackward(tf, delta, col)*/

int tfed_scroll_delta(tf, offsetdir)
	TFStruct *tf;
	int offsetdir;
{
	int i, h, linesToMove, span, limit, py, probepy, fonty;
	TFLineNode *currentp;

	if (!TFWINDOW || !GET_visible(tf->self)) return 0;

	if (offsetdir == 0) return 0;
	else if (offsetdir > 0) linesToMove = offsetdir;
	else linesToMove = -offsetdir;

	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}
	if (offsetdir < 0) {
		/* scroll page downward
		 */
		span = 0;
		currentp = tf->offsetp;
		for (i = 0; i < linesToMove; i++) {

			if (currentp->breakcIsValid == 0) {
				setBreaks(tf, currentp);
				currentp->breakcIsValid = 1;
			}
			span += currentp->maxFontHeight * currentp->breakc;
			if (span > tf->height) return 0;
			currentp = currentp->next;
			if (!currentp) return 0;
		}
		XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
			  tf->xUL, tf->yUL,
			  tf->width, tf->height - span,
			  tf->xUL, tf->yUL + span);

if (verbose) fprintf(stdout, "tfed: 20 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
				0, 0,
				tf->width, span);

		XClearArea(display, TFWINDOW,
				tf->xUL, tf->yUL,
				tf->width, span, False);
		
		currentp = tf->offsetp;
		py = tf->yUL;

		if (currentp->breakcIsValid == 0) {
			setBreaks(tf, currentp);
			currentp->breakcIsValid = 1;
		}

		probepy = py + (currentp->maxFontHeight * currentp->breakc);
		fonty = py + currentp->maxFontHeight 
		        - currentp->maxFontDescent;
		for (;;) {
		  /*printf("down span=%d py=%d probepy=%d fonty=%d\n", 
		    span, py, probepy, fonty);*/
			drawLineSeg(tf, currentp, &py, &fonty);
			if (probepy >= span) break;
			if (!(currentp = currentp->next)) break;
			if (currentp->breakcIsValid == 0) {
				setBreaks(tf, currentp);
				currentp->breakcIsValid = 1;
			}
			py = probepy;
			probepy += currentp->maxFontHeight * currentp->breakc;
			fonty = py + currentp->maxFontHeight 
			        - currentp->maxFontDescent;
		}
		return 1;
	} else {
		/* scroll page upward
		 */
		span = 0;
		currentp = tf->offsetp;
		for (i = 0; i < linesToMove; i++) {
			if (currentp->prev) currentp = currentp->prev;
			else break;
			span += currentp->maxFontHeight * currentp->breakc;
			if (span > tf->height) return 0;
		}
		py = probepy = tf->yUL;
		limit = tf->yUL + tf->height - span;
		currentp = tf->offsetp;
		for (;;) {
			probepy += currentp->maxFontHeight * currentp->breakc;
			if (probepy >= limit) break;
			py = probepy;
			
			if (!(currentp = currentp->next)) break;
		}
		if (currentp) {
			fonty = py + currentp->maxFontHeight 
		        	- currentp->maxFontDescent;
		} else {
			fonty = py;
		}
		XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy, 
				tf->xUL, tf->yUL + span,
				tf->width, tf->height - span,
				tf->xUL, tf->yUL);
if (verbose) fprintf(stdout, "tfed: 21 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
				tf->xUL, py,
				tf->width, tf->height - py);
		XClearArea(display, TFWINDOW,
				tf->xUL, py,
				tf->width, tf->height - py + tf->yUL, False);
		if (currentp) {
			for (;;) {
/*printf("up span=%d py=%d probepy=%d fonty=%d\n", span, py, probepy, fonty);*/
		    drawLineSeg(tf, currentp, &py, &fonty);
				if (probepy >= tf->yLR) return 1;
				if (!(currentp = currentp->next)) break;
				py = probepy;
				probepy += currentp->maxFontHeight 
					* currentp->breakc;
				fonty = py + currentp->maxFontHeight 
			        	- currentp->maxFontDescent;
			}
		}
		return 1;
	}
}

int mapFromPixelToCharPosition(tf, px, py, cx, cy, exact_px, exact_py)
	TFStruct *tf;
	int px, py;
	int *cx, *cy;
	int *exact_px, *exact_py;
{
	TFLineNode *currentp;
	TFChar *tfcp;
	int ycheck;
	int gWidth;

	*cx = 0;
	*cy = 0;
	*exact_py = ycheck = tf->yUL; 
	*exact_px = tf->xUL;

	/* for optimization, if mx is > csr_py then start fron currentp...*/

	for (currentp = tf->offsetp; currentp; currentp = currentp->next) {
		*cx = 0;
		*exact_py = ycheck;
		ycheck += currentp->maxFontHeight;
/*printf("py=%d epy=%d cy=%d ycheck=%d\n", py, *exact_py, *cy, ycheck);*/
		for (tfcp = currentp->linep; tfcp;) {
			if (!TFCChar(tfcp)) {
				if (py >= *exact_py && py < ycheck) {
					return 2;
				}
				break;
			}
			if (py >= *exact_py && py < ycheck) {
/*
printf("* py=%d epy=%d cy=%d ycheck=%d\n", py, *exact_py, *cy, ycheck);
printf("* px=%d epx=%d cx=%d\n", px, *exact_px, *cx);
*/
				gWidth = glyphWidth(tf, tfcp);
				*exact_px += gWidth;

				if (px <= *exact_px) {
				    if (TFCFlags(tfcp) & MASK_PIC) {
					    /* slide to left side */
					    *exact_px -= gWidth;
					    return 1;
				    } else {
					if (*exact_px - px > (gWidth / 2)) {
					    /* slide to left side */
					    *exact_px -= gWidth;
					    return 1;
					} else {
					    /* slide to right side */
					    if (TFCFlags(tfcp) & MASK_WRAP) {
							*exact_px = tf->xUL; 
							*exact_py = ycheck;
					    }
					    (*cx)++;
					    return 1;
					}
 				    }
				}
			}
			if (TFCFlags(tfcp) & MASK_WRAP) {
				if (py >= *exact_py && py < ycheck) {
					*exact_px = tf->xUL;
					*exact_py = ycheck;
					(*cx)++;
					return 1;
				}
				*exact_px = tf->xUL; 
				*exact_py = ycheck;
				ycheck += currentp->maxFontHeight;
			}
			(*cx)++;
			tfcp++;
		}
		*exact_px = tf->xUL; 
		(*cy)++;
	}
	return 1;
}

#define VERBOSE_TFED_EXPOSE 0
/*
 * an unsophisticated expose handler (simple mod of renderTF())
 */ 
int tfed_expose(self, x, y, width, height)
	VObj *self;
	int x, y, width, height;
{
	TFStruct *tf = updateEStrUser(self);
	TFLineNode *currentp;
	int probey, yoffset, fontyoffset;
	int bottom;
	int i = 1;

	if (!tf) return 0;
	if (!TFWINDOW || !GET_visible(tf->self)) return 0;

	currentp = tf->offsetp;
	yoffset = probey = tf->yUL;

/*printf(">> y=%d \n", y);*/

	if (BDPixel == BGPixel) {
		XSetForeground(display, gc_mesh, FGPixel);
		XSetBackground(display, gc_mesh, BGPixel);
		XSetForeground(display, gc_bd, FGPixel);
	} else {
		XSetForeground(display, gc_mesh, BDPixel);
	}

	while (currentp) {
		if (!currentp->linep) return 1;
if (VERBOSE_TFED_EXPOSE) 
	printf(" i = %d... probey=%d  yoffset=%d\n", 
	       i, probey, yoffset);

		if (tf->currentp->breakcIsValid == 0) {
			setBreaks(tf, tf->currentp);
			tf->currentp->breakcIsValid = 1;
		}

/*XXXXXX WHY multiply by breakc??? this is probably wrong. */
		probey += currentp->maxFontHeight * currentp->breakc;
		if (probey >= y) break;

		yoffset = probey;
		currentp = currentp->next;
	}
	bottom = y + height;

if (VERBOSE_TFED_EXPOSE) 
	printf("bottom=%d \n", bottom);

	while (currentp) {
		if (!currentp->linep) return 1;

		fontyoffset = yoffset + currentp->maxFontHeight - 
				currentp->maxFontDescent;

		if (yoffset >= bottom) break;

if (VERBOSE_TFED_EXPOSE) 
	printf("*i = %d... bottom=%d  yoffset=%d  fontyoffset=%d\n", 
	       i++, bottom, yoffset, fontyoffset);

		drawLineSeg(tf, currentp, &yoffset, &fontyoffset);

		yoffset = probey;
		currentp = currentp->next;
		if (currentp) {
			probey += currentp->maxFontHeight * currentp->breakc;
			if (tf->currentp->breakcIsValid == 0) {
				setBreaks(tf, tf->currentp);
				tf->currentp->breakcIsValid = 1;
			}
		}
	}
	return 1;
}

int tfed_append(tf, str)
	TFStruct *tf;
	char *str;
{
/*	TFLineNode *currentp;*/
	TFLineNode *insertp = tf->currentp;
	TFChar tbuff[TBUFFSIZE]; /*XXX*/
	int tbuffi = 0;
	int setTops = 0;
	int tagID = 0;
	TFCBuildInfo buildInfo;

	tf->building_maxFontHeight = 0;
	tf->building_maxFontDescent = 0;

	if (!tf) return 0;
	if (insertp) {
		if (!TFCChar(insertp->linep) && !insertp->next) {
			insertp = NULL;
			setTops = 1;
		} else {
			while (insertp->next) insertp = insertp->next;
		}
	} else {
		setTops = 1;
	}

	buildInfo.self = tf->self;
	buildInfo.currentp = insertp;
	buildInfo.beginp = tf->firstp;
	buildInfo.str = str;
	buildInfo.fontID = tf->currentFontID;
	buildInfo.lineNodeCountp = tf->lineNodeCount;
	buildInfo.lineVisibleCountp = tf->lineVisibleCount;
	buildInfo.tbuff = tbuff;
	buildInfo.tbuffi = tbuffi;
	buildInfo.tagID = tagID;
	buildInfo.flags = 0;
	buildInfo.maxFontHeight = FontMaxHeight(buildInfo.fontID);
	buildInfo.maxFontDescent = FontDescent(buildInfo.fontID);

	buildInfo.vspan = tf->building_vspan;

	buildInfo.px = tf->xUL; 
	buildInfo.breaki = 0;
	buildInfo.makeLinep = 1;
	buildInfo.vspan = 0;
	buildInfo.verbatim = GET_verbatim(tf->self);
	buildInfo.format = GET_format(tf->self);

	tfed_buildLines(&buildInfo);

	if (setTops) {
		tf->offsetp = tf->currentp = tf->firstp;
	}

	if (helper_txtDisp_updateShownInfo(tf)) {
		VObjList *objl;
		for (objl = GET__shownDepend(tf->self); objl; 
			objl = objl->next) {
			if (objl->o) {
				sendMessage1N2int(objl->o,
					"shownInfoV", 
					GET_shownPositionV(tf->self),
					GET_shownSizeV(tf->self));
			}
		}
	}

	tf->firstp = buildInfo.beginp;
	tf->offsetp = buildInfo.beginp;
	tf->currentp = buildInfo.currentp;
	tf->building_vspan += buildInfo.vspan;
	if (tf->building_vspan > tf->yLR) tf->yLR = tf->building_vspan;

/*
	currentp = insertp;
	replaceNodeLine(theBuffLN, currentp, 1, tf->mg);
	TFCstrcpy(theBuffLN->linep, currentp);
	TFCstrcpy(theBuffLN->linep + theBuffLN->length, currentp);
	TFCstrcpy(theEditLN->linep + theEditLN->length, tf->insertp->linep);
	replaceNodeLine(currentp, theBuffLN, 0, NULL);
*/
	return 1;
}

int LogicOrTFCFlag(tfcp, from, to, val)
	TFChar *tfcp;
	int from;
	int to;
	int val;
{
	int i = from;

	tfcp += from;
	do {
		if (!TFCChar(tfcp)) break;
		TFCFlags(tfcp) |= val;
		tfcp++;
	} while (i < to);
	return i;
}

int LogicAndTFCFlag(tfcp, from, to, val)
	TFChar *tfcp;
	int from;
	int to;
	int val;
{
	int i = from;

	tfcp += from;
	do {
		if (!TFCChar(tfcp)) break;
		TFCFlags(tfcp) &= val;
		tfcp++;
	} while (i < to);
	return i;
}

/* mode: RANGEMODE_NONE none
 *       RANGEMODE_CLIP clip       
 *       RANGEMODE_DELETE delete
 */
char *rangeOperation(tf, from_cx, from_cy, to_cx, to_cy, 
			drawP, underlineP, mode)
	TFStruct *tf;
	int from_cx, from_cy, to_cx, to_cy;
	int drawP, underlineP, mode;
{
	TFLineNode *currentp = tf->firstp, *lastp = NULL, *nextp;
	TFChar *tfcp;
	int left, right, i, buffi = 0; 
	int rangeHint; /* 0=not set, 1=all, 2=mid, 3=left, 4=right*/

	if (to_cy == from_cy) {
		if (to_cx < from_cx) {
			i = from_cx;
			from_cx = to_cx;
			to_cx = i;
		}
	} else if (to_cy < from_cy) {
		i = from_cy;
		from_cy = to_cy;
		to_cy = i;

		i = from_cx;
		from_cx = to_cx;
		to_cx = i;
	}
	for (i = 0; i < from_cy; i++) {
		if (!currentp) return NULL;
		currentp = currentp->next;
		if (!currentp) return NULL; /* error */
	}
	if (i <= to_cy) {
		for (;;) {
			if (from_cy == to_cy) {	
				if (from_cx <= to_cx) {
					left = from_cx;
					right = to_cx-1;
				} else {
					left = to_cx;
					right = from_cx-1;
				}
				rangeHint = 2;
			} else {
				if (i == from_cy) {
					left = from_cx;
					right = currentp->length;
					rangeHint = 4;
				} else if (i == to_cy) {
					left = 0;
					right = to_cx+1;
					rangeHint = 3;
				} else {
					left = 0;
					right = currentp->length;
					rangeHint = 1;
				}
			}

			if (!currentp) return NULL;

			tfcp = currentp->linep + left;

			/* yucky code... there ought to be a law...
			 */
			if (mode == RANGEMODE_DELETE) {
				if (rangeHint == 1) {
					/* delete the whole line */
#ifdef RANGE_OPERATION_VERBOSE
printf("deleteing this whole line: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
					nextp = currentp->next;
					if (lastp)
				  		lastp->next = currentp->next;
					if (currentp->next)
						currentp->next->prev = lastp;
					if (currentp->linep) {
						Vfree(tf->mg, currentp->linep);
						Vfree(tf->mg, currentp);
					}
					currentp = nextp;
					goto toNextLine;
				} else if (rangeHint == 3) {
#ifdef RANGE_OPERATION_VERBOSE
printf("(left-e)before: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
 					TFCShiftStr(currentp->linep, right+1,
						    -right-1);
#ifdef RANGE_OPERATION_VERBOSE
printf("after: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
				} else if (rangeHint == 4) {

#ifdef RANGE_OPERATION_VERBOSE
printf("(right-e)before: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
					currentp->length = left;
					TFCClear(currentp->linep + 
						 currentp->length);
#ifdef RANGE_OPERATION_VERBOSE
printf("after: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
				} else {
#ifdef RANGE_OPERATION_VERBOSE
printf("(mid)before: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
					TFCShiftStr(currentp->linep, right+1,
						    -(right-left)-1);
#ifdef RANGE_OPERATION_VERBOSE
printf("after: >>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
dumpTFCArray(DUMP_CHAR, currentp->linep, NULL, currentp->tagInfoCount);
printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
#endif
				}
			} else {
			  if (underlineP == 0) { 
				do {
					if (!TFCChar(tfcp)) {
						if (mode == RANGEMODE_CLIP) 
							buff[buffi++] = '\n';
						break;
					}
					if (mode == RANGEMODE_CLIP) 
						buff[buffi++] = TFCChar(tfcp);
					tfcp++;
				} while (left++ < right);
			  } else if (underlineP == 1) { 
				do {
					if (!TFCChar(tfcp)) {
						if (mode == RANGEMODE_CLIP) 
							buff[buffi++] = '\n';
						break;
					}
					TFCFlags(tfcp) |= MASK_REVERSE;
					if (mode == RANGEMODE_CLIP) 
						buff[buffi++] = TFCChar(tfcp);
					tfcp++;
				} while (left++ < right);
			  } else if (underlineP == -1) { 
				do {
					if (!TFCChar(tfcp)) {
						if (mode == RANGEMODE_CLIP) 
							buff[buffi++] = '\n';
						break;
					}
					TFCFlags(tfcp) &= ~MASK_REVERSE;
					if (mode == RANGEMODE_CLIP) 
						buff[buffi++] = TFCChar(tfcp);
					tfcp++;
				} while (left++ < right);
			  }
			}
			if (drawP == 1) {
				drawLineOffset(tf, i - tf->screen_row_offset, 
						1); /*XXX terribly slow...*/
			} else if (drawP == -1) {
				drawLineOffset(tf, i - tf->screen_row_offset, 
						0); /*XXX terribly slow...*/
			}
			lastp = currentp;
			currentp = currentp->next;
		toNextLine:
			if (!currentp) break;
			if (++i > to_cy) break;
		}
	}
	if (mode == RANGEMODE_CLIP) {
		buff[buffi] = '\0';
		return buff;
	} else {
		return NULL;
	}
}

int lineFlagSet(tf, ln, cn, mask, op)
	TFStruct *tf;
	int ln, cn;
	int mask;
	int op;
{
	TFLineNode *currentp = tf->firstp;
	TFChar *tfcp;
	int left, right, i;

	if (!currentp) return 0;
	while (ln-- > 0) {
		currentp = currentp->next;
		if (!currentp) return 0;
	}
	tfcp = currentp->linep;
	if (op > 0) {
		for (; TFCChar(tfcp); tfcp++) TFCFlags(tfcp) |= mask;
	} else if (op < 0) {
		for (; TFCChar(tfcp); tfcp++) TFCFlags(tfcp) &= ~mask;
	}
	if (currentp == tf->currentp) {
	  if (tf->editableP)
	    replaceNodeLine(theEditLN, tf->currentp, 0, NULL);
	}
	return 1;
}


/*
 * Used to set num_of_lines without rendering
 * bug: does not take breakc into account 
 */
int scanVerticalMetrics(tf)
	TFStruct *tf;
{
	TFLineNode *currentp;
	TFChar *linep, *tfcp;
	int yoffset, maxFontHeight, maxFontDescent, pwidthlimit;
	int fontID = 0, lfontID = -1;
	int segpx;
	FontInfo *fip;
	int inViewP = 0;
	int old_num_of_lines;
	int lineNodeCount = 0;
	int lineVisibleCount = 0;
	int vspan = 0;
	TFPic *picp, *pic = NULL;
	int picID;
	int renderedLines;

	pwidthlimit = tf->xLR;
	currentp = tf->firstp;
	yoffset = tf->yUL;

	old_num_of_lines = tf->num_of_lines;
	tf->num_of_lines = 0;

	while (currentp) {
		linep = currentp->linep;
		if (!linep) break;

		lineNodeCount++;
		lineVisibleCount++;
		renderedLines = 1;

		if (currentp == tf->offsetp) inViewP = 1;
		if (!TFCChar(linep)) {
			fip = &fontInfo[TFCFontID(linep)];
			maxFontHeight = currentp->maxFontHeight = 
				fip->maxheight;
			maxFontDescent = currentp->maxFontDescent = 
				fip->descent;
		} else {
			maxFontHeight = 0;
			maxFontDescent = 0;
			currentp->breakc = 1;
			lfontID = -1;
			segpx = tf->xUL;

/*XXX clumsy safety*/
			tfcp = linep;
			if (tfcp->fontID >= maxNumOfFonts) {
				fprintf(stderr, 
			"internal error detected. Trying to recover...\n");
				tf->num_of_lines = old_num_of_lines;
				return 0;
			}
			for (; TFCChar(tfcp); ++tfcp) {
				segpx += glyphWidth(tf, tfcp);
				if (segpx > pwidthlimit) {
					if (tfcp > currentp->linep) {
						TFCFlags(tfcp-1) |= MASK_WRAP;
						currentp->breakc++;
						lineVisibleCount++;
						if (inViewP) {
						    renderedLines++;
						    if (yoffset > tf->height) {
							tf->num_of_lines +=
								renderedLines;
							inViewP = 0;
						    }
						}
						yoffset += maxFontHeight;
/*
printf("WRAP     lineVisibleCount=%d renderedLines=%d\n", 
lineVisibleCount, renderedLines);
*/
					}
					segpx = tf->xUL;
				}
				if (TFCFontID(tfcp) != lfontID || 
				    (TFCFlags(tfcp) & (MASK_PIC | MASK_OBJ))) {
/*XXX inefficient to call glyphWidth() which does the following op as well*/
				    if (TFCFlags(tfcp) & MASK_PIC) {
					picID = TFCFontID(tfcp);
					picp = (TFPic*)GET__content(tf->self);
					if (!picp)
					     picp = (TFPic*)GET__content(
						GET__parent(tf->self));

					for (; picp; picp = picp->next)
						if (picp->id == picID) {
							pic = picp;
							break;
						}
					if (!pic) pic = dunselPic;
					if (maxFontHeight < pic->height)
						maxFontHeight = pic->height;
				    } else if (TFCFlags(tfcp) & MASK_OBJ) {
					VObj *inset;
					char *insetName = NULL;
					int insetHeight;
					if (currentp->tagInfo) {
					  insetName = 
					   currentp->tagInfo[TFCTagID(tfcp)].info;
					} else {
TFCFlags(tfcp) &= ~MASK_OBJ; /*damage control*/
TFCTagID(tfcp) = 0; /*damage control*/
printf("tfed.c: error: currentp->tagInfo == NULL.\n");
					}
					if (insetName) {
					  inset = getObject(insetName);
					  if (inset) {
					    insetHeight = GET_height(inset)+2;
					    if (maxFontHeight < insetHeight)
						maxFontHeight = insetHeight;
					  }
					}
				    } else {
					fontID = TFCFontID(tfcp);

					fip = &fontInfo[fontID];
					if (maxFontHeight < fip->maxheight)
						maxFontHeight = fip->maxheight;
					if (maxFontDescent < fip->descent)
						maxFontDescent = fip->descent;
				    }
				    lfontID = fontID;
				}
			}
			currentp->maxFontHeight = maxFontHeight;
			currentp->maxFontDescent = maxFontDescent;
		}
		if (inViewP) {
			if (yoffset > tf->height) inViewP = 0;
			tf->num_of_lines += renderedLines; /* in view only */
			yoffset += maxFontHeight;
		}
		vspan += maxFontHeight;
		currentp = currentp->next;
	}
	
	tf->lineNodeCount = lineNodeCount;
	tf->lineVisibleCount = lineVisibleCount; /* whole doc */
	tf->building_vspan = vspan;

	return 1;
}


int tfed_setReverseMaskInButtonRange(tf, boolval)
	TFStruct *tf;
	int boolval;
{
	TFChar *tfcp, *tfcpOrig;
	int i = tf->current_col;

	if (tf->editableP) tfcp = theEditLN->linep + i;
	else {
		if (!tf->currentp) return 0;
		tfcp = tf->currentp->linep + i;
	}
	tfcpOrig = tfcp;
	if (boolval) {
		/* to the beginning of line direction */
		for (i = tf->current_col; i >= 0; i--) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) |= MASK_REVERSE;
/*			print("<%c", TFCChar(tfcp));
*/
			--tfcp;
		}
		/* to the end of line direction */
		tfcp = tfcpOrig + 1;
		for (i = tf->current_col + 1; i < tf->currentp->length; i++) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) |= MASK_REVERSE;
/*			print(">%c", TFCChar(tfcp));
*/
			++tfcp;
		}
/*		print("\n");*/
	} else {
		/* to the beginning of line direction */
		for (i = tf->current_col; i >= 0; i--) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) &= ~MASK_REVERSE;
/*			print("<%c", TFCChar(tfcp));
*/
			--tfcp;
		}
		/* to the end of line direction */
		tfcp = tfcpOrig + 1;
		for (i = tf->current_col + 1; i < tf->currentp->length; i++) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
/*			print(">%c", TFCChar(tfcp));
*/
			TFCFlags(tfcp) &= ~MASK_REVERSE;
			++tfcp;
		}
/*		print("\n");*/
	}
	return 1;
}

int tfed_setReliefMaskInButtonRange(tf, boolval)
	TFStruct *tf;
	int boolval;
{
	TFChar *tfcp, *tfcpOrig;
	int i = tf->current_col;

	if (tf->editableP) tfcp = theEditLN->linep + i;
	else {
		if (!tf->currentp) return 0;
		tfcp = tf->currentp->linep + i;
	}
	tfcpOrig = tfcp;
	if (boolval) {
		/* to the beginning of line direction */
		for (i = tf->current_col; i >= 0; i--) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) |= MASK_REVERSE;
			TFCFlags(tfcp) |= MASK_RELIEF;
/*			print("<%c", TFCChar(tfcp));
*/
			--tfcp;
		}
		/* to the end of line direction */
		tfcp = tfcpOrig + 1;
		for (i = tf->current_col + 1; i < tf->currentp->length; i++) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) |= MASK_REVERSE;
			TFCFlags(tfcp) |= MASK_RELIEF;
/*			print(">%c", TFCChar(tfcp));
*/
			++tfcp;
		}
/*		print("\n");*/
	} else {
		/* to the beginning of line direction */
		for (i = tf->current_col; i >= 0; i--) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
			TFCFlags(tfcp) &= ~MASK_REVERSE;
			TFCFlags(tfcp) &= ~MASK_RELIEF;
/*			print("<%c", TFCChar(tfcp));
*/
			--tfcp;
		}
		/* to the end of line direction */
		tfcp = tfcpOrig + 1;
		for (i = tf->current_col + 1; i < tf->currentp->length; i++) {
			if (!(TFCFlags(tfcp) & MASK_BUTTON)) break;
/*			print(">%c", TFCChar(tfcp));
*/
			TFCFlags(tfcp) &= ~MASK_REVERSE;
			TFCFlags(tfcp) &= ~MASK_RELIEF;
			++tfcp;
		}
/*		print("\n");*/
	}
	return 1;
}

TFPic *tfed_addPicFromFile(pics, id, src)
	TFPic **pics;
        char *id;
	char *src;
{
	TFPic *pic, *picp;
	int largestID = 0, len, span = 0;
	char *ext;

	pic = (TFPic*)malloc(sizeof(struct TFPic));
	len = strlen(id);

	for (ext = id + len; ext >= id; ext--) {
		if (*ext == '.') break;
		if (*ext == '/' || span++ > 6) {
			/* assume extensions are <6 chars */
			break;
		}
	}
	if (!STRCMP(ext, ".xbm")) pic->type = TFPic_XBM;
	else if (!STRCMP(ext, ".gif")) pic->type = TFPic_GIF;
	else if (!STRCMP(ext, ".xpm")) pic->type = TFPic_XPM;
	else {
		/* play dumb and put no the dunsel icon */
		pic->type = TFPic_XBML;
		pic->id = dunselPic->id;
		pic->width = dunselPic->width;
		pic->height = dunselPic->height;
		pic->data = dunselPic->data;
		pic->canFree = 0;
		return pic;
	}

	pic->data = NULL;


	if (pic->type == TFPic_XBML) {
		int hotx, hoty;
		Pixmap bitmap;

		if (XReadBitmapFile(display, rootWindow, id, 
				&(pic->width), &(pic->height),
				&bitmap, &hotx, &hoty) == 0) {
			pic->data = bitmap;
			pic->canFree = PK_CANFREE_STR;
		} else {
			pic->canFree = 0;
		}
		/* clean up mess created by HTTPGet() in HMML_img */
		tfed_registerTmpFileToFree(src);
/*		unlink(src);*/
	} else if (pic->type == TFPic_XBM ||
		   pic->type == TFPic_GIF) {
		XImage *theImage;
		int width, height;

		if (GLGIFLoad(id, src, rootWindow, 0, 0, 0, 0, 
			&(pic->width), &(pic->height), &theImage)) {
			pic->data = theImage;
			pic->canFree = PK_CANFREE_STR;
		} else {
			pic->canFree = 0;
		}

		pic->width += 2; /* to blindly accomodate for hotlink border*/
		pic->height += 2;

		/* clean up mess created by HTTPGet() in HMML_img */
		tfed_registerTmpFileToFree(src);
/*		unlink(src);*/
	} else {
		pic->canFree = 0;
	}
	if (pic->data) {
		pic->next = NULL;
		if (*pics) {
			for (picp = *pics; picp; picp = picp->next) {
				if (picp->id > largestID) largestID = picp->id;
				if (!picp->next) break;
			}
			picp->next = pic;
		} else {
			*pics = pic;
		}
		pic->id = largestID + 1;
		return pic;
	}
	free(pic);
	return NULL;
}

TFPic *tfed_addPic(pics, oldpic)
	TFPic **pics;
	TFPic *oldpic;
{
	TFPic *pic, *picp;
	int largestID = 0, len, span = 0;
	char *ext;

	pic = (TFPic*)malloc(sizeof(struct TFPic));

	bcopy(oldpic, pic, sizeof(struct TFPic));

	if (pic->data) {
		pic->next = NULL;
		if (*pics) {
			for (picp = *pics; picp; picp = picp->next) {
				if (picp->id > largestID) largestID = picp->id;
				if (!picp->next) break;
			}
			picp->next = pic;
		} else {
			*pics = pic;
		}
		pic->id = largestID + 1;
		return pic;
	}
	free(pic);
	return NULL;
}


/* temporary hack...
 */
int filesToFreeCount = 0;
char *filesToFree[20];

void tfed_registerTmpFileToFree(f)
	char *f;
{
	if (filesToFreeCount >= 19) {
/*		fprintf(stderr, "warning, garbage collection in progress.\n");
*/
		tfed_FreeTmpFileToFree(1);
		filesToFreeCount = 0;
	}
	filesToFree[filesToFreeCount++] = saveString(f);
}

void tfed_FreeTmpFileToFree(freeP)
	int freeP;
{
	int i = filesToFreeCount;
	
	if (freeP) {
		char *s;
		while (i) {
			s = filesToFree[--i];
			unlink(s);
			free(s);
		}
	} else
		while (i) unlink(filesToFree[--i]);
}

