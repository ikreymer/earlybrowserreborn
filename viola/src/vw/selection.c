 /*
  * selection.c
  *
  * Code for getting the selection and for handling selection requests.
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
#include <X11/Xatom.h>
#include <stdio.h>

#include "vw.h"
#include "selection.h"


/*
 * X Doesn't send a SelectionClear event when the new selectionOwner is
 * part of the same application as the old owner.  Sigh.
 *
 * This variable keeps track of which Spider window, if any, is the
 * current selection owner.
 */
DocViewInfo *selectionOwner = NULL;


void urlLoseSelection(widget, selection)
    Widget widget;
    Atom *selection;
{
    Pixel p;
    
    XtVaGetValues(widget, XmNbackground, &p, NULL);
    XtVaSetValues(widget, XmNborderColor, p, NULL);

    selectionOwner = NULL;
}


void urlFinishSelection(widget, selection, target)
    Widget widget;
    Atom *selection, *target;
{
    /*
     * No need to do anything.
     *
     * However, toolkit will attempt to XtFree() the value passed
     * by urlDoSelectionRequest() if this routine is not registered.
     */
}


Boolean urlDoSelectionRequest(widget, selection, target, type_return,
			      value_return, length_return, format_return)
    Widget widget;
    Atom *selection;
    Atom *target;
    Atom *type_return;
    XtPointer *value_return;
    unsigned long *length_return;
    int *format_return;
{
    extern DocViewInfo *selectionOwner;
    
    if (*target == XA_STRING) {
	if (selectionOwner) {
	    *value_return = selectionOwner->URL;
	    *length_return = strlen(selectionOwner->URL);
	    *type_return = XA_STRING;
	    *format_return = 8;
	}
	
	return(True);
    } else {
	/* Standard selections. */
	*value_return = NULL;
	*length_return = 0;
	*type_return = None;
	*format_return = 8;
	return(False);
    }
}


void urlToggleSelection(widget, dvi)
    Widget widget;
    DocViewInfo *dvi;
{
    if (selectionOwner == dvi)
	urlGiveUpSelection(widget);
    else
	urlGetSelection(widget, dvi);
}


void urlGetSelection(widget, dvi)
    Widget widget;
    DocViewInfo *dvi;
{
    Pixel p;
    
    XtVaGetValues(widget, XmNforeground, &p, NULL);
    XtVaSetValues(widget, XmNborderColor, p, NULL);

    XtOwnSelection(widget, XA_PRIMARY, CurrentTime,
		   urlDoSelectionRequest,
		   urlLoseSelection,
		   urlFinishSelection);

    selectionOwner = dvi;
}


void urlGiveUpSelection(widget)
    Widget widget;
{
    XtDisownSelection(widget, XA_PRIMARY, CurrentTime);
    selectionOwner = NULL;
}


