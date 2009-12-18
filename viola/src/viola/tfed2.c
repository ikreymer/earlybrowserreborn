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
 * tfed2.c 
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

/* delete to left and pull 
 */
int kbf_delete_backward_char(tf)
	TFStruct *tf;
{
	if (theEditLN->length > 0) {
		if (tf->highLiteFrom_cx != -1) {
			int i;
			TFLineNode *currentp;

			replaceNodeLine(tf->currentp, theEditLN, 1, tf->mg);
			xselectionObj = NULL;

			tf->current_col_sticky = 
			tf->current_col = 
				(tf->highLiteFrom_cx < tf->highLiteTo_cx ?
				  tf->highLiteFrom_cx : tf->highLiteTo_cx);
			tf->current_row = 
				(tf->highLiteFrom_cy < tf->highLiteTo_cy ?
				  tf->highLiteFrom_cy : tf->highLiteTo_cy);

			/* move cursor pixel coords to the upper-left edge */
			tf->csr_py = tf->yUL;
			currentp = tf->offsetp;
			for (i = tf->screen_row_offset; 
			     i < tf->current_row; i++) {
				tf->csr_py += currentp->maxFontHeight *
						currentp->breakc;
				currentp = currentp->next;
				if (!currentp) break;
			}
			tf->currentp = currentp;

			tf->csr_px = tf->xUL;
			for (i = 0; i < tf->current_col; i++) {
				tf->csr_px += TFCWidth(tf->currentp->linep+i);
				if (TFCFlags(tf->currentp->linep + i) &
				    MASK_WRAP) {
					tf->csr_px = tf->xUL;
				}
			}

			rangeOperation(tf, 
				tf->highLiteFrom_cx, tf->highLiteFrom_cy,
				tf->highLiteTo_cx, tf->highLiteTo_cy,
				0/*draw*/, -999/*doesn`t matter?*/,
				RANGEMODE_DELETE);
			tf->highLiteFrom_cx = 
			tf->highLiteFrom_cy =
			tf->highLiteTo_cx = 
			tf->highLiteTo_cy = -1;
			replaceNodeLine(theEditLN, tf->currentp, 0, NULL);
			refreshMode = SCREEN;
		} else {
			--(tf->current_col);
			tf->current_col_sticky = tf->current_col;
			--(theEditLN->length);
			tf->csr_px -= TFCWidth(theEditLN->linep + 
					tf->current_col);
/*
       printf("kbf_delete_backward_char(): px=%d c=[%d]'%c'\n",
		       tf->csr_px,
		       tf->current_col,
		       TFCChar(theEditLN->linep + tf->current_col));
*/
			scrollLineBackward(tf, 1, tf->current_col);
			TFCShiftStr(theEditLN->linep, tf->current_col+1, -1);
		}
	}
	tf->bufferUsed = 1;
	return 1;
}

#define VERBOSE_SCROLLINEBACKWARD___
int scrollLineBackward(tf, delta, col)
	TFStruct *tf;
	int delta;
	int col;
{
	int sy;
	int pwidth = 0;
	TFChar *tfcp, *linep;
	int px, py, i, hasMore = 1;
	int exitAfterPatch = 0;
	int patchstart_col, patchstart_px;
	TFLineNode *currentp = theEditLN;
	int orig_breakc;

	linep = currentp->linep;

	px = tf->csr_px;
	py = tf->csr_py;
	sy = py + currentp->maxFontHeight - currentp->maxFontDescent;
	orig_breakc = currentp->breakc;
	currentp->breakc = 0;

	for (i = 0; i < col; i++)
		if (TFCFlags(linep + i) & MASK_WRAP) currentp->breakc++;

	for (i = 0; i < delta; i++) {
		pwidth += TFCWidth(linep + col);
		if (TFCFlags(linep + col) & MASK_WRAP) {
			break;
		}
		++col;
	}

	while (hasMore) {

		/* shift line backward
		 */
		if (pwidth == 0) {
		printf("** exiting b/c pwidth==0\n");
		  return 1; /* hmm... */
		}

#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf("** displace: pwidth=%d px=%d \n", pwidth, px);
#endif

		if (TFWINDOW) {
			XCopyArea(display, TFWINDOW, TFWINDOW, gc_copy0, 
				px + pwidth, py, tf->width - px - pwidth + 1,
				currentp->maxFontHeight, px, py);
		}

		/* read till the wrap line break 		
		 */
		for (;;) {
			tfcp = linep + col;
			if (!TFCChar(tfcp)) {
				int diff_breakc;

				currentp->breakc++;
				diff_breakc = orig_breakc - currentp->breakc;

				if (TFWINDOW) {
#ifdef VERBOSE_SCROLLINEBACKWARD 
fprintf(stdout, "tfed: 14 XClearArea w=%x %d %d %d %d\n", TFWINDOW,
						px + TFCWidth(tfcp), py, 
						tf->xLR - px,
						currentp->maxFontHeight);
#endif

					XClearArea(display, TFWINDOW,
						px + TFCWidth(tfcp), py, 
						tf->xLR - px,
						currentp->maxFontHeight, False);
				}

#ifdef VERBOSE_SCROLLINEBACKWARD 
				printf("### orig_breakc=%d\n", orig_breakc);
				printf("### breaks=%d\n", currentp->breakc);
				printf("### diff_breakc=%d\n", diff_breakc);
				printf("### current_row=%d\n",tf->current_row);
#endif

				if (diff_breakc > 0) {
					int save_breakc = currentp->breakc;

					tf->currentp->breakc = 
						currentp->breakc;
					tf->currentp->maxFontHeight = 
						currentp->maxFontHeight;

					currentp->breakc = orig_breakc;
					scrollUpLowerPart(tf,
						tf->current_row - 
							tf->screen_row_offset,
						currentp->maxFontHeight);
					currentp->breakc = save_breakc;

				}
				goto drawEdges;
			}
/*			printf("... px=%d c=[%d]'%c' w=%d\n",
				px, col, TFCChar(tfcp), TFCWidth(tfcp));
*/
			px += TFCWidth(tfcp);
			if (TFCFlags(tfcp) & MASK_WRAP) {
				TFCFlags(tfcp) &= ~MASK_WRAP;
				break;
			}
			col++;
		}

		/* then start characters-fitting into the remainding space
		 * later, deal with word-fitting.
		 */
		patchstart_col = ++col;
		patchstart_px = px;
		pwidth = tf->xLR - px;

#ifdef VERBOSE_SCROLLINEBACKWARD 
printf("####### px=%d\n", px);
#endif
		/* start from after the old marker, to the edge of the window
		 */
		for (;;) {
			tfcp = linep + col;
#ifdef VERBOSE_SCROLLINEBACKWARD 
			printf("### px=%d c=[%d]'%c' w=%d px+w=%d\n", 
				px, col, TFCChar(tfcp), TFCWidth(tfcp),
				px + TFCWidth(tfcp));
#endif

			if (!TFCChar(tfcp)) break;
			if ((px + TFCWidth(tfcp)) > tf->xLR) {
				if (col > 0) {

#ifdef VERBOSE_SCROLLINEBACKWARD 
					printf("### new flag: c=[%d]'%c'\n", 
						col-1, TFCChar(tfcp-1));
#endif

					TFCFlags(tfcp - 1) |= MASK_WRAP;
				} else {
#ifdef VERBOSE_SCROLLINEBACKWARD 
					printf("###?? new flag: c=[%d]'%c'\n", 
						col, TFCChar(tfcp));
#endif

					TFCFlags(tfcp) |= MASK_WRAP;
				}
				currentp->breakc++;
				break;
			}
			px += TFCWidth(tfcp);
			pwidth -= TFCWidth(tfcp);
			col++;
		}
#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf("###### px=%d c=[%d]'%c' pwidth(remaining)=%d\n", 
			px, col, TFCChar(tfcp), pwidth);
#endif

		/* clear the void-to-the-edge-of-window
		 */
		if (TFWINDOW) {
#ifdef VERBOSE_SCROLLINEBACKWARD 
fprintf(stdout, "tfed: 15 XClearArea w=%x %d %d %d %d %d\n", TFWINDOW,
				patchstart_px, py, 
				tf->xLR - patchstart_px,
				pwidth,
				currentp->maxFontHeight);
#endif

			XClearArea(display, TFWINDOW,
				patchstart_px, py, 
				tf->xLR, currentp->maxFontHeight,
				False);
		}
#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf("new marker@ col=%d startcol=%d startpx\n", 
			col, patchstart_col, patchstart_px);
#endif
		/* patch the void-to-the-edge-of-window
		 */
		px = patchstart_px;
		for (i = patchstart_col; i < col; i++) {
			drawChar(tf, linep + i, px, sy);
#ifdef VERBOSE_SCROLLINEBACKWARD 
			printf("patching: [%d]'%c' px=%d sy=%d width=%d\n", 
				i, TFCChar(linep + i), px, sy,
				TFCWidth(linep + i));
#endif
			px += TFCWidth(linep + i);
		}
		pwidth = px - patchstart_px;

#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf(">>> pwidth =%d\n", pwidth);
#endif

		if (exitAfterPatch) {
#ifdef VERBOSE_SCROLLINEBACKWARD 
			printf("****** exitAfterPatch\n");
#endif
			return 1;
		}

		if (!hasMore) break;
		exitAfterPatch = !hasMore;

#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf("~~~~ exitAfterPatch=%d\n", exitAfterPatch);
		printf("**2 px=%d col=%d hasMore=%d\n", px, col, hasMore);
#endif

		if (exitAfterPatch) {
#ifdef VERBOSE_SCROLLINEBACKWARD 
			printf("****** exitAfterPatch\n");
#endif
			return 1;
		}

#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf("*** px=%d py=%d pwidth=%d\n", px, py, pwidth);
		printf("*** paramdx=%d px=%d\n", tf->xLR, px);
#endif

		px = tf->xUL;
		py += currentp->maxFontHeight;
		sy += currentp->maxFontHeight;

#ifdef VERBOSE_SCROLLINEBACKWARD 
		printf(">>>>>>>>>>> col=%d px=%d sy=%d pwidth=%d delta=%d\n",
			col, px, sy, pwidth, delta);
#endif

	}
drawEdges:
	{
		int x1 = tf->xUL-1;
		int y1 = tf->yUL-1;
		int x2 = tf->xLR+1;
		int y2 = tf->yLR+1;
		int w = tf->w;
		/* draws right and bottom edge of the box-- 
		 * not always necesary 
		 */
		XDrawLine(display, w, gc_bg_lighter, x2, y1+1, x2, y2);
		XDrawLine(display, w, gc_bg_lighter, x2-1, y1+2, x2-1, y2);

	      	/* bottom edge */
		XDrawLine(display, w, gc_bg_lighter, x1+1, y2, x2, y2);
		XDrawLine(display, w, gc_bg_lighter, x1+2, y2-1, x2, y2-1);
	}
	return 1;
}

