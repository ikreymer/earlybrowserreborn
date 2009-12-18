/*
 * Xl.h --
 *
 * Author: Teemu Rantanen <tvr@cs.hut.fi>
 * Copyright (c) 1992 Teemu Rantanen
 *                    All rights reserved
 *
 * Created: Tue Mar 10 20:43:22 1992 tvr
 * Last modified: Wed May 13 06:21:11 1992 tvr
 *
 */


/*
 * Prototypes
 */
int XlFormatText(int leftmargin, int width, int topmargin,
		  int *vwidth, int *vheight, HText_t * htext);

int XlSetupResources(void *(*func) (void *, char *));

int XlSetupText(Display * display, Window window,
		 unsigned long fg,
		 unsigned long bg,
		 HText_t * htext);

void XlSetPageCoordinates(int virtualx, int virtualy, HText_t * htext);

void XlGetCoordinates(int *virtualx, int *virtualy,
		       HTextObject_t * htextobject);

HTextObject_t *XlLocateHTextObject(int windowx, int windowy,
				    HText_t * htext);

void XlDeleteText(HText_t * htext);

void XlRedraw(int windowx, int windowy, int width, int height,
	       HText_t * htext);

void XlMoveWindow(int newvx, int newvy, HText_t * htext);

void XlClearWindow(int width, int height, HText_t * htext);

void XlSetCursor(HText_t * htext, HTextObject_t * object);
