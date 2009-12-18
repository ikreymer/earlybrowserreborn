/*
 * cursor.h
 *
 * Routines to switch the mouse cursor.
 *
 */
/*
 * Copyright 1993 O'Reilly & Associates. All rights reserved.
 *
 * Permission to use, copy, and/or distribute for any purpose and
 * without fee is hereby granted, provided that both the above copyright
 * notice and this permission notice appear in all copies and derived works.
 * Fees for distribution or use of this software or derived works may only
 * be charged with express written permission of the copyright holder.
 * This software is provided ``as is'' without express or implied warranty.
 */

#ifndef _CURSOR_H_
#define _CURSOR_H_


extern Cursor busyCursor;


#define setBusyCursor(dvi) 			   \
    XDefineCursor(XtDisplay((dvi)->topMostWidget),   \
		  XtWindow((dvi)->topMostWidget),    \
		  busyCursor)

#define setIdleCursor(dvi)    			   \
    XUndefineCursor(XtDisplay((dvi)->topMostWidget), \
		    XtWindow((dvi)->topMostWidget))


void initCursors();
void busyCursorMH();
void idleCursorMH();


#endif _CURSOR_H_
