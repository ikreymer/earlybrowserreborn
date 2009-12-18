/*
 * hotlist.c
 *
 * Routines and data structures for handling document traversal hotlist.
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

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>

#include "vw.h"
#include "callbacks.h"
#include "hotlist.h"
#include "cursor.h"
#include "dialog.h"

#include "../viola/hotlist.h"
#include "../viola/utils.h"
#include "../viola/sys.h"
#include "../viola/hash.h"
#include "../viola/obj.h"
#include "../viola/packet.h"

Packet pk;

void setBusyCursor_history(dvi)
    DocViewInfo *dvi;
{
	XDefineCursor(XtDisplay((dvi)->hotlistListWidget),
		  XtWindow((dvi)->hotlistListWidget),
		  busyCursor);
	setBusyCursor(dvi);
}

void setIdleCursor_history(dvi)
    DocViewInfo *dvi;
{
	XUndefineCursor(XtDisplay((dvi)->hotlistListWidget),
		  XtWindow((dvi)->hotlistListWidget));
	setIdleCursor(dvi);
}

void printHotlistList(dvi)
    DocViewInfo *dvi;
{
    int i;
/*
    fprintf(stderr, "\nHotlist List (%d items, %d = current):\n",
	    dvi->nHotlistItems, dvi->currentHotlistItem);
    
    for (i=0; i<dvi->nHotlistItems; i++)
	fprintf(stderr, "%s\n", dvi->hotlistList[i]);
    fprintf(stderr, "\n");
*/
}


void hotlistPrev(dvi)
    DocViewInfo *dvi;
{
/*
	dvi->currentHotlistItem--;
	if (dvi->hotlistListWidget) {
	    XmListDeselectAllItems(dvi->hotlistListWidget);
	    XmListSelectPos(dvi->hotlistListWidget,
			    dvi->currentHotlistItem+1,
			    FALSE);
	}
*/
}


void hotlistNext(dvi)
    DocViewInfo *dvi;
{
/*
	dvi->currentHotlistItem++;
	if (dvi->hotlistListWidget) {
	    XmListDeselectAllItems(dvi->hotlistListWidget);
	    XmListSelectPos(dvi->hotlistListWidget,
			    dvi->currentHotlistItem+1,
			    FALSE);
	}
*/
}


void hotlistBackUp(dvi)
    DocViewInfo *dvi;
{
    int end, i;
/*
    if (dvi->currentHotlistItem == 0) {
	XBell(XtDisplay(dvi->shell), 50);
	return;
    }

    if (dvi->hotlistListWidget)
      XmListDeletePos(dvi->hotlistListWidget, dvi->nHotlistItems);

    dvi->nHotlistItems--;
    free(dvi->hotlistList[dvi->nHotlistItems]);

    if (dvi->currentHotlistItem > dvi->nHotlistItems - 1)
	dvi->currentHotlistItem = dvi->nHotlistItems - 1;

    if (dvi->hotlistListWidget) {
	XmListDeselectAllItems(dvi->hotlistListWidget);
	XmListSelectPos(dvi->hotlistListWidget,
			dvi->currentHotlistItem+1,
			FALSE);
    }
*/
    /*printHotlistList(dvi);*/
}


void hotlistAdd(dvi, newItem)
    DocViewInfo *dvi;
    char *newItem;
{
    int i, nitems;
    XmString itemXMS;

    nitems = theHotListCount;

    dvi->nHotlistItems++;
    dvi->currentHotlistItem = dvi->nHotlistItems-1;

    if (dvi->hotlistListWidget) {
	itemXMS = XmStringCreateSimple(newItem);
	XmListAddItem(dvi->hotlistListWidget, itemXMS, 0);
	XmStringFree(itemXMS);
	
	XmListDeselectAllItems(dvi->hotlistListWidget);
	XmListSelectPos(dvi->hotlistListWidget,
			dvi->currentHotlistItem+1,
			FALSE);
    }
}


void hotlistSelect(dvi, url)
    DocViewInfo *dvi;
    char *url;
{
    /*printHotlistList(dvi);*/
/*
    int i, nitems=dvi->nHotlistItems;

    for (i=nitems-1; i>=0; i--)
	if (!strcmp(url, dvi->hotlistList[i])) {
	    dvi->currentHotlistItem = i;
	    if (dvi->hotlistListWidget) {
		XmListDeselectAllItems(dvi->hotlistListWidget);
		XmListSelectPos(dvi->hotlistListWidget, i+1, FALSE);
	    }
	    break;
	}
*/
}

void setHotlistList(dvi, newList, numItems)
    DocViewInfo *dvi;
    char *newList[];
    int numItems;
{
#ifdef NONONO
    int i;
    XmStringTable listItems = NULL;

    if (!newList || !numItems)
	return;

    /*
     * Allocate the new items in the hotlist list.
     */
    freeHotlistList(dvi);
    dvi->hotlistSize = (numItems + HOTLIST_CHUNK);
    dvi->hotlistList = (char **) malloc(dvi->hotlistSize * sizeof(char *));
    for (i=0; i<numItems; i++)
	dvi->hotlistList[i] = makeString(newList[i]);
    dvi->nHotlistItems = numItems;

    /*
     * Set the items in the hotlist list widget.
     */
    if (dvi->hotlistListWidget) {
	listItems = (XmStringTable) malloc(numItems * sizeof(XmString));
	for (i=0; i<numItems; i++)
	    listItems[i] = XmStringCreateSimple(newList[i]);
	XtVaSetValues(dvi->hotlistListWidget,
		      XmNitemCount, numItems,
		      XmNitems, listItems,
		      NULL);
	free(listItems);
    }
#endif
}


void freeHotlistList(dvi)
    DocViewInfo *dvi;
{
    int i;
    
    dvi->hotlistSize = 0;
    dvi->nHotlistItems = 0;
}


Widget createHotlistDialog(dvi)
    DocViewInfo *dvi;
{
    Widget dlog, form, list, doneButton;
    Widget actionForm, addButton, deleteButton, gotoButton, editButton;
    Arg args[16];
    int n=0, i;
    XmString labelXMS;
    XmStringTable xmsList;

    XtSetArg(args[n], XmNborderWidth, 2);  n++;
    XtSetArg(args[n], XmNhorizontalSpacing, 4);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 4);  n++;
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;

    form = XmCreateFormDialog(dvi->shell, "Hotlist Manager", args, n);
    XtVaSetValues(form,
		  XmNhorizontalSpacing, 6,
		  XmNverticalSpacing, 6,
		  NULL);

    actionForm = XtVaCreateManagedWidget("actionForm",
					xmFormWidgetClass, form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNfractionBase, 4,
					NULL);

    addButton = XtVaCreateManagedWidget("Add Current",
				    xmPushButtonWidgetClass,  actionForm,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_POSITION,
				    XmNrightPosition, 1,
				    NULL);
    XtAddCallback(addButton, XmNactivateCallback,
		  addHotlistItem, (XtPointer) dvi);

    deleteButton = XtVaCreateManagedWidget("Delete",
				    xmPushButtonWidgetClass,  actionForm,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_POSITION,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget, addButton,
				    XmNrightPosition, 2,
				    NULL);
    XtAddCallback(deleteButton, XmNactivateCallback,
		  deleteHotlistItem, (XtPointer) dvi);

    gotoButton = XtVaCreateManagedWidget("Goto",
				    xmPushButtonWidgetClass,  actionForm,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNrightAttachment, XmATTACH_POSITION,
				    XmNleftWidget, deleteButton,
				    XmNrightPosition, 3,
				    NULL);
    XtAddCallback(gotoButton, XmNactivateCallback,
		  gotoHotlistItem, (XtPointer) dvi);

    editButton = XtVaCreateManagedWidget("Edit Label",
				    xmPushButtonWidgetClass,  actionForm,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNrightPosition, 4,
				    XmNleftWidget, gotoButton,
				    NULL);
    XtAddCallback(editButton, XmNactivateCallback,
		  editHotlistItem, (XtPointer) dvi);

    doneButton = XtVaCreateManagedWidget("Hide", xmPushButtonWidgetClass,
					 form,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_FORM,
					 XmNleftOffset, 4,
					 XmNrightOffset, 4,
					 XmNbottomOffset, 4,
					 NULL);
    XtAddCallback(doneButton, XmNactivateCallback,
		  hideHotlist, (XtPointer) dvi);

    n = 0;
    {
	HotListItem *hip;
	dvi->nHotlistItems = theHotListCount;
	xmsList = (XmStringTable) malloc(dvi->nHotlistItems * sizeof(XmString));
	for (i=0, hip = theHotList; hip; i++, hip = hip->next) {
	  xmsList[i] = XmStringCreateSimple(hip->comment);
	}
	XtSetArg(args[n], XmNitems, xmsList);  n++;
	XtSetArg(args[n], XmNitemCount, i);  n++;
    }
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNtopWidget, actionForm);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNbottomWidget, doneButton);  n++;
    XtSetArg(args[n], XmNtopOffset, 4);  n++;
    XtSetArg(args[n], XmNleftOffset, 4);  n++;
    XtSetArg(args[n], XmNrightOffset, 4);  n++;
    XtSetArg(args[n], XmNselectionPolicy, XmBROWSE_SELECT);  n++;
    XtSetArg(args[n], XmNheight, 300);  n++;
    XtSetArg(args[n], XmNwidth, 400);  n++;
    XtSetArg(args[n], XmNlistSizePolicy, XmCONSTANT);  n++;
    dvi->hotlistListWidget = list = XmCreateScrolledList(form, "hotlistList",
							 args, n);
    XtAddCallback(list, XmNbrowseSelectionCallback,
		  hotlistSelectCB, (XtPointer) dvi);
    XtAddCallback(list, XmNdefaultActionCallback,
		  hotlistSelectCB, (XtPointer) dvi);
    XtManageChild(list);

    XtManageChild(form);

    if (dvi->nHotlistItems) 
    	XmListSelectPos(dvi->hotlistListWidget, dvi->nHotlistItems+1, FALSE);

    return(XtParent(form));
}

void hotlistSelectCB(list, clientData, callData)
    Widget list;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *) callData;

    if (dvi->currentHotlistItem == cbs->item_position - 1) {
      if (dvi->violaDocViewObj) {
        setBusyCursor_history(dvi);
	sendMessage1N1int(dvi->violaDocViewObj,
		  "hotlistList_goto", dvi->currentHotlistItem);
        setIdleCursor_history(dvi);
      }
    } else {
      dvi->currentHotlistItem = cbs->item_position - 1;
    }
}

void addToHotlistCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    addToHotlist(dvi);
}

void addToHotlist(dvi)
    DocViewInfo *dvi;
{
    if (dvi->violaDocViewObj) {
        setBusyCursor_history(dvi);
	clearPacket(&pk);
	sendMessage1N1int_result(dvi->violaDocViewObj,
			"hotlistList_add", dvi->currentHotlistItem, &pk);
	hotlistAdd(dvi, PkInfo2Str(&pk));
	saveHotList();
        setIdleCursor_history(dvi);
    }
}

void showHotlistCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    if (!dvi->hotlistDlog)
	dvi->hotlistDlog = createHotlistDialog(dvi);
    else
	XtPopup(dvi->hotlistDlog, XtGrabNone);
}

void showHotlist(dvi)
    DocViewInfo *dvi;
{
/*
    if (!dvi->hotlistDlog)
	dvi->hotlistDlog = createHotlistDialog(dvi);
    else
	XtPopup(dvi->hotlistDlog, XtGrabNone);
*/
}

void hideHotlist(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    XtPopdown(dvi->hotlistDlog);
}

void addHotlistItem(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    addToHotlist(dvi);
}

void deleteHotlistItem(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    
    if (dvi->violaDocViewObj) {
	if (dvi->currentHotlistItem >= 0) {
          setBusyCursor_history(dvi);
	  XmListDeletePos(dvi->hotlistListWidget, dvi->currentHotlistItem+1);
	  sendMessage1N1int(dvi->violaDocViewObj,
			"hotlistList_delete", dvi->currentHotlistItem);
	  XmListDeselectPos(dvi->hotlistListWidget, dvi->currentHotlistItem+1);
	  deleteHotListItem(dvi->currentHotlistItem);
	  saveHotList();
	  dvi->currentHotlistItem = -1;/* to be safe*/
          setIdleCursor_history(dvi);
	}
    }
}

void gotoHotlistItem(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    if (dvi->violaDocViewObj) {
	if (dvi->currentHotlistItem == -1) {
		/* prompt: select an item first */
		return;
	}
        setBusyCursor_history(dvi);
	sendMessage1N1int(dvi->violaDocViewObj,
			"hotlistList_goto", dvi->currentHotlistItem);
        setIdleCursor_history(dvi);
    }
}

void editHotlistItemCB(lds)
	struct lentryDlogStruct *lds;
{
	XmString itemXMS;
	HotListItem *hip;

	itemXMS = XmStringCreateSimple(lds->data);
	hip = getNthHotListItem(lds->dvi->currentHotlistItem);
	if (!hip) {
		/* huh? no such item? internal error */
		return;
	}
	free(hip->comment); /* free old comment*/
	hip->comment = trimEdgeSpaces(lds->data);
	XmListReplaceItemsPos(lds->dvi->hotlistListWidget, &itemXMS, 1, 
				lds->dvi->currentHotlistItem+1);
	saveHotList();
        setIdleCursor_history(lds->dvi);
}

void editHotlistItem(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    char *cp;
    HotListItem *hip;

    if (dvi->violaDocViewObj) {
	hip = getNthHotListItem(dvi->currentHotlistItem);
	if (!hip) {
		/* huh? no such item? internal error */
		return;
	}
	openSimpleLineEntryDialog(dvi, "Edit Hot Link Label",
				hip->comment, editHotlistItemCB);
    }
}




