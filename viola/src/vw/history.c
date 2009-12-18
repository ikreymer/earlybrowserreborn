/*
 * history.c
 *
 * Routines and data structures for handling document traversal history.
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
#include "history.h"
#include "cursor.h"


void printHistoryList(dvi)
    DocViewInfo *dvi;
{
    int i;
    
    fprintf(stderr, "\nHistory List (%d items, %d = current):\n",
	    dvi->nHistoryItems, dvi->currentHistoryItem);
    
    for (i=0; i<dvi->nHistoryItems; i++)
	fprintf(stderr, "%s\n", dvi->historyList[i]);
    fprintf(stderr, "\n");
}


void historyPrev(dvi)
    DocViewInfo *dvi;
{
	dvi->currentHistoryItem--;
	if (dvi->historyListWidget) {
	    XmListDeselectAllItems(dvi->historyListWidget);
	    XmListSelectPos(dvi->historyListWidget,
			    dvi->currentHistoryItem+1,
			    FALSE);
	}
}


void historyNext(dvi)
    DocViewInfo *dvi;
{
	dvi->currentHistoryItem++;
	if (dvi->historyListWidget) {
	    XmListDeselectAllItems(dvi->historyListWidget);
	    XmListSelectPos(dvi->historyListWidget,
			    dvi->currentHistoryItem+1,
			    FALSE);
	}
}


void historyBackUp(dvi)
    DocViewInfo *dvi;
{
    int end, i;

    if (dvi->currentHistoryItem == 0) {
	XBell(XtDisplay(dvi->shell), 50);
	return;
    }

    if (dvi->historyListWidget)
      XmListDeletePos(dvi->historyListWidget, dvi->nHistoryItems);

    dvi->nHistoryItems--;
    free(dvi->historyList[dvi->nHistoryItems]);

    if (dvi->currentHistoryItem > dvi->nHistoryItems - 1)
	dvi->currentHistoryItem = dvi->nHistoryItems - 1;

    if (dvi->historyListWidget) {
	XmListDeselectAllItems(dvi->historyListWidget);
	XmListSelectPos(dvi->historyListWidget,
			dvi->currentHistoryItem+1,
			FALSE);
    }
    /*printHistoryList(dvi);*/
}


void historyAdd(dvi, newItem)
    DocViewInfo *dvi;
    char *newItem;
{
    int i, nitems;
    char **tmpList;
    XmString itemXMS;

    nitems = dvi->nHistoryItems;
    tmpList = dvi->historyList;

    if (dvi->nHistoryItems == dvi->historySize)
	growHistoryList(dvi);
    
    dvi->historyList[dvi->nHistoryItems] = makeString(newItem);
    dvi->nHistoryItems++;
    dvi->currentHistoryItem = dvi->nHistoryItems-1;

    if (dvi->historyListWidget) {
	itemXMS = XmStringCreateSimple(newItem);
	XmListAddItem(dvi->historyListWidget, itemXMS, 0);
	XmStringFree(itemXMS);
	
	XmListDeselectAllItems(dvi->historyListWidget);
	XmListSelectPos(dvi->historyListWidget,
			dvi->currentHistoryItem+1,
			FALSE);
    }
}


void historySelect(dvi, url)
    DocViewInfo *dvi;
    char *url;
{
    /*printHistoryList(dvi);*/
/*
    int i, nitems=dvi->nHistoryItems;

    for (i=nitems-1; i>=0; i--)
	if (!strcmp(url, dvi->historyList[i])) {
	    dvi->currentHistoryItem = i;
	    if (dvi->historyListWidget) {
		XmListDeselectAllItems(dvi->historyListWidget);
		XmListSelectPos(dvi->historyListWidget, i+1, FALSE);
	    }
	    break;
	}
*/
}

void setHistoryList(dvi, newList, numItems)
    DocViewInfo *dvi;
    char *newList[];
    int numItems;
{
    int i;
    XmStringTable listItems = NULL;

    if (!newList || !numItems)
	return;

    /*
     * Allocate the new items in the history list.
     */
    freeHistoryList(dvi);
    dvi->historySize = (numItems + HISTORY_CHUNK);
    dvi->historyList = (char **) malloc(dvi->historySize * sizeof(char *));
    for (i=0; i<numItems; i++)
	dvi->historyList[i] = makeString(newList[i]);
    dvi->nHistoryItems = numItems;

    /*
     * Set the items in the history list widget.
     */
    if (dvi->historyListWidget) {
	listItems = (XmStringTable) malloc(numItems * sizeof(XmString));
	for (i=0; i<numItems; i++)
	    listItems[i] = XmStringCreateSimple(newList[i]);
	XtVaSetValues(dvi->historyListWidget,
		      XmNitemCount, numItems,
		      XmNitems, listItems,
		      NULL);
	free(listItems);
    }
}


void freeHistoryList(dvi)
    DocViewInfo *dvi;
{
    int i;
    
    for (i=0; i<dvi->nHistoryItems; i++)
	free(dvi->historyList[i]);
    if (dvi->historyList)
	free(dvi->historyList);

    dvi->historyList = (char **) NULL;
    dvi->nHistoryItems = 0;
    dvi->historySize = 0;
}


void growHistoryList(dvi)
    DocViewInfo *dvi;
{
    int i;
    char **newList = (char **) malloc((dvi->historySize + HISTORY_CHUNK) * sizeof(char *));
    dvi->historySize += HISTORY_CHUNK;
    
    for (i=0; i<dvi->nHistoryItems; i++)
	newList[i] = dvi->historyList[i];

    free(dvi->historyList);
    dvi->historyList = newList;
}


Widget createHistoryDialog(dvi)
    DocViewInfo *dvi;
{
    Widget dlog, form, list, doneButton, label1, label2, labelForm;
    Arg args[16];
    int n=0, i;
    XmString labelXMS;
    XmStringTable xmsList;

    XtSetArg(args[n], XmNborderWidth, 2);  n++;
    XtSetArg(args[n], XmNhorizontalSpacing, 4);  n++;
    XtSetArg(args[n], XmNverticalSpacing, 4);  n++;
    XtSetArg(args[n], XmNautoUnmanage, FALSE);  n++;

    form = XmCreateFormDialog(dvi->shell, "History Manager", args, n);
    XtVaSetValues(form,
		  XmNhorizontalSpacing, 6,
		  XmNverticalSpacing, 6,
		  NULL);

    labelForm = XtVaCreateManagedWidget("labelForm",
					xmFormWidgetClass,
					form,
					XmNhorizontalSpacing, 4,
					XmNverticalSpacing, 4,
					XmNshadowThickness, 2,
					XmNshadowType, XmSHADOW_ETCHED_IN,
					XmNtopAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    labelXMS = XmStringCreateLtoR("History Manager", "largeFont");
    label1 = XtVaCreateManagedWidget("History Manager",
				    xmLabelWidgetClass,
				    labelForm,
				    XmNalignment, XmALIGNMENT_BEGINNING,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, labelXMS,
				    NULL);
    XmStringFree(labelXMS);

    labelXMS = XmStringCreateLtoR("Below is a list of documents you have previously viewed.\nClick on a document name to reload that document.",
				  "plainFont");
    label2 = XtVaCreateManagedWidget("historyInstructions",
				     xmLabelWidgetClass,
				     labelForm,
				     XmNalignment, XmALIGNMENT_BEGINNING,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, label1,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNlabelString, labelXMS,
				     NULL);
    XmStringFree(labelXMS);
				    

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
		  hideHistory, (XtPointer) dvi);

    n = 0;
    if (dvi->nHistoryItems) {
	xmsList = (XmStringTable) malloc(dvi->nHistoryItems * sizeof(XmString));
	for (i=0; i<dvi->nHistoryItems; i++)
	    xmsList[i] = XmStringCreateSimple(dvi->historyList[i]);
	XtSetArg(args[n], XmNitems, xmsList);  n++;
	XtSetArg(args[n], XmNitemCount, dvi->nHistoryItems);  n++;
    }
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNtopWidget, labelForm);  n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);  n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg(args[n], XmNbottomWidget, doneButton);  n++;
    XtSetArg(args[n], XmNtopOffset, 4);  n++;
    XtSetArg(args[n], XmNleftOffset, 4);  n++;
    XtSetArg(args[n], XmNrightOffset, 4);  n++;
    XtSetArg(args[n], XmNselectionPolicy, XmBROWSE_SELECT);  n++;
    XtSetArg(args[n], XmNheight, 100);  n++;
    XtSetArg(args[n], XmNwidth, 550);  n++;
    XtSetArg(args[n], XmNlistSizePolicy, XmCONSTANT);  n++;
    dvi->historyListWidget = list = XmCreateScrolledList(form, "historyList",
							 args, n);
    XtAddCallback(list, XmNsingleSelectionCallback,
		  historySelectCB, (XtPointer) dvi);
    XtAddCallback(list, XmNdefaultActionCallback,
		  historySelectCB, (XtPointer) dvi);
    XtManageChild(list);

    XtManageChild(form);

    if (dvi->nHistoryItems)
    	XmListSelectPos(dvi->historyListWidget, dvi->currentHistoryItem+1, FALSE);

    return(XtParent(form));
}


void historySelectCB(list, clientData, callData)
    Widget list;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    XmListCallbackStruct *cbs = (XmListCallbackStruct *) callData;

    dvi->currentHistoryItem = cbs->item_position - 1;

    if (dvi->violaDocViewObj) {
        setBusyCursor(dvi);
	sendMessage1N1str(dvi->violaDocViewObj,
				"showHistoryDoc",
				dvi->historyList[cbs->item_position - 1]);
	sendMessage1N1int(dvi->violaDocViewObj,
				"historyList_selectedItem",
				cbs->item_position - 1);
        setIdleCursor(dvi);
    }
}


void showHistoryCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    if (!dvi->historyDlog)
	dvi->historyDlog = createHistoryDialog(dvi);
    else
	XtPopup(dvi->historyDlog, XtGrabNone);
}


void showHistory(dvi)
    DocViewInfo *dvi;
{
    if (!dvi->historyDlog)
	dvi->historyDlog = createHistoryDialog(dvi);
    else
	XtPopup(dvi->historyDlog, XtGrabNone);
}


void hideHistory(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    
    XtPopdown(dvi->historyDlog);
}

void historyBackUpMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    historyBackUp(dvip);
}

void historyPrevMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    historyPrev(dvip);
}

void historyNextMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    historyNext(dvip);
}

void historyAddMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    
    if (argc < 3)
	return;

    if (dvip->cloneID != atol(arg[1]))
	return;

    historyAdd(dvip, arg[2]);
}


/*
 * arg[0] = "historyList"
 * arg[1] = cloneID
 * arg[2] - arg[argc-1] = list items
 */
void setHistoryListMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvip = (DocViewInfo *) clientData;
    
    if (argc < 3)
	return;

    if (dvip->cloneID != atol(arg[1]))
	return;

    setHistoryList(dvip, argc-2, (char **) &arg[2]);
}
