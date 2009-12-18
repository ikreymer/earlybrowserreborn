/*
 * callbacks.c
 *
 * Callback routines for buttons and menus.
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
#include <sys/param.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/ScrollBar.h>
#include <Xm/TextF.h>

#include "vw.h"
#include "menu.h"
#include "dialog.h"
#include "history.h"
#include "hotlist.h"
#include "selection.h"
#include "callbacks.h"
#include "cursor.h"
#include "edit.h"
#include "../viola/ident.h"


extern Widget mainHelpWidget;
extern int activeHelpLock;


static MenuItem clonePageMenuItems[] = {
    { "Close", &xmPushButtonWidgetClass, 'C', "Ctrl<Key>C", "Ctrl-C",
	  closePageCB, NULL, 
	  "Close this window",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};

static Menu clonePageMainMenus[] = {
    {"File", 'F', clonePageMenuItems},
    NULL,
};


void titleButtonEH(w, clientData, evt, cont)
    Widget w;
    XtPointer clientData;
    XEvent *evt;
    Boolean *cont;
{
    XButtonReleasedEvent *buttonEvt = (XButtonEvent *) evt;

    if (buttonEvt->button == 1) {
	reloadDoc((DocViewInfo *) clientData);
    } else if (buttonEvt->button == 3) {
	DocViewInfo *dvi = (DocViewInfo *) clientData;
	urlToggleSelection(w, dvi);
    }
}


void editorTitleButtonEH(w, clientData, evt, cont)
    Widget w;
    XtPointer clientData;
    XEvent *evt;
    Boolean *cont;
{
    XButtonReleasedEvent *buttonEvt = (XButtonEvent *) evt;

    if (buttonEvt->button == 1) {
	/* Reload the source of the document into the editor. */;
    } else if (buttonEvt->button == 3) {
	DocViewInfo *dvi = (DocViewInfo *) clientData;
	urlToggleSelection(w, dvi);
    }
}


void navigateHomeEH(w, clientData, evt, cont)
    Widget w;
    XtPointer clientData;
    XEvent *evt;
    Boolean *cont;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    sendMessage1(dvi->violaDocViewObj, "home");
}


void navigateHome(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, "home");
}

void navigateBackUp(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, "back");
    historyBackUp(dvi);
}

void navigatePrev(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, "prev");
    historyPrev(dvi);
}

void navigateNext(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, "next");
    historyNext(dvi);
}

void changeFonts(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1N1str(dvi->violaDocViewObj, 
			    "changeFonts", (char*)cd->data);
}

void oneWordMessageCB(w, clientData, callData)
    Widget w;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    sendMessage1(dvi->violaDocViewObj, (char *) cd->data);
}

void clonePage(parentDocViewInfo)
     DocViewInfo *parentDocViewInfo;
{
    Widget shell, menu, menuBar, closeB, titleFrame, title, helpLabel, 
           form, titleForm, frame, canvasForm, scrollBar, violaCanvas;
    DocViewInfo *localDocViewInfo;
    ClientData *cd;
    Pixel bg;
    extern Pixmap icon;
    XmString xms, xms_file, xms_close;
    int i;

    localDocViewInfo = (DocViewInfo *) calloc(1, sizeof(DocViewInfo));
    if (!localDocViewInfo) {
	fprintf(stderr, "Out of memory.\n");
	return;
    }

    localDocViewInfo->shellName = makeString(parentDocViewInfo->URL);

    shell = XtVaAppCreateShell(localDocViewInfo->shellName
			         /*XXX assuming Xt doesn't free this !!!*/,
			       "Viola",
			       applicationShellWidgetClass,
			       XtDisplay(parentDocViewInfo->shell),
			       XmNborderWidth, 2,
			       XmNiconPixmap, icon,
			       NULL);

    form =  XtVaCreateManagedWidget("form",
				    xmFormWidgetClass, shell,
				    XmNhorizontalSpacing, FORM_SPACING,
				    XmNverticalSpacing, FORM_SPACING,
				    NULL);

    localDocViewInfo->topMostWidget = form;
/*
    menuBar = XmCreateMenuBar(form, "menuBar", NULL, 0);
    for (i=0; clonePageMainMenus[i].title != NULL; i++)
	buildPulldownMenu(menuBar, 
			  clonePageMainMenus[i].title,
			  clonePageMainMenus[i].mnemonic,
			  clonePageMainMenus[i].menuItems,
			  helpLabel,
			  localDocViewInfo);
    XtManageChild(menuBar);
*/
#ifdef UNUSED

    titleForm =  XtVaCreateManagedWidget("titleForm", xmFormWidgetClass, form,
					 XmNtopAttachment, XmATTACH_FORM,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 NULL);

    closeB = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, titleForm,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrecomputeSize, FALSE,
				     XmNhighlightThickness, 0,
				     XmNmarginHeight, 6,
				     XmNmarginWidth, 6,
				     NULL);
    XtAddCallback(closeB, XmNactivateCallback,
		  closePage, (XtPointer) localDocViewInfo);

    titleFrame =  XtVaCreateManagedWidget("titleFrame", xmFrameWidgetClass, titleForm,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_WIDGET,
					  XmNleftWidget, closeB,
					  XmNleftOffset, 4,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNmarginWidth, 2,
					  XmNmarginHeight, 2,
					  NULL);

    if (!strlen(parentDocViewInfo->docName))
	/* The document doesn't have an official <TITLE>. */
	xms = makeXMSTitle(parentDocViewInfo->URL, parentDocViewInfo->docName);
    else
	xms = makeXMSTitle(parentDocViewInfo->docName, parentDocViewInfo->URL);
    title = XtVaCreateManagedWidget("cloneTitle", xmLabelWidgetClass, titleFrame,
				    XmNlabelString, xms,
				    XmNmarginHeight, 4,
				    XmNmarginWidth, 4,
				    XmNborderWidth, 4,
				    XmNfontList, titleFontList,
				    NULL);
    XtVaGetValues(title, XmNbackground, &bg, NULL);
    XtVaSetValues(title, XmNborderColor, bg, NULL);
    XmStringFree(xms);

    /* URL selection mechanism. */
    XtAddEventHandler(title, ButtonReleaseMask, FALSE,
		      titleButtonEH, (XtPointer) localDocViewInfo);

#endif

    helpLabel = XtVaCreateManagedWidget("helpLabel", xmLabelWidgetClass, form,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrecomputeSize, FALSE,
					XmNborderWidth, 1,
					XmNalignment, XmALIGNMENT_BEGINNING,
					NULL);

    localDocViewInfo->msgLabel = helpLabel;
/*
    setHelp(closeB, helpLabel, "Close this particular document viewer.");
    setHelp(title, helpLabel, "Click left MB to reload document.  Click right MB to copy URL to clipboard.");
*/
   
    menuBar = XmCreateMenuBar(form, "menuBar", NULL, 0);
    for (i=0; clonePageMainMenus[i].title != NULL; i++)
	buildPulldownMenu(menuBar, 
			  clonePageMainMenus[i].title,
			  clonePageMainMenus[i].mnemonic,
			  clonePageMainMenus[i].menuItems,
			  helpLabel,
			  localDocViewInfo);
    XtManageChild(menuBar);

    frame = XtVaCreateManagedWidget("frame", xmFormWidgetClass, form,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, menuBar,
				    XmNbottomAttachment, XmATTACH_WIDGET,
				    XmNbottomWidget, helpLabel,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNshadowThickness, 4,
				    XmNshadowType, XmSHADOW_ETCHED_IN,
				    XmNhorizontalSpacing, 4,
				    XmNverticalSpacing, 4,
				    NULL);

    scrollBar = XtVaCreateManagedWidget("scrollBar", 
					xmScrollBarWidgetClass, frame,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNmaximum, SBAR_MAGNITUDE,
					NULL);
    
    violaCanvas = XtVaCreateManagedWidget("violaCanvas", 
					  xmPrimitiveWidgetClass, frame,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_WIDGET,
					  XmNrightWidget, scrollBar,
					  XmNwidth, 550, 
					  XmNheight, 500, /*650*/
					  NULL);
    setHelp(violaCanvas, helpLabel, "Clicking on links in this document will cause the main window to display the result of the link.");

    XmMainWindowSetAreas(form, menuBar, (Widget) NULL, (Widget) NULL, 
			(Widget) NULL, frame);

    localDocViewInfo->docName = makeString(parentDocViewInfo->docName);
    localDocViewInfo->URL = makeString(parentDocViewInfo->URL);
    localDocViewInfo->shell = shell;
    localDocViewInfo->cloneID = (long) shell;
    localDocViewInfo->title = title;
    localDocViewInfo->scrollBar = scrollBar;
    localDocViewInfo->canvas = violaCanvas;
    localDocViewInfo->quitDlog = (Widget) 0;
    localDocViewInfo->violaDocViewObj = NULL;
    localDocViewInfo->violaDocToolsObj = NULL;
    localDocViewInfo->violaDocViewName = NULL;
    localDocViewInfo->violaDocToolsName = NULL;
    localDocViewInfo->violaDocViewWindow = (Window) 0;
    localDocViewInfo->violaDocToolsWindow = (Window) 0;

    cd = (ClientData *) malloc(sizeof(ClientData));
    cd->data = (void *) parentDocViewInfo->violaDocViewObj;
    cd->shellInfo = (void *) localDocViewInfo;
    XtAddEventHandler(violaCanvas, StructureNotifyMask, FALSE,
		      pageCloneMapped, (XtPointer) cd);

    XtPopup(shell, XtGrabNone);
}

void clonePageCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
  clonePage((DocViewInfo*)((ClientData*)clientData)->shellInfo);
}

void clonePageMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    clonePage((DocViewInfo*)clientData);
}

void pageCloneMapped(canvas, clientData, event, continueDispatch)
    Widget canvas;
    XtPointer clientData;
    XEvent *event;
    Boolean *continueDispatch;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    char *parentObjName = (char *) cd->data;

    XtRemoveEventHandler(dvi->canvas, StructureNotifyMask, FALSE,
			 pageCloneMapped, clientData);
    free(cd);
    
    /*
     * Instruct Viola to make a clone of the document in parentDocViewInfo and
     * wait for a return message with the name of the cloned page object.
     */
    ViolaRegisterMessageHandler("showPageClone", showPageClone, (void *) dvi);
    sendMessage1N1int(parentObjName, "clonePage", (int) dvi->cloneID);
}


/*
 * MessageHandler "showPageClone"
 *
 * A clonePage shell has been created and a "clonePage" message was sent to
 * Viola.  Viola responds by creating a page object with the requested
 * document and then sending back the "showClone" message along with the name
 * of the page object.
 *
 * arg[0] = "showPageClone"
 * arg[1] = cloneID
 * arg[2] = top object name
 */
void showPageClone(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    Dimension width, height;
    VObj *obj;
    char errString[1024];

    if (argc < 3)
	return;

    if (dvi->cloneID != atol(arg[1]))
	return;

    obj = findObject(getIdent(arg[2]));
    if (!obj) {
	sprintf(errString,
		"Internal Error:  Sent \"clonePage\" but Viola returned\nbogus \"showPageClone\" object name: \"%s\"",
		arg[2]);
	modalErrorDialog(dvi, errString);
	closePageShell(dvi);
	return;
    }

    /* could probably eliminate this step */
    dvi->violaDocViewName = makeString(arg[2]);
    if (!dvi->violaDocViewName) {
	fprintf(stderr, "Out of memory!");
	return;
    }
    dvi->violaDocViewObj = obj;

    sendMessage1(dvi->violaDocViewObj, "render");
    dvi->violaDocViewWindow = GET_window(obj);
    XReparentWindow(XtDisplay(dvi->canvas),
		    dvi->violaDocViewWindow,
		    XtWindow(dvi->canvas),
		    0, 0);
    XtVaGetValues(dvi->canvas,
		  XmNwidth, &width,
		  XmNheight, &height,
		  NULL);
    XResizeWindow(XtDisplay(dvi->canvas),
		  (Window) dvi->violaDocViewWindow,
		  width, height);
    ViolaDeleteMessageHandler("showPageClone", showPageClone, (void *) dvi);

    /*
     * Set up the scrollBar and window resizing handler.
     */
    ViolaRegisterMessageHandler("sliderConfig", sliderConfig, (void *) dvi);
    ViolaDeleteMessageHandler("modalErrorDialog", modalErrorDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("errorDialog", errorDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("warningDialog", warningDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("infoDialog", infoDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("promptDialog", promptDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("questionDialog", questionDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("busyCursor", busyCursorMH, (void *) dvi);
    ViolaDeleteMessageHandler("idleCursor", idleCursorMH, (void *) dvi);
    ViolaDeleteMessageHandler("helpMessage", tmpMessageMH, (void *) dvi);

    XtAddCallback(dvi->scrollBar, XmNdragCallback,
		  scrollBarDrag, (XtPointer) dvi->violaDocViewObj);
    XtAddCallback(dvi->scrollBar, XmNvalueChangedCallback,
		  scrollBarValueChanged, (XtPointer) dvi->violaDocViewObj);
    XtAddEventHandler(dvi->canvas, StructureNotifyMask, FALSE,
		      resizeViola, (XtPointer) dvi->violaDocViewWindow);
}


void appCloneMapped(canvas, clientData, event, continueDispatch)
    Widget canvas;
    XtPointer clientData;
    XEvent *event;
    Boolean *continueDispatch;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    char *parentObjName = (char *) cd->data;

    XtRemoveEventHandler(dvi->canvas, StructureNotifyMask, FALSE,
			 appCloneMapped, (XtPointer) clientData);
    free(cd);
    
    /*
     * Instruct Viola to make a clone of the document in parentDocViewInfo and
     * wait for a return message with the name of the cloned page object.
     */
    ViolaRegisterMessageHandler("showAppClone", showAppClone, (void *) dvi);
    sendMessage1N1int(parentObjName, "cloneApp", (int) dvi->cloneID);
}

/*
 * MessageHandler "showAppClone"
 *
 * A cloneApp shell has been created and a "cloneApp" message was sent to
 * Viola.  Viola responds by creating a page object with the requested
 * document and then sending back the "showAppClone" message along with the name
 * of the App object.
 *
 * arg[0] = "showAppClone"
 * arg[1] = cloneID
 * arg[2] = top object name
 */
void showAppClone(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    Dimension width, height;
    VObj *obj;
    char errString[1024];

    if (argc < 3)
	return;

    if (dvi->cloneID != atol(arg[1]))
	return;

    obj = findObject(getIdent(arg[2]));
    if (!obj) {
	sprintf(errString,
		"Internal Error:  Sent \"cloneApp\" but Viola returned\nbogus \"showAppClone\" object name: \"%s\"",
		arg[2]);
	modalErrorDialog(dvi, errString);
	closeAppShell(dvi);
	return;
    }

    dvi->violaDocViewName = makeString(arg[2]);
    if (!dvi->violaDocViewName) {
	fprintf(stderr, "Out of memory!");
	return;
    }
    dvi->violaDocViewObj = obj;

    sendMessage1(dvi->violaDocViewObj, "render");
    dvi->violaDocViewWindow = GET_window(obj);
    XReparentWindow(XtDisplay(dvi->canvas),
		    dvi->violaDocViewWindow,
		    XtWindow(dvi->canvas),
		    0, 0);
    XtVaGetValues(dvi->canvas,
		  XmNwidth, &width,
		  XmNheight, &height,
		  NULL);
    XResizeWindow(XtDisplay(dvi->canvas),
		  (Window) dvi->violaDocViewWindow,
		  width, height);
    ViolaDeleteMessageHandler("showPageClone", showPageClone, (void *) dvi);

    /*
     * Set up the scrollBar and window resizing handler.
     */
    ViolaRegisterMessageHandler("newDocument", newDocument, (void *) dvi);
    ViolaRegisterMessageHandler("sliderConfig", sliderConfig, (void *) dvi);
    ViolaRegisterMessageHandler("searchOn", searchModeMH, (void *) dvi);
    ViolaRegisterMessageHandler("searchOff", searchModeMH, (void *) dvi);
    ViolaRegisterMessageHandler("saveAs", saveAsMH, (void *) dvi);
    ViolaRegisterMessageHandler("historyDocument", newDocument, (void *) dvi);
    ViolaRegisterMessageHandler("historyAdd", historyAddMH, (void *) dvi);
    ViolaRegisterMessageHandler("modalErrorDialog", modalErrorDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("errorDialog", errorDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("warningDialog", warningDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("infoDialog", infoDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("promptDialog", promptDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("questionDialog", questionDialogMH, (void *) dvi);
    ViolaRegisterMessageHandler("busyCursor", busyCursorMH, (void *) dvi);
    ViolaRegisterMessageHandler("idleCursor", idleCursorMH, (void *) dvi);
    ViolaRegisterMessageHandler("helpMessage", tmpMessageMH, (void *) dvi);

    XtAddCallback(dvi->scrollBar, XmNdragCallback,
		  scrollBarDrag, (XtPointer) dvi->violaDocViewObj);
    XtAddCallback(dvi->scrollBar, XmNvalueChangedCallback,
		  scrollBarValueChanged, (XtPointer) dvi->violaDocViewObj);
    XtAddEventHandler(dvi->canvas, StructureNotifyMask, FALSE,
		      resizeViola, (XtPointer) dvi->violaDocViewWindow);
}


void closePageShell(dvi)
    DocViewInfo *dvi;
{
    ViolaDeleteMessageHandler("sliderConfig", sliderConfig, (void *) dvi);
    
    /* The following is done just to be sure...
     *  it is possible that showPageClone wasn't given a proper
     *  object name and returned without removing itself in
     *  the hope that Viola might send another "showPageClone"
     *  message.  If Viola didn't resend, the showPageClone
     *  handler would still be installed and must be removed.
     */
    ViolaDeleteMessageHandler("showPage", showPageClone, (void *) dvi);
    ViolaDeleteMessageHandler("modalErrorDialog", modalErrorDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("errorDialog", errorDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("warningDialog", warningDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("infoDialog", infoDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("promptDialog", promptDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("questionDialog", questionDialogMH, (void *) dvi);
    ViolaDeleteMessageHandler("busyCursor", busyCursorMH, (void *) dvi);
    ViolaDeleteMessageHandler("idleCursor", idleCursorMH, (void *) dvi);
    ViolaDeleteMessageHandler("helpMessage", tmpMessageMH, (void *) dvi);

    /* Tell the Viola object to free itself. */
    if (dvi->violaDocViewObj)
       sendMessage1(dvi->violaDocViewObj, "freeSelf");

    XtDestroyWidget(dvi->shell);
    freeDocViewInfo(dvi);
}

void closePageCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    closePageShell((DocViewInfo*)((ClientData*)clientData)->shellInfo);
}

void cloneApp(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    Widget shell;
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    ClientData *cd;
    char nameBuf[256];
    extern Pixmap icon;
    extern char *appName;

    nshells++;
    sprintf(nameBuf, "%s%d", appName, nshells);
    shell = XtVaAppCreateShell(nameBuf, nameBuf,
			       applicationShellWidgetClass,
			       XtDisplay(button),
			       XmNborderWidth, 2,
			       XmNiconPixmap, icon,
			       NULL);
    dvi = makeBrowserInterface(shell, nameBuf, dvi, 0, NULL);

    cd = (ClientData *) malloc(sizeof(ClientData));
    cd->data = (void *) dvi->violaDocViewName;
    cd->shellInfo = (void *) dvi;
    XtAddEventHandler(dvi->canvas, StructureNotifyMask, FALSE,
		      appCloneMapped, (XtPointer) cd);
    
    XtPopup(shell, XtGrabNone);    
}


void closeThisShell(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    closeAppShell((DocViewInfo *) clientData);
}


void quitOk(widget, clientData, call)
    Widget widget;
    XtPointer clientData, call;
{
    VObj *obj;
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;

    if (dvi->violaDocViewObj) {
      deleteFromBox(&docViews,
		  (void *) &(dvi->shell),
		  compareDocViewShell,
		  freeDocViewInfo,
		  TRUE);
      nshells--;
      sendMessage1(dvi->violaDocViewObj, "quit");
    }
    exit(0);
}


void cancelCB(widget, client, call)
    Widget widget;
    XtPointer client, call;
{
    ClientData *cd = (ClientData *) client;
    Widget dlog = (Widget) cd->data;
    XtPopdown(XtParent(dlog));
}


void quitCallback(widget, client, call)
    Widget widget;
    XtPointer client, call;
{
    ClientData *clientData = (ClientData *) client;
    DocViewInfo *dvi = (DocViewInfo *) clientData->shellInfo;
    Widget dlog = (Widget) 0;
    Arg args[8];
    int n=0;
    XmString xms;
    Widget button;
	    

    if (!dvi->quitDlog) {
	if (dvi->shell == topWidget) {
	    xms = XmStringCreateSimple("Really Exit the Program?");

	    n = 0;
	    XtSetArg(args[n], XmNmessageString, xms); n++;
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON); n++;
	    XtSetArg(args[n], XmNshadowThickness, 4); n++;
/*	    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;*/
	    dlog = XmCreateQuestionDialog(topWidget,
					  "Confirm Quit",
					  args, n);
	    XmStringFree(xms);
	    
	    button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
	    XtUnmanageChild(button);
	    
	    button = XmMessageBoxGetChild(dlog, XmDIALOG_OK_BUTTON);
	    XtAddCallback(button, XmNactivateCallback, quitOk, clientData);
	} else {
	    XmString okLabel, cancelLabel, helpLabel;
	    
	    xms = XmStringCreateLtoR("Only Close this Particular Viewer or Exit the Entire Program?", "largeFont");
	    okLabel = XmStringCreateSimple("Close this Viewer");
	    cancelLabel = XmStringCreateSimple("Exit Program");
	    helpLabel = XmStringCreateSimple("Cancel");
	    
	    n = 0;
	    XtSetArg(args[n], XmNmessageString, xms); n++;
	    XtSetArg(args[n], XmNokLabelString, okLabel); n++;
	    XtSetArg(args[n], XmNcancelLabelString, cancelLabel); n++;
	    XtSetArg(args[n], XmNhelpLabelString, helpLabel); n++;
	    XtSetArg(args[n], XmNdefaultButtonType, XmDIALOG_HELP_BUTTON); n++;
	    XtSetArg(args[n], XmNshadowThickness, 4); n++;
	    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
	    XtSetArg(args[n], XmNautoUnmanage, TRUE); n++;
	    dlog = XmCreateQuestionDialog(dvi->shell,
					  "Confirm Quit",
					  args, n);
	    XmStringFree(xms);
	    XmStringFree(okLabel);
	    XmStringFree(cancelLabel);
	    XmStringFree(helpLabel);
	    
	    button = XmMessageBoxGetChild(dlog, XmDIALOG_OK_BUTTON);
	    XtAddCallback(button, XmNactivateCallback,
			  closeThisShell, dvi);
	    
	    button = XmMessageBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
	    XtAddCallback(button, XmNactivateCallback, quitOk, clientData);
	    
	    button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
	    ((ClientData *) client)->data = dlog;
	    XtAddCallback(button, XmNactivateCallback, cancelCB, clientData);
	}
	dvi->quitDlog = dlog;
    }

    XtManageChild(dvi->quitDlog);
    XtPopup(XtParent(dvi->quitDlog), XtGrabExclusive);
}


void okCallback(button, clientData, callData)
{
    Widget dlog = (Widget) clientData;

    /*XtUnmanageChild(dlog);*/
    XtDestroyWidget(XtParent(dlog));
}


void genericCallback(thing, clientData, callData)
    Widget thing;
    XtPointer callData, clientData;
{
    Widget dlog, button;
    DocViewInfo *dvi = ((ClientData *) clientData)->shellInfo;
    XmString message;
    char msg[256];
    Arg args[4];
    int n;
    
    sprintf(msg, "Selected the \"%s\" item.",
	    ((char *) (((ClientData *) clientData)->data)));
    message = XmStringCreateSimple(msg);
    
    n = 0;
    XtSetArg(args[n], XmNmessageString, message); n++;
    XtSetArg(args[n], XmNshadowThickness, 4); n++;
    XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
    dlog = XmCreateMessageDialog(dvi->shell, "Button Selection", args, n);
    
    button = XmMessageBoxGetChild(dlog, XmDIALOG_CANCEL_BUTTON);
    XtUnmanageChild(button);
    button = XmMessageBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(button);
    button = XmMessageBoxGetChild(dlog, XmDIALOG_OK_BUTTON);
    XtAddCallback(button, XmNactivateCallback, okCallback, dlog);
    
    XtManageChild(dlog);
    
    XmStringFree(message);
}


void openOk(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    XmFileSelectionBoxCallbackStruct *fscb = (XmFileSelectionBoxCallbackStruct *) callData;
    char *fileName;

    /* If this is going to take a while, set cursor and post thermometer. */
    
    XmStringGetLtoR(fscb->value, XmSTRING_DEFAULT_CHARSET, &fileName);
    XtPopdown(XtParent((Widget) cd->data));
    XmUpdateDisplay(mainHelpWidget);	
    sendMessage1N2str(dvi->violaDocViewObj, "show", fileName, "file:");
}


void openCancel(widget, clientD, callData)
    Widget widget;
    XtPointer clientD, callData;
{
    ClientData *clientData = (ClientData *) clientD;
    XtPopdown(XtParent((Widget) clientData->data));
}


void openDocument(widget, clientD, callData)
    Widget widget;
    XtPointer clientD, callData;
{
    static Widget dlog = (Widget) 0;
    static ClientData cd, *helpCD=NULL;
    ClientData *clientData = (ClientData *) clientD;
    DocViewInfo *dvi = (DocViewInfo *) clientData->shellInfo;
    Widget textField, button;
    int n;
    Arg args[8];
    XmString xms;
    XtCallbackRec okCallback[2], cancelCallback[2], helpCallback[2], filterCallback[2];
	    
    if (!dlog) {
	xms = XmStringCreateSimple("File Name:");
	n = 0;
	XtSetArg(args[n], XmNselectionLabelString, xms); n++;
	XtSetArg(args[n], XmNshadowThickness, 4); n++;
	XtSetArg(args[n], XmNshadowType, XmSHADOW_ETCHED_OUT); n++;
	dlog = XmCreateFileSelectionDialog(dvi->shell,
					   "Open Document",
					   args, n);
	XmStringFree(xms);

	textField = XmFileSelectionBoxGetChild(dlog, XmDIALOG_TEXT);
	XtVaSetValues(textField, XmNcolumns, 50, NULL);
	button = XmFileSelectionBoxGetChild(dlog, XmDIALOG_HELP_BUTTON);
	XtVaSetValues(button, XmNautoUnmanage, FALSE, NULL);

	cd.shellInfo = (void *) dvi;
	cd.data = (void *) dlog;
	okCallback[0].callback = (XtCallbackProc) openOk;
	okCallback[0].closure = (XtPointer) &cd;
	okCallback[1].callback = NULL;
	okCallback[1].closure = NULL;
	cancelCallback[0].callback = (XtCallbackProc) openCancel;
	cancelCallback[0].closure = (XtPointer) &cd;
	cancelCallback[1].callback = NULL;
	cancelCallback[1].closure = NULL;
	helpCallback[0].callback = (XtCallbackProc) oneWordMessageCB;
	if (!helpCD)
	    helpCD = (ClientData *) malloc(sizeof(ClientData));
	helpCD->shellInfo = dvi;
	helpCD->data = "about_browser";
	helpCallback[0].closure = (XtPointer) helpCD;
	helpCallback[1].callback = NULL;
	helpCallback[1].closure = NULL;
	XtVaSetValues(dlog,
		      XmNokCallback, (XtCallbackList) okCallback,
		      XmNcancelCallback, (XtCallbackList) cancelCallback,
		      XmNhelpCallback, (XtCallbackList) helpCallback,
		      NULL);
	XtManageChild(dlog);
    }

    /*XtPopup(XtParent(dlog), XtGrabExclusive);*/
    XtPopup(XtParent(dlog), XtGrabNone);
}



/*
 * Canvas ScrollBar callbacks.
 */
static int calledDrag;

void scrollBarConfig(sbar, thumbPercent, positionPercent)
    Widget sbar;
    int thumbPercent, positionPercent;
{
    int sliderSize, maximum;

    XtVaGetValues(sbar, XmNmaximum, &maximum, NULL);
    XtVaSetValues(sbar,
		  XmNsliderSize, (thumbPercent * maximum / SBAR_MAGNITUDE),
		  XmNvalue, (positionPercent * maximum / SBAR_MAGNITUDE),
		  NULL);
}


void scrollBarDrag(sbar, clientData, callData)
    Widget sbar;
    XtPointer clientData, callData;
{
    XmScrollBarCallbackStruct *sbData = (XmScrollBarCallbackStruct *) callData;
    VObj *violaObj = (VObj*) clientData;
    int sliderSize, maximum, percent;
    extern int calledDrag;

    /* Report position to Viola. */
    XtVaGetValues(sbar,
		  XmNsliderSize, &sliderSize,
		  XmNmaximum, &maximum,
		  NULL);
    percent = SBAR_MAGNITUDE*sbData->value/(maximum-sliderSize);

    if (violaObj) sendTokenMessageN1int(violaObj, STR_shownPositionV, percent);

    calledDrag++;
}


void scrollBarValueChanged(sbar, clientData, callData)
    Widget sbar;
    XtPointer clientData, callData;
{
    XmScrollBarCallbackStruct *sbData = (XmScrollBarCallbackStruct *) callData;
    char *violaObj = (char *) clientData;
    extern int calledDrag;

    if (calledDrag)
	calledDrag = 0;
    else {
	/* Report position to Viola. */
	int sliderSize, maximum, percent;

	if (!sbar) {
fprintf(stderr, 
"internal error occured in scrollBarValueChanged(sbar==NULL,,,)\n");

	  /* XXX it is a known bug that sbar may == 0. find the cause...

dbx out from obs@cs.Technion.AC.IL:

isaac/118:Bin=>dbx vw
Reading symbolic information...
Read 66779 symbols
(dbx) run
Running: vw 
signal SEGV (segmentation violation) in XtGetValues at 0x13a0e4
XtGetValues+4:  ld      [%i0 + 4], %i4
(dbx) where
XtGetValues() at 0x13a0e4
XtVaGetValues() at 0x1563c4
scrollBarValueChanged(sbar = (nil), clientData = 0x4f3130 "", callData =
0xeffff588 ""), line 962 in "callbacks.c"
XtCallCallbackList() at 0x12e39c
ScrollCallback() at 0xe1ff4
`ScrollBar`Select() at 0xe09f0
HandleActions() at 0x1532c4
HandleSimpleState() at 0x153894
_XtTranslateEvent() at 0x153e0c
DispatchEvent() at 0x136ffc
DecideToDispatch() at 0x13770c
XtDispatchEvent() at 0x1377dc
main(argc = 1, argv = 0xeffffa4c), line 435 in "vw.c"
(dbx) quit
*/
	  return;
	}

	XtVaGetValues(sbar,
		      XmNsliderSize, &sliderSize,
		      XmNmaximum, &maximum,
		      NULL);
	percent = SBAR_MAGNITUDE*sbData->value/(maximum-sliderSize);
	if (violaObj)
		sendTokenMessageN1int(violaObj, STR_shownPositionV, percent);
    }
}


/*
 * titleIcon
 */
void flipGlobe(clientData, intervalID)
    XtPointer clientData;
    XtIntervalId *intervalID;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    dvi->globeNum = (dvi->globeNum? 0: 1);
    XtVaSetValues(dvi->titleIcon, XmNlabelPixmap, globes[dvi->globeNum], NULL);
    dvi->intervalID = XtAppAddTimeOut(appCon, GLOBE_TIME, flipGlobe, clientData);
}


void doViolaEvent(widget, clientData, event, continueDispatch)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *continueDispatch;
{
    violaProcessEvent(event);
}


void doViolaIdle(clientData)
    XtPointer clientData;
{
    violaIdleEvent();
}


void resizeViola(widget, clientData, event, continueDispatch)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean continueDispatch;
{
    if (event->type == ConfigureNotify) {
	XConfigureEvent *xcep = (XConfigureEvent *) event;

	XResizeWindow(XtDisplay(widget), (Window) clientData,
		      xcep->width, xcep->height);
    }
}

void showHelpMessage(widget, message)
    Widget widget;
    char *message;
{
    XmString xms;

    /* there's bug elsewhere in this code, to cause this check to be necessary
     * in order to prevent lock up of Xwindows .......
     */
    if (!widget) return;
    if (activeHelpLock) return;

    /* Pei: this is a temporary patch, until this widget-dependent 
     * approach is redesigned.
     */
    if (mainHelpWidget == NULL) mainHelpWidget = widget;

    xms = XmStringCreateLtoR(message, "boldFont");
    XtVaSetValues(widget, XmNlabelString, xms, NULL);
    XmStringFree(xms);
}

/* Pei: needed for showing anything other than just what-GUIs-do-what,
 * like progress reporting, etc.
 */
void showHelpMessageInMainWindow(message)
    char *message;
{
    if (mainHelpWidget) {
      XmString xms;
      xms = XmStringCreateLtoR(message, "boldFont");
      XtVaSetValues(mainHelpWidget, XmNlabelString, xms, NULL);
      if (perishableActiveHelp) XmUpdateDisplay(mainHelpWidget);
      XmStringFree(xms);
      activeHelpLock = 1;
    }
}

void helpHandler(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    if (!widget) return;
    if (activeHelpLock == 0) {
      struct helpStruct *hdata = (struct helpStruct *) clientData;

      showHelpMessage(hdata->helpLabel, hdata->helpText);
    }
}


void blankHelpHandler(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    if (!widget) return;
    if (activeHelpLock == 0) {
      static XmString blankXMS = (XmString) 0;
      struct helpStruct *hdata = (struct helpStruct *) clientData;

      if (!blankXMS) blankXMS = XmStringCreateSimple("");
      XtVaSetValues(hdata->helpLabel, XmNlabelString, blankXMS, NULL);
    }
}

/*PYW*/
void blankHelpHandlerInMainWindow()
{
    static XmString blankXMS = (XmString) 0;

    if (mainHelpWidget) {
      if (!blankXMS) blankXMS = XmStringCreateSimple("");
      XtVaSetValues(mainHelpWidget, XmNlabelString, blankXMS, NULL);
      if (perishableActiveHelp) XmUpdateDisplay(mainHelpWidget);
    }
    activeHelpLock = 0;
}

void reloadDoc(dvi)
    DocViewInfo *dvi;
{
    if (dvi->violaDocViewObj)
	sendMessage1(dvi->violaDocViewObj, "reload");
}

void reloadDocCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;

    reloadDoc((DocViewInfo *) cd->shellInfo);
}

void oneMessageCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo*) (cd->shellInfo);

    if (dvi && cd->data)
	sendMessage1(dvi->violaDocViewObj, (char *) cd->data);
}

/* --- MessageHandlers -----------------------------------------------------*/

/*
 * newDocument
 *
 * arg[0] = "newDocument" or "historyDocument"
 * arg[1] = cloneID
 * arg[2] = document URL
 * arg[3] = optional document title
 *
 */
void newDocument(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    long cloneID;
    XmString xms;
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    if (argc < 3)
	return;
    
    if (dvi->cloneID != atol(arg[1]))
	return;

    dvi->URL = makeString(arg[2]);
#ifdef UNUSED
    if (argc>3 && arg[3]) {
	if (!strcmp(arg[2], arg[3])) {
	    /* Viola sets the doc's title to its URL when there is
	     * no official <TITLE> in the document. */
	    dvi->docName = "";
	    xms = makeXMSTitle(dvi->URL, dvi->docName);
	} else {
	    dvi->docName = makeString(arg[3]);
	    xms = makeXMSTitle(dvi->docName, dvi->URL);
	}
    } else {
	dvi->docName = "";
	xms = makeXMSTitle(dvi->URL, dvi->docName);
    }
    XtVaSetValues(dvi->title, XmNlabelString, xms, NULL);
    XmStringFree(xms);
#endif
    if (!strcmp("newDocument", arg[0])) {
	historyAdd(dvi, arg[2]);
    } else {
	historySelect(dvi, arg[2]);
    }
}


/*
 * sliderConfig
 *
 * arg[0] = "sliderConfig"
 * arg[1] = cloneID
 * arg[2] = Position of slider thumb in thousandths (1 - 1000).
 * arg[3] = Size of slider thumb in thousandths.
 *
 */
void sliderConfig(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    int sliderSize, sliderPos;
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    
    if (argc < 4)
	return;

    if (dvi->cloneID != atol(arg[1]))
	return;

    sliderPos = atoi(arg[2]);
    sliderSize = atoi(arg[3]);

    if (sliderSize < 0)
	sliderSize = -sliderSize;
    else if (sliderSize == 0)
	sliderSize = 1;
    if (sliderSize > SBAR_MAGNITUDE)
	sliderSize = SBAR_MAGNITUDE;
    if (sliderPos < 0)
	sliderPos = -sliderPos;
    if (sliderPos > SBAR_MAGNITUDE - sliderSize)
	sliderPos = SBAR_MAGNITUDE - sliderSize;

    XtVaSetValues(dvi->scrollBar,
		  XmNsliderSize, sliderSize,
		  XmNpageIncrement, sliderSize,
		  XmNvalue, sliderPos,
		  NULL);
}


void searchCallback(textField, clientData, callData)
    Widget textField;
    XtPointer clientData, callData;
{
    char *searchText;
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    
    searchText = XmTextFieldGetString(textField);
    
    if (!searchText)
	return;

    sendMessage1N1str(dvi->violaDocViewObj, "search", searchText);

    XmTextFieldSetString(textField, "");
    free(searchText);
}


void vwSaveAsCB(button, clientData, callData)
    Widget button;
    XtPointer clientData, callData;
{
    int length;
    char *fileName, cwd[MAXPATHLEN];
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = cd->shellInfo;

    getcwd(cwd, MAXPATHLEN);
    length = strlen(cwd);
    if (length < MAXPATHLEN) {
	cwd[length] = '/';
	cwd[length+1] = '\0';
    }
    fileName = promptDialog(dvi,
      "Saving the current document to a local file.\n\nEnter the full pathname of the new file:",
      cwd);

    if (fileName) {
	sendMessage1N1str(dvi->violaDocViewObj, "saveAs", fileName);
	free(fileName);
    }
}


/*
 * arg[0] = "saveAs"
 * arg[1] = cookie
 * arg[2] = temp data file name
 * arg[3] = filename
 */
void saveAsMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    char *data;

    if (argc != 4)
	return;

    if (dvi->cloneID != atol(arg[1]))
	return;

    data = readFile(arg[2], dvi);
    (void) writeFile(data, arg[3], dvi);
    unlink(arg[2]);
    free(data);
}


void printFile(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;
    
    printDialog(dvi, "the current document...");
}


/*
 * searchOn/searchOff
 *
 * arg[0] = "searchOn" or "searchOff"
 * arg[1] = cloneID
 *
 */
void searchModeMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    DocViewInfo *dvi = (DocViewInfo *) clientData;
    int isIndex;

    if (argc != 2) return;

    if (dvi->cloneID != atol(arg[1])) return;

    isIndex = !strcmp("searchOn", arg[0]);

    if (dvi->searchLabel)
      XtVaSetValues(dvi->searchLabel, XmNsensitive, isIndex, NULL);
    if (dvi->searchText)
      XtVaSetValues(dvi->searchText, XmNsensitive, isIndex, NULL);
}
    

