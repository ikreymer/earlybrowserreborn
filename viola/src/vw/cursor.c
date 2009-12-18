/*
 * cursor.c
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

#include <Xm/Xm.h>
#include <X11/cursorfont.h>
#include <stdio.h>

#include "vw.h"
#include "cursor.h"


Cursor busyCursor;


void initCursors(display)
    Display *display;
{
    busyCursor = XCreateFontCursor(display, XC_watch);
}


void busyCursorMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
/*    fprintf(stderr, "VW: busyCursor\n");*/
    setBusyCursor((DocViewInfo *) clientData);
}


void idleCursorMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
/*    fprintf(stderr, "VW: idleCursor\n");*/
    setIdleCursor((DocViewInfo *) clientData);
}

