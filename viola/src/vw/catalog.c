/*
 * catalog.c
 *
 * Personal URL Link Manager
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
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vw.h"
#include "dialog.h"
#include "callbacks.h"
#include "selection.h"
#include "edit.h"
#include "menu.h"
#include "catalog.h"


/* ---- GLOBALS  ------------------------------------------------------- */

static Pixmap defaultFolderIcon = 0;
static short defaultFolderWidth = 0;
static short defaultFolderHeight = 0;

static Pixmap defaultLinkIcon = 0;
static short defaultLinkWidth = 0;
static short defaultLinkHeight = 0;


/* ---- MENUS  -------------------------------------------------------- */

static MenuItem catalogFileMenuItems[] = {
    { "Open", &xmPushButtonWidgetClass, 'O', "Ctrl<Key>O", "Ctrl-O",
	  NULL, (XtPointer) NULL,
	  "Open the selected folder or link.",
	  TRUE, (MenuItem *) NULL},
    { "Close", &xmPushButtonWidgetClass, 'Q', "Ctrl<Key>Q", "Ctrl-Q",
	  closeCatalogCB, (XtPointer) NULL,
	  "Close this catalog window.",
	  TRUE, (MenuItem *) NULL},
    NULL,
};


static MenuItem catalogEditMenuItems[] = {
    { "Copy", &xmPushButtonWidgetClass, 'C', "Ctrl<Key>C", "Ctrl-C",
	  genericCallback, "Copy", 
	  "Copy the selection to the clipboard.",
	  TRUE, (MenuItem *) NULL},
    { "Paste", &xmPushButtonWidgetClass, 'P', "Ctrl<Key>V", "Ctrl-V",
	  genericCallback, "Paste", 
	  "Paste the clipboard to the current selection.",
	  TRUE, (MenuItem *) NULL},
    NULL,
};


static MenuItem catalogCatalogMenuItems[] = {
    { "Clean Up Window", &xmPushButtonWidgetClass, 'W', "Ctrl<Key>W", "Ctrl-W",
	  genericCallback, "",
	  "Neatly organize the icons in the current folder.",
	  TRUE, (MenuItem *) NULL},
    { "Alphabetize Window", &xmPushButtonWidgetClass, 'A', "Ctrl<Key>A", "Ctrl-A",
	  genericCallback, "",
	  "Organize icons alphabetically.",
	  TRUE, (MenuItem *) NULL},
    { "Info", &xmPushButtonWidgetClass, 'I', "Ctrl<Key>I", "Ctrl-I",
	  genericCallback, "",
	  "View and edit the comments about the selected link or folder.",
	  TRUE, (MenuItem *) NULL},
    { "Modify", &xmPushButtonWidgetClass, 'M', "Ctrl<Key>M", "Ctrl-M",
	  genericCallback, "",
	  "Modify the selected link or folder.",
	  TRUE, (MenuItem *) NULL},
    NULL,
};


static MenuItem catalogHelpMenuItems[] = {
    { "Help on a Helpful Topic", &xmPushButtonWidgetClass, 'H', "Ctrl<Key>H", "Ctrl-H",
	  genericCallback, "Help on a Helpful Topic",
	  "Place holder for the help menu items.",
	  TRUE, (MenuItem *) NULL},
    NULL,
};


static Menu catalogMenus[] = {
    {"File", 'F', catalogFileMenuItems},
    {"Edit", 'E', catalogEditMenuItems},
    {"Catalog", 'C', catalogCatalogMenuItems},
    NULL,
};


/* ---- ROUTINES  ---------------------------------------------------- */

int initCatalog(shell)
    Widget shell;
{
    if (!defaultLinkIcon) {
	/*
	 * Get the icons and make them.
	 */

	/*
	 * Make the default Catalog GC's.
	 */
    }
}


Widget makeCatalogMenus(mainWin, helpLabel, catalog)
    Widget mainWin;
    Widget helpLabel;
    Catalog *catalog;
{
    return(buildMenus(catalogMenus,
		      catalogHelpMenuItems,
		      mainWin,
		      helpLabel,
		      (void *) catalog));
}


readCatalog(catalogFile)
    char *catalogFile;
{
}


writeCatalog(catalogFile)
    char *catalogFile;
{
}


void drawLink(canvas, link)
    Widget canvas;
    Link *link;
{
    XmFontList bogusFontList;

    if (!link)
	return;
    
    XCopyArea(XtDisplay(canvas), link->icon, XtWindow(canvas), link->gc,
	      0, 0, link->w, link->h, link->x, link->y);
    XmStringDraw(XtDisplay(canvas), 
		 XtWindow(canvas), 
		 bogusFontList,
		 link->nameXMS, 
		 link->gc,
		 link->nx, link->ny,
		 1000,
		 XmALIGNMENT_BEGINNING,
		 XmSTRING_DIRECTION_L_TO_R,
		 (XRectangle *) NULL);
}


void drawFolder(canvas, folder)
    Widget canvas;
    Folder *folder;
{
    XmFontList bogusFontList;

    if (!folder)
	return;
    
    XCopyArea(XtDisplay(canvas), folder->icon, XtWindow(canvas), folder->gc,
	      0, 0, folder->w, folder->h, folder->x, folder->y);
    XmStringDraw(XtDisplay(canvas), 
    		 XtWindow(canvas), 
		 bogusFontList,
		 folder->nameXMS, 
    		 folder->gc,
		 folder->nx, folder->ny,
		 1000,
		 XmALIGNMENT_BEGINNING,
		 XmSTRING_DIRECTION_L_TO_R,
		 (XRectangle *) NULL);
    XmStringDraw(XtDisplay(canvas), 
    		 XtWindow(canvas), 
		 bogusFontList,
		 folder->nItemsXMS, 
    		 folder->gc,
		 folder->numx, folder->numy,
		 1000,
		 XmALIGNMENT_BEGINNING,
		 XmSTRING_DIRECTION_L_TO_R,
		 (XRectangle *) NULL);
}


#define drawItem(canvas, item)				\
    switch ((item)->type) {				\
    case FOLDER:					\
	drawFolder((canvas), (Folder *) (item));	\
	break;						\
    case LINK:						\
	drawLink((canvas), (Link *) (item));		\
	break;						\
    }


void drawCatalog(catalog)
    Catalog *catalog;
{
    int i;
    int nItems = catalog->currentFolder->nItems;
    Display *theDisplay = XtDisplay(catalog->canvas);
    Window theWindow = XtWindow(catalog->canvas);

    for (i=0; i<nItems; i++)
	drawItem(catalog->canvas, catalog->currentFolder->items[i]);
}


void catalogButtonDownEH(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    /*
     * If in an object:
     *    If shift-click:
     *       If object is selected,
     *          Unselect object.
     *       Else
     *          Select object.
     *    Else
     *       Unselect other objects.
     *       Select object.
     *    Set object drag mode.
     *    Start double click timer.
     * Else,
     *    Set multi select mode.
     *    Draw selection rectangle.
     */
}


void catalogButtonUpEH(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    /*
     * If in drag or delta-drag mode:
     *    - Clear mode.
     *    - If double click timer hasn't finished
     *        and this is the second click:
     *        - Open selected object. (multiple selections?)
     *    - Else (no double click):
     *        - Move selected objects to new positions.
     * Else:
     *    - Undraw selection rectangle.
     *    - Select all objects in selection rectangle.
     */
}


void catalogButtonMotionEH(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    /*
     * If in drag mode:
     *    If mouse has moved beyond delta of original click position
     *      - Set deltaDrag mode.
     * Else if delta-drag mode:
     *    - XOR draw copy of moving objects at old position.
     *    - XOR draw copy of moving objects at new position.
     * Else (multi select mode):
     *    - XOR draw old selection rectangle.
     *    - XOR draw new selection rectangle.
     *    - Store new selection rectangle.
     */
}


void catalogExposureEH(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
    int i, nitems;
    Catalog *catalog = (Catalog *) clientData;

    if (!catalog->currentFolder)
	return;
    
    nitems = catalog->currentFolder->nItems;
    for (i=0; i<nitems; i++)
	drawItem(catalog->canvas, catalog->currentFolder->items[i]);
}


void cleanupFolder(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
}


void alphabetizeFolder(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
}


void catalogSelectItem(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
}

    
void catalogDrag(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
}


void catalogSelectMultipleItems(widget, clientData, event, cont)
    Widget widget;
    XtPointer clientData;
    XEvent *event;
    Boolean *cont;
{
}


void catalogFindOpenLocation(folder, x, y, w, h)
    Folder *folder;
    int *x, *y;
    int w, h;
{
    *x = 50;
    *y = 50;
}

void createFolder(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    Catalog *catalog = (Catalog *) clientData;
    Folder *newfolder;
    XmFontList bogusList;
    int width, height;

    if (!catalog->currentFolder)
	return;

    newfolder = (Folder *) calloc(1, sizeof(Folder));
    newfolder->type = FOLDER;
    newfolder->state = SELECTED;
    newfolder->icon = defaultFolderIcon;
    newfolder->iconFile = NULL;
    newfolder->w = defaultFolderWidth;
    newfolder->h = defaultFolderHeight;
    catalogFindOpenLocation(catalog->currentFolder,
			    &newfolder->x, &newfolder->y,
			    newfolder->w, newfolder->h);
    newfolder->items = (Item **) calloc(ITEM_ALLOC_CHUNK,
					sizeof(CatalogItem *));
    newfolder->nItems = 0;
    newfolder->allocedItems = ITEM_ALLOC_CHUNK;

    newfolder->nameXMS = XmStringCreateLtoR("New Folder", "folderFont");
    width = XmStringWidth(bogusList, newfolder->nameXMS);
    height = XmStringHeight(bogusList, newfolder->nameXMS);
    newfolder->ny = newfolder->y + 1.2*newfolder->h + height;
    newfolder->nx = newfolder->w/2 - width/2;

    newfolder->nItemsXMS = XmStringCreateLtoR("0", "folderFont");
    width = XmStringWidth(bogusList, newfolder->nItemsXMS);
    height = XmStringHeight(bogusList, newfolder->nItemsXMS);
    newfolder->numx = newfolder->w/2 - width/2;
    newfolder->numy = newfolder->h/2 - height/2;

    catalog->currentFolder->items[catalog->currentFolder->nItems] =
	(Item *) newfolder;

    if (catalog->visible)
	drawFolder(catalog->canvas, newfolder);
}


void deleteItemCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    /* Look for all selected items and attempt to delete them. */
}


#define forceDeleteItem(item, catalog) {		\
    if (item) { 					\
	if ((item)->type == LINK)			\
	    forceDeleteLink((item), (catalog));		\
	else						\
	    forceDeleteFolder((item), (catalog));	\
    }							\
}
			      

void forceDeleteLink(link, catalog)
    Link *link;
    Catalog *catalog;
{
    if (!link)
	return;

    if (link->name) free(link->name);
    if (link->nameXMS) XmStringFree(link->nameXMS);
    if (link->commentURL) free(link->commentURL);
    if (link->url) free(link->url);
    if (link->comment) free(link->comment);

    /* folder and folderName are pointers to fields in pointer */
}


void forceDeleteFolder(folder, catalog)
    Folder *folder;
    Catalog *catalog;
{
    if (folder) {
	int i;
	int nItems = folder->nItems;
	
	for (i=0; i<nItems; i++)
	    forceDeleteItem(folder->items[i], catalog);
	
	if (folder->items) free(folder->items);
	if (folder->iconFile) free(folder->iconFile);
	if (folder->commentURL) free(folder->commentURL);
	if (folder->name) free(folder->name);
	if (folder->nameXMS) XmStringFree(folder->nameXMS);
	if (folder->nItemsXMS) XmStringFree(folder->nItemsXMS);
	free(folder);
    }
}


void deleteFolder(folder, catalog)
    Folder *folder;
    Catalog *catalog;
{
    int nItems=0;
    
    if (folder->nItems) {
	char *answer, question[256];

	sprintf(question,
		"The folder %s still has %d items in it.\nAre you sure you want to delete it?",
		folder->name, folder->nItems);
	answer = questionDialog(catalog->dvi,
				question,
				"Cancel",
				"Delete Folder",
				"Cancel",
				NULL);
	if (answer[0] == 'C')
	    return;
    }

    forceDeleteFolder(folder, catalog);
}


void modifyFolder()
{
}


void openItemSameWindowCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    /* Look for all selected items and attempt to open them. */
}


void openItemNewWindowCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    /* Look for all selected items and attempt to open them. */
}


void openItemSameWindow(item, catalog)
    Item *item;
    Catalog *catalog;
{
    /* Open the given item in the current catalog shell. */
}


void openItemNewWindow(item, catalog)
    Item *item;
    Catalog *catalog;
{
    /* Open the given item in a new catalog shell. */
}


void openFolder(folder, catalog)
    Folder *folder;
    Catalog *catalog;
{
}

void openLink(link, catalog)
    Link *link;
    Catalog *catalog;
{
}


void createLink(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
}


void modifyLink(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    /*
     * Popup dialog for editing link parameters and fields.
     */
}


/*
 * Pops up a text editor with the given string data in it.
 */
void showCatalog(catalogFile, parentDVI)
    char *catalogFile;
    DocViewInfo *parentDVI;
{
    XmString xms;
    Catalog *catalog;
    Widget shell,
           mainWindow,
           form,
           helpLabel,
           titleFrame,
           folderNameLabel,
           folderNameText,
           catalogTitleLabel,
           catalogTitleText,
           catalogFileLabel,
           catalogFileText,
           frame,
           menubar,
           canvasFrame,
           canvas;

    catalog = (Catalog *) calloc(1, sizeof(Catalog));

    shell = XtVaAppCreateShell("Personal URL Catalog", "Personal URL Catalog",
			       applicationShellWidgetClass,
			       XtDisplay(parentDVI->shell),
			       XmNborderWidth, 2,
			       XmNiconPixmap, icon,
			       XmNshadowType, XmSHADOW_ETCHED_OUT,
			       XmNshadowThickness, 2,
			       NULL);

    initCatalog(shell);

    mainWindow = XmCreateMainWindow(shell, "mainWindow", NULL, 0);

    form =  XtVaCreateManagedWidget("form", xmFormWidgetClass, mainWindow,
				    XmNhorizontalSpacing, FORM_SPACING,
				    XmNverticalSpacing, FORM_SPACING,
				    NULL);

    xms = XmStringCreateLtoR("Personal URL Catalog", "helpFont");
    helpLabel = XtVaCreateManagedWidget("helpLabel", xmLabelWidgetClass, form,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNborderWidth, 1,
					XmNrecomputeSize, FALSE,
					XmNlabelString, xms,
					NULL);
    XmStringFree(xms);
    setHelp(helpLabel, helpLabel, "This is the message area.");

    titleFrame =  XtVaCreateManagedWidget("titleFrame",
					  xmFormWidgetClass,
					  form,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNshadowThickness, 1,
					  XmNshadowType, XmSHADOW_ETCHED_IN,
					  XmNhorizontalSpacing, 4,
					  XmNverticalSpacing, 4,
					  NULL);
    
    xms = XmStringCreateLtoR("Current Folder:", "boldFont");
    folderNameLabel = XtVaCreateManagedWidget("folderNameLabel",
					      xmLabelWidgetClass,
					      titleFrame,
					      XmNtopAttachment, XmATTACH_FORM,
					      XmNleftAttachment, XmATTACH_FORM,
					      XmNalignment, XmALIGNMENT_BEGINNING,
					      XmNrecomputeSize, FALSE,
					      XmNlabelString, xms,
					      NULL);
    XmStringFree(xms);
    
    xms = XmStringCreateLtoR("Foobar Folder", "plainFont");
    folderNameText = XtVaCreateManagedWidget("folderNameText",
					     xmLabelWidgetClass,
					     titleFrame,
					     XmNtopAttachment, XmATTACH_FORM,
					     XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					     XmNbottomWidget, folderNameLabel,
					     XmNbottomOffset, 0,
					     XmNleftAttachment, XmATTACH_WIDGET,
					     XmNleftWidget, folderNameLabel,
					     XmNrightAttachment, XmATTACH_FORM,
					     XmNalignment, XmALIGNMENT_BEGINNING,
					     XmNrecomputeSize, FALSE,
					     XmNlabelString, xms,
					     NULL);
    XmStringFree(xms);
    setHelp(folderNameLabel, helpLabel, "Name of the currently open folder.");
    setHelp(folderNameText, helpLabel, "Name of the currently open folder.");
    
    xms = XmStringCreateLtoR("Catalog Title", "boldFont");
    catalogTitleLabel = XtVaCreateManagedWidget("catalogTitleLabel",
						xmLabelWidgetClass,
						titleFrame,
						XmNtopAttachment, XmATTACH_WIDGET,
						XmNtopWidget, folderNameLabel,
						XmNleftAttachment, XmATTACH_FORM,
						XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
						XmNrightWidget, folderNameLabel,
						XmNrightOffset, 0,
						XmNalignment, XmALIGNMENT_BEGINNING,
						XmNrecomputeSize, FALSE,
						XmNlabelString, xms,
						NULL);
    XmStringFree(xms);
    
    xms = XmStringCreateLtoR("Foobar catalog of things and stuff.", "plainFont");
    catalogTitleText = XtVaCreateManagedWidget("catalogTitleText",
					       xmLabelWidgetClass,
					       titleFrame,
					       XmNtopAttachment, XmATTACH_WIDGET,
					       XmNtopWidget, folderNameLabel,
					       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					       XmNbottomWidget, catalogTitleLabel,
					       XmNbottomOffset, 0,
					       XmNleftAttachment, XmATTACH_WIDGET,
					       XmNleftWidget, catalogTitleLabel,
                                               XmNrightAttachment, XmATTACH_FORM,
                                               XmNalignment, XmALIGNMENT_BEGINNING,
                                               XmNrecomputeSize, FALSE,
                                               XmNlabelString, xms,
                                               NULL);
    XmStringFree(xms);
    setHelp(catalogTitleLabel, helpLabel, "Title of the URL Catalog being viewed.");
    setHelp(catalogTitleText, helpLabel, "Title of the URL Catalog being viewed.");

    xms = XmStringCreateLtoR("File:", "boldFont");
    catalogFileLabel = XtVaCreateManagedWidget("catalogFileLabel",
                                               xmLabelWidgetClass,
                                               titleFrame,
                                               XmNtopAttachment, XmATTACH_FORM,
					       XmNtopWidget, catalogTitleLabel,
					       XmNbottomAttachment, XmATTACH_FORM,
					       XmNleftAttachment, XmATTACH_FORM,
					       XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
					       XmNrightWidget, folderNameLabel,
					       XmNrightOffset, 0,
					       XmNrecomputeSize, FALSE,
					       XmNalignment, XmALIGNMENT_BEGINNING,
					       XmNlabelString, xms,
					       NULL);
    XmStringFree(xms);
    
    xms = XmStringCreateLtoR(catalogFile, "plainFont");
    catalogFileText = XtVaCreateManagedWidget("catalogFileText",
					      xmLabelWidgetClass,
					      titleFrame,
					      XmNtopAttachment, XmATTACH_WIDGET,
					      XmNtopWidget, catalogTitleLabel,
					      XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
					      XmNbottomWidget, catalogFileLabel,
					      XmNbottomOffset, 0,
					      XmNleftAttachment, XmATTACH_WIDGET,
					      XmNleftWidget, catalogFileLabel,
					      XmNrightAttachment, XmATTACH_FORM,
					      XmNalignment, XmALIGNMENT_BEGINNING,
					      XmNrecomputeSize, FALSE,
					      XmNlabelString, xms,
					      NULL);
    XmStringFree(xms);
    setHelp(catalogFileLabel, helpLabel, "Source file of current catalog.");
    setHelp(catalogFileText, helpLabel, "Source file of current catalog.");
    
    canvasFrame = XtVaCreateManagedWidget("canvasFrame",
					  xmScrolledWindowWidgetClass,
					  form,
					  XmNtopAttachment, XmATTACH_WIDGET,
					  XmNtopWidget, titleFrame,
					  XmNbottomAttachment, XmATTACH_WIDGET,
					  XmNbottomWidget, helpLabel,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNshadowThickness, 1,
					  XmNwidth, 400,
					  XmNheight, 250,
					  NULL);

    setHelp(canvasFrame, helpLabel,
            "Double click on a document or folder to open it.");
    canvas = XtVaCreateManagedWidget("canvas", widgetClass, canvasFrame,
                                     XmNwidth, 1,
                                     XmNheight, 1,
                                     XmNborderWidth, 0,
                                     NULL);
    XtAddEventHandler(canvas, ExposureMask, FALSE,
                      catalogExposureEH, (XtPointer) catalog);

    menubar = makeCatalogMenus(mainWindow, helpLabel, catalog);

    XmMainWindowSetAreas(mainWindow, menubar, (Widget) 0,
                         (Widget) 0, (Widget) 0, form);
    XtManageChild(mainWindow);

    catalog->shell = shell;
    catalog->canvas = canvas;
    catalog->helpLabel = helpLabel;
    catalog->dvi = parentDVI;

    XtPopup(shell, XtGrabNone);
}


void showCatalogCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    DocViewInfo *dvi = (DocViewInfo *) cd->shellInfo;

    /* Determine the catalog file to be opened. */
    
    showCatalog(DEFAULT_CATALOG_FILE, dvi);
}


void closeCatalogCB(widget, clientData, callData)
    Widget widget;
    XtPointer clientData, callData;
{
    ClientData *cd = (ClientData *) clientData;
    Catalog *catalog = (Catalog *) cd->shellInfo;

    /*
     * If reference count on the topFolder is 1, free the
     * folder heirarchy.
     */
    XtDestroyWidget(catalog->shell);
}
