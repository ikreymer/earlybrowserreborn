 /*
  * vw.c
  *
  * vw is the Motif interface program for ViolaWWW
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
/*
 * PYW: 
 * * added progress reporter.
 * * added fonts menu.
 * * added toolbar.
 * * tweaked accelerators to not conflick with viola text editors.
 * * duplicated buttons into menu, and into toolbar.
 * * optimized messaging.
 * * simplified some codes-- fewer functions...
 * * added servers menu.
 */
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/ScrollBar.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/MessageB.h>
#include <stdio.h>
#include <stdlib.h>

#include "vw.h"
#include "menu.h"
#include "callbacks.h"
#include "history.h"
#include "hotlist.h"
#include "edit.h"
#include "dialog.h"
#include "../viola/msgHandler.h"
#include "selection.h"
#include "catalog.h"
#include "fonts.h"
#include "mail.h"
#include "cursor.h"
#include "../libXPM/xpm.h"

#include "WWW.xbm"
#include "globes.xpm"


/* --- Constants ----------------------------------------------------------- */

/* --- Globals ------------------------------------------------------------- */
XtAppContext appCon;
Widget topWidget;
Pixmap icon = (Pixmap) 0;
Pixmap globes[] = {
    (Pixmap) 0,
    (Pixmap) 0,
};
char *appName;
int nshells=0;
Box *docViews;

Widget mainHelpWidget = NULL;
int activeHelpLock = 0;

int toolBarHeight = TOOLBAR_HEIGHT;

/*
 * This is here until I figure out a way to get around the Motif1.2.3 bug
 * that prevents resource conversion of a multiple font fontList.  This
 * thing prevents users from using resources to set the font for the title
 * label.
 */
XmFontList titleFontList;


/* --- Prototypes ---------------------------------------------------------- */
Widget makeButtons();
void checkForDebugOutput();
DocViewInfo *makeBrowserInterface();
DocViewInfo *makeClonePageInterface();


static String fallback_resources[] = {
    /*
     * The following multiple font fontList doesn't work in Motif1.2.3
     *   (but it works in Motif1.2 though!).
     * Not sure how to fix the problem.  Could manually construct a font
     *   list, but then that would override any resources the user might
     *   have.
     */
    /*
    "*XmLabel*fontList: -*times*medium*-r-*--14-*=plainFont, \
                        -*times*bold*-r-*--14-*=boldFont, \
                        -*times*bold*-r-*--18-*=largeFont, \
                        -*times*bold*-r-*--24-*=titleFont, \
                        -*helv*medium*-r-*--14-*=urlFont, \
                        -*helv*medium*-r-*--12-*=smallFont",
    */
    "*XmLabel*fontList: -*times*medium*-r-*--14-*=plainFont",
    "*XmText*fontList: 8x13=textFont",
    "*XmList*fontList: *times*bold*-r-*-12-*=itemFont",
    "*XmTextField*fontList: *helv*bold*-r-*-12-*=textFont",
    "*XmPushButton*fontList: *times*bold*-r-*-14-*=buttonFont",
    "*XmPushButtonGadget*fontList: *times*bold*-r-*-14-*=menuFont",
    "*XmCascadeButton*fontList: *times*bold*-r-*-14-*=menuButtonFont",
    "*XmCascadeButtonGadget*fontList: *times*bold*-r-*-14-*=menuButtonFont",
    "*labelFontList: *times*medium*-r-*-14-*=plainFont",
    "*messageText*fontList:  *helv*bold*-r-*-12-*=textFont",
    "*XmScrollBar*width: 19",
    "*foreground: white",
    "*background: antiqueWhite4",
    "*borderColor: yellow",
    "*XmText.translations: #override\n\
                           None<Key>KDelete: delete-previous-character()",
    "*XmTextField.translations: #override\n\
                           None<Key>KDelete: delete-previous-character()",
    "*mailCommandText*fontList: -*courier*bold*-r-*--12-*=plainFont",
    "*mailAddressText*fontList: -*courier*bold*-r-*--12-*=plainFont",
    "*mailSubjectText*fontList: -*courier*bold*-r-*--12-*=plainFont",
    "*helpLabel*fontList: -*helv*bold*-r-*--12-*=plainFont",
    NULL,
};


/* --- Menus ------------------------------------------------------------ */
static MenuItem fileMenuItems[] = {
    { "Open File", &xmPushButtonWidgetClass, 'O', "Ctrl<Key>O", "Ctrl-O",
	  openDocument, "Open",
	  "Open a document by file browser.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Open URL", &xmPushButtonWidgetClass, 'U', "Ctrl<Key>U", "Ctrl-U",
	  simpleOpenURLDialog, "Open URL",
	  "Open a document by typing in URL.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Open URL in Selection Buffer", &xmPushButtonWidgetClass, NULL,NULL,NULL,
	  openURLInSelectionBuffer, "Open URL in Selection Buffer", 
	 "Follow the URL in the X selection/cut buffer.",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL,
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Reload", &xmPushButtonWidgetClass, 'R', "Ctrl<Key>R", "Ctrl-R",
	  oneMessageCB, "reload",
	  "Reload the current document.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Clone", &xmPushButtonWidgetClass, 'C', "Ctrl<Key>C", "Ctrl-C",
	  clonePageCB, "clone",
	  "Clone the current document.",
	  TRUE,
	  (MenuItem *) NULL},
    /*
    { "Print", &xmPushButtonWidgetClass, 'P', NULL, NULL,
	  printFile, NULL, 
	  "Print the current document on a local printer.",
	  FALSE,
	  (MenuItem *) NULL},
    */
    { "Show Source", &xmPushButtonWidgetClass, 'S', "Ctrl<Key>S", "Ctrl-S",
	  showSourceCB, "showSource",
	  "Show the document source.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Save As", &xmPushButtonWidgetClass,  NULL, NULL, NULL,
	/*'A', "Ctrl<Key>A", "Ctrl-A",*/
	  vwSaveAsCB, NULL, 
	  "Save the current document to a new local file.",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL,
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Quit", &xmPushButtonWidgetClass, 'Q', "Ctrl<Key>Q", "Ctrl-Q",
	  quitCallback, NULL, 
	  "Quit the entire application.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};


static MenuItem editMenuItems[] = {
    { "Copy", &xmPushButtonWidgetClass, 'C', "Ctrl<Key>C", "Ctrl-C",
	  genericCallback, "Copy", 
	  "Copy the selection to the clipboard.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Paste", &xmPushButtonWidgetClass, 'P', "Ctrl<Key>V", "Ctrl-V",
	  genericCallback, "Paste", 
	  "Paste the clipboard to the current selection.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};


static MenuItem navigationMenuItems[] = {
    { "Home", &xmPushButtonWidgetClass, NULL, NULL, NULL,
	/*'H', "Ctrl<Key>H", "Ctrl-H",*/
	  navigateHome, "Home", 
	  "Go to the HOME page document.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Back Up", &xmPushButtonWidgetClass,  NULL, NULL, NULL,
	/*'B', "Ctrl<Key>B", "Ctrl-B",*/
	  navigateBackUp, "Back Up", 
	  "Backup to the previous document, removing the current document from memory.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Previous", &xmPushButtonWidgetClass,  NULL, NULL, NULL,
	/*'P', "Ctrl<Key>P", "Ctrl-P",*/
	  navigatePrev, "Previous", 
	  "Go the the previous document but keep the current document in memory.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Next", &xmPushButtonWidgetClass,  NULL, NULL, NULL,
	/*'N', "Ctrl<Key>N", "Ctrl-N",*/
	  navigateNext, "Next", 
	 "Go to the next document in history list.",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL, 
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "History List", &xmPushButtonWidgetClass, 'L', "Ctrl<Key>L", "Ctrl-L",
	  showHistoryCB, "History List", 
	  "Show history of document link traversals.",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL, 
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Hot List", &xmPushButtonWidgetClass, 'H', "Ctrl<Key>H", "Ctrl-H",
	  showHotlistCB, "Hot List", 
	  "Show list of saved URL links.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Add to Hot List", &xmPushButtonWidgetClass, 0, NULL, NULL,
	  addToHotlistCB, "Add To Hot List", 
	  "Add current document to Hot List.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};


static MenuItem fontsMenuItems[] = {
    { "Helvetica small", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useHelveticaSmallFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "Helvetica medium", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useHelveticaMediumFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "Helvetica large", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useHelveticaLargeFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL, 
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Times small", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useTimesSmallFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "Times medium", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useTimesMediumFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "Times large", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	  changeFonts, "useTimesLargeFonts", 
	  "Pick a font",
	  TRUE,
	  (MenuItem *) NULL},
    { "", &xmSeparatorGadgetClass, (char) 0, NULL, NULL,
	  NULL, NULL, 
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Fixed", &xmPushButtonWidgetClass, (char) 0, NULL, NULL,
	/*'F', "Ctrl<Key>F",  "Ctrl-F",*/
	  changeFonts, "useAllFixedFonts", 
	  "A very simple font, in case other fonts don't work.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};

/*
MenuItem testMenuItems[] = {
    { "Catalog", &xmPushButtonWidgetClass, 'C', "Ctrl<Key>C",  "Ctrl-C",
	  showCatalogCB, "",
	  "Test of personal URL organizer tool.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};
*/

MenuItem helpMenuItems[] = {
    { "About ViolaWWW", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_infoOnViolaWWW", 
	  "Information on Viola WWW.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Help...", &xmPushButtonWidgetClass,
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_helpOnViolaWWW", 
	  "General help on Viola.",
	  TRUE,
	  (MenuItem *) NULL},
    { "Demos...", &xmPushButtonWidgetClass,
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_demoForViolaWWW", 
	  "Demonstration documents.",
	  TRUE,
	  (MenuItem *) NULL},
/*
    { "Info on browser version", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  'I', "Ctrl<Key>I",  "Ctrl-I",
	  oneWordMessageCB, "versionInfo", 
	  "Version information for this browser.",
	  TRUE,
	  (MenuItem *) NULL},
*/
    { "", &xmSeparatorGadgetClass, 
	  (char) 0, NULL, NULL,
	  NULL, NULL, 
	  NULL,
	  TRUE,
	  (MenuItem *) NULL},
    { "Send Email to Developers", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  showMailEditor, "", 
	  "Send mail to developers at viola@ora.com.",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};

MenuItem serversMenuItems[] = {
    { "GNN's Whole Internet Catalog", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_gnnWIC", 
	  "Link to O'Reilly & Associates Resource Listing.",
	  TRUE,
	  (MenuItem *) NULL},
    { "W3O's Web Servers Catalog", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_cern", 
	  "Link to WWW Organization's List of Servers.",
	  TRUE,
	  (MenuItem *) NULL},
    { "More Meta Centers", &xmPushButtonWidgetClass, 
	  '\0', NULL, NULL,
	  oneWordMessageCB, "get_moreServers",
	  "More servers...",
	  TRUE,
	  (MenuItem *) NULL},
    NULL,
};



static Menu mainMenus[] = {
    {"File", 'F', fileMenuItems},
    {"Navigation", 'N', navigationMenuItems},
    {"Fonts", 'o', fontsMenuItems},
    {"Guides", 'S', serversMenuItems},
    NULL,
};


main(argc, argv)
    int argc;
    char *argv[];
{
    int i, loadCmdLineDoc=0;
    DocViewInfo *dvip;
    Dimension width, height;
    XEvent event;
    FILE *output;
   
    topWidget = XtVaAppInitialize(&appCon, "Viola",
				  NULL, 0,	
				  &argc, argv,
				  fallback_resources,
				  XmNborderWidth, 2,
				  NULL);

    appName = makeString(argv[0]);
    checkForDebugOutput(argc, argv);
    initCursors(XtDisplay(topWidget));
    dvip = makeBrowserInterface(topWidget, appName, NULL, argc, argv);

    for (i=1; i<argc; i++) {
	if (argv[i][0] != '-') {
	    sendMessage1N1str(dvip->violaDocViewObj, "show", argv[i]);
	    loadCmdLineDoc = 1;
	    break;
	}
    }

    if (!loadCmdLineDoc) {
	char *wwwHome = getenv("WWW_HOME");

	if (wwwHome)
	    sendMessage1N1str(dvip->violaDocViewObj, "show", wwwHome);
	else
	    sendMessage1(dvip->violaDocViewObj, "home");
    }
	
    
    while (1) {
	XtAppNextEvent(appCon, &event);
	violaProcessEvent(&event);
	XtDispatchEvent(&event);
    }
}


DocViewInfo *makeBrowserInterface(shell, shellName, parentInfo, argc, argv)
    Widget shell;
    char *shellName;
    DocViewInfo *parentInfo;
    int argc;
    char *argv[];
{
    Widget mainWin;
    Widget menuBar;
    Widget form;
    Widget topForm;
    Widget titleIconFrame;
    Widget titleIcon;
    Widget titleFrame;
    Widget toolBarForm;
    Widget title;
    Widget fieldBox;
    Widget frame;
    Widget canvasForm;
    Widget scrollBar;
    Widget violaCanvas;
    Widget messageText;
    Widget searchLabel;
    Widget searchField;
    Widget buttonBox;
    Pixel bg;
    Dimension th, tw;
    XmString xms;
    DocViewInfo *docViewInfo;
    int status;
    VObj *obj;
    extern Window topWindow; /* super kludge to corse viola objects to 
			      * stick into the window */

    docViewInfo = (DocViewInfo *) calloc(1, sizeof(DocViewInfo));
    docViewInfo->shellName = makeString(shellName);
    docViewInfo->shell = shell;
    if (parentInfo) {
      docViewInfo->URL = makeString(parentInfo->URL);
      docViewInfo->docName = makeString(parentInfo->docName);
    } else {
      docViewInfo->docName = NULL;
      docViewInfo->URL = NULL;
    }

    mainWin = XtVaCreateWidget("mainWin", xmMainWindowWidgetClass, shell, NULL);
    docViewInfo->topMostWidget = mainWin;
    
    form = XtVaCreateManagedWidget("form",
				xmFormWidgetClass, mainWin,
				XmNhorizontalSpacing, FORM_SPACING,
				XmNverticalSpacing, FORM_SPACING,
				XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
				NULL);

    toolBarForm =  XtVaCreateManagedWidget("toolBarForm",
				xmFormWidgetClass, form,
				XmNtopAttachment, XmATTACH_FORM,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightAttachment, XmATTACH_FORM,
				XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
				XmNheight, toolBarHeight,
				NULL);
#ifdef UNUSED
    topForm =  XtVaCreateManagedWidget("topForm", xmFormWidgetClass, form,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
				       NULL);

    titleIconFrame = XtVaCreateManagedWidget("titleIconFrame", xmFrameWidgetClass, topForm,
					     XmNtopAttachment, XmATTACH_FORM,
					     XmNleftAttachment, XmATTACH_FORM,
					     XmNbottomAttachment, XmATTACH_FORM,
					     NULL);

    titleIcon = XtVaCreateManagedWidget("titleIcon", xmLabelWidgetClass, titleIconFrame,
					XmNlabelType, XmPIXMAP,
					XmNrecomputeSize, FALSE,
					XmNwidth, TITLE_ICON_WIDTH,
					XmNheight, TITLE_ICON_HEIGHT,
					XmNbackground, XBlackPixelOfScreen(XtScreen(shell)),
					NULL);
    docViewInfo->titleIcon = titleIcon;
    docViewInfo->intervalID = XtAppAddTimeOut(appCon,
					      GLOBE_TIME,
					      flipGlobe,
					      (XtPointer) docViewInfo);
    XtAddEventHandler(titleIcon, ButtonReleaseMask, FALSE,
		      navigateHomeEH, (XtPointer) docViewInfo);

    titleFrame =  XtVaCreateManagedWidget("titleFrame", xmFrameWidgetClass, topForm,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_WIDGET,
					  XmNleftWidget, titleIconFrame,
					  XmNleftOffset, 4,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNmarginWidth, 4,
					  XmNmarginHeight, 4,
					  NULL);


    titleFontList = loadFonts(XtDisplay(shell));
    xms = makeXMSTitle("Viola World Wide Web Browser", "Alpha Release");
    title = XtVaCreateManagedWidget("title", xmLabelWidgetClass, titleFrame,
				    XmNlabelString, xms,
				    XmNborderWidth, 4,
				    XmNrecomputeSize, FALSE,
				    XmNfontList, titleFontList,
				    NULL);
    XtVaGetValues(title, XmNbackground, &bg, NULL);
    XtVaSetValues(title, XmNborderColor, bg, NULL);
    XmStringFree(xms);
    XmFontListFree(titleFontList);


    /* URL selection mechanism. */
    XtAddEventHandler(title, ButtonReleaseMask, FALSE,
		      titleButtonEH, (XtPointer) docViewInfo);
    /*
    XtAddEventHandler(title, 0L, TRUE,
		      urlSelectionRequest, (XtPointer) docViewInfo);
    XtAddEventHandler(title, 0L, TRUE,
                      urlSelectionClear, (XtPointer) title);
    */

#endif UNUSED
    
    /* Dynamic help area. */
    messageText = XtVaCreateManagedWidget("messageText", 
					  xmLabelWidgetClass, form,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_FORM,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNalignment, XmALIGNMENT_BEGINNING,
					  XmNtraversalOn, FALSE,
/*					  XmNborderWidth, 1,*/
					  XmNrecomputeSize, FALSE,
					  NULL);
    docViewInfo->msgLabel = messageText;

    /* this is to make sure that mainHelpWidget is set to the help widget
     * widget in the main window, and not to clone page help widget -- which
     * can happen if the first help-hints happen in the clonePage...
     */
    if (mainHelpWidget == NULL) mainHelpWidget = messageText;

/*
    setHelp(titleIcon, messageText, "Click with the mouse to go to the HOME document.");
    setHelp(title, messageText, "Click left MB to reload document.  Click right MB to copy URL to clipboard.");
    setHelp(messageText, messageText, "Contextual help message area.");
*/
    
    docViewInfo->hotlistListWidget = NULL;
    docViewInfo->currentHotlistItem = -1;
    docViewInfo->hotlistSize = 0; /* size of the list allocated */

    /* Quick action buttons at bottom of interface. */
/*
    buttonBox = makeButtons(form, messageText, docViewInfo);
    XtVaSetValues(messageText,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, buttonBox,
		  NULL);
    XtVaSetValues(form,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, messageText,
		  NULL);
*/

    
    /* Search region.  Only becomes active when a document is searchable. */
/*
    searchLabel = XtVaCreateManagedWidget("Index: ",
				xmLabelWidgetClass, form,
				XmNleftAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget, messageText,
				XmNrecomputeSize, FALSE,
				XmNheight, SEARCH_HEIGHT,
				XmNsensitive, FALSE,
				NULL);
    docViewInfo->searchLabel = searchLabel;

    searchField = XtVaCreateManagedWidget("searchField",
				xmTextFieldWidgetClass, form,
				XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
				XmNtopOffset, 0,
				XmNtopWidget, searchLabel,
				XmNleftAttachment, XmATTACH_WIDGET,
				XmNleftWidget, searchLabel,
				XmNrightAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget, messageText,
				XmNsensitive, FALSE,
				NULL);
    setHelp(searchLabel, messageText, "Index entry field.");
    setHelp(searchField, messageText, "Index entry field.");

    XtAddCallback(searchField, XmNactivateCallback,
		  searchCallback, (XtPointer) docViewInfo);
    docViewInfo->searchText = searchField;
*/

    /* Main document view area ... where Viola displays document. */
    frame = XtVaCreateManagedWidget("frame",
					xmFrameWidgetClass, form,
					XmNtopAttachment, XmATTACH_WIDGET,
					XmNtopWidget, toolBarForm,
					XmNbottomAttachment, XmATTACH_WIDGET,
					XmNbottomWidget, messageText /*searchLabel*/,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNshadowThickness, 3,
					XmNnavigationType, XmNONE,
					NULL);

    canvasForm = XtVaCreateManagedWidget("canvasForm", 
					xmFormWidgetClass, frame,
					XmNhorizontalSpacing, 1,
					XmNverticalSpacing, 1,
					NULL);

    scrollBar = XtVaCreateManagedWidget("scrollBar", 
					xmScrollBarWidgetClass, canvasForm,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNmaximum, SBAR_MAGNITUDE,
					NULL);
    
    violaCanvas = XtVaCreateManagedWidget("violaCanvas", 
					xmPrimitiveWidgetClass, canvasForm,
					XmNtopAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_WIDGET,
					XmNrightWidget, scrollBar,
					XmNwidth, 600,
					XmNheight, 650,
					XmNtraversalOn, TRUE,
					NULL);

    XtAddEventHandler(violaCanvas, VIOLA_EVENT_MASK, TRUE, doViolaEvent,
			NULL);

    /* Menubar. */
    menuBar = buildMenus(mainMenus, helpMenuItems, mainWin, messageText, 
			docViewInfo);
    XmMainWindowSetAreas(mainWin, menuBar, (Widget) NULL, (Widget) NULL, 
			(Widget) NULL, form);

    XtManageChild(mainWin);

    XtRealizeWidget(shell);


    /* Set color of main scrollbar to be palattable. */
    if (XDefaultDepthOfScreen(XtScreen(shell)) == 1) {
	Pixel bg;
	XtVaGetValues(canvasForm, XmNbackground, &bg, NULL);
	/*if (bg == XBlackPixelOfScreen(XtScreen(shell)))*/
	XtVaSetValues(scrollBar, XmNtroughColor, bg, NULL);
    } else {
	Pixel bg;
	XtVaGetValues(frame, XmNtopShadowColor, &bg, NULL);
	XtVaSetValues(violaCanvas, XmNbackground, bg, NULL);
    }


    /* Make the icon. */
/*
    if (!icon) {
	Pixel fg, bg;
	XtVaGetValues(titleIcon, XmNforeground, &fg, XmNbackground, &bg, NULL);
	icon = XCreatePixmapFromBitmapData(XtDisplay(titleIcon),
					   XtWindow(titleIcon),
					   WWW_bits, WWW_width, WWW_height,
					   fg, bg,
					   XDefaultDepthOfScreen(XtScreen(titleIcon)));
    }
    XtVaSetValues(shell, XmNiconPixmap, icon, NULL);
*/

    /* Set the title icon to a globe or the app icon if necessary. */
/*
    if (!globes[0]) {
	int status;
	Pixmap mask;
	XpmAttributes xpma;

	status = XpmCreatePixmapFromData(XtDisplay(titleIcon),
					 XtWindow(titleIcon),
					 globe1,
					 &globes[0],
					 &mask,
					 NULL);
	if (status != XpmSuccess) {
	    fprintf(stderr, "Couldn't create globe1 pixmap!\n");
	    globes[0] = icon;
	}
	
	status = XpmCreatePixmapFromData(XtDisplay(titleIcon),
					 XtWindow(titleIcon),
					 globe2,
					 &globes[1],
					 &mask,
					 NULL);
	if (status != XpmSuccess) {
	    fprintf(stderr, "Couldn't create globe2 pixmap!\n");
	    globes[1] = icon;
	}
    }
    XtVaSetValues(titleIcon, XmNlabelPixmap, globes[0], NULL);
*/

    /* Finish filling the docViewInfo structure. */
    docViewInfo->scrollBar = scrollBar;
    docViewInfo->title = title;
    docViewInfo->docName = "";
    docViewInfo->canvas = violaCanvas;
    docViewInfo->historySize = 0;
    docViewInfo->nHistoryItems = 0;
    docViewInfo->currentHistoryItem = 0;
    docViewInfo->historyListWidget = (Widget) 0;
    
    
    /*
     * Start up Viola in the net interface.
     */
    if (shell == topWidget) {
	/*
	 * Building the first top level application shell.
	 */
	Dimension width, height;

	status = initViola(argc, argv,
			 VIOLA_OBJ_NAME,
			 XtDisplay(violaCanvas),
			 XtScreen(violaCanvas),
			 XtWindow(violaCanvas));
	if (status == 0) {
		exit(100);
	}

	docViewInfo->violaDocViewName = makeString("mvw");
	docViewInfo->violaDocToolsName = makeString("mvwTools");

	docViewInfo->violaDocViewObj = 
		findObject(getIdent(docViewInfo->violaDocViewName));
	docViewInfo->violaDocToolsObj = 
		findObject(getIdent(docViewInfo->violaDocToolsName));

	if (!(docViewInfo->violaDocViewObj && docViewInfo->violaDocToolsObj)) {
		exit(101);
	}
	docViewInfo->violaDocViewWindow = 
		GET_window(docViewInfo->violaDocViewObj);
	docViewInfo->violaDocToolsWindow =
		GET_window(docViewInfo->violaDocToolsObj);

	docViewInfo->cloneID = (long) 0;
	XtAppAddWorkProc(appCon,
			 (XtWorkProc) doViolaIdle,
			 (XtPointer) NULL);
	
	XtVaGetValues(violaCanvas,
		      XmNwidth, &width,
		      XmNheight, &height,
		      NULL);

	XResizeWindow(XtDisplay(violaCanvas), docViewInfo->violaDocViewWindow,
		      (unsigned int) width, (unsigned int) height);
	XMoveWindow(XtDisplay(violaCanvas), docViewInfo->violaDocViewWindow,
			-1, -1);
	XtAddEventHandler(violaCanvas, StructureNotifyMask, FALSE,
			resizeViola,
			(XtPointer)docViewInfo->violaDocViewWindow);

	/* Hook in the document title object
	 */
	XtVaGetValues(toolBarForm, XmNwidth, &width, XmNheight, &height, NULL);
	topWindow = XtWindow(toolBarForm);
	obj = docViewInfo->violaDocToolsObj;
/*
	sendMessage1N4int(obj, "config", -1, -1, width+1, height+1);
*/
	SET_x(obj, -1);
	SET_y(obj, -1);
	SET_width(obj, width+1);
	SET_height(obj, height+1);
	sendMessage1N1int(obj, "visible", 1);
	docViewInfo->violaDocToolsWindow = GET_window(obj);
	XResizeWindow(XtDisplay(toolBarForm), 
		      docViewInfo->violaDocToolsWindow,
		      width, height);
	XtAddEventHandler(toolBarForm, StructureNotifyMask, FALSE,
			  resizeViola, 
			  (XtPointer)docViewInfo->violaDocToolsWindow);

	XtAddCallback(scrollBar,
		      XmNdragCallback, scrollBarDrag,
		      (XtPointer) docViewInfo->violaDocViewObj);
	XtAddCallback(scrollBar,
		      XmNvalueChangedCallback, scrollBarValueChanged,
		      (XtPointer) docViewInfo->violaDocViewObj);

	ViolaRegisterMessageHandler("newDocument", newDocument, (void *) docViewInfo);
	ViolaRegisterMessageHandler("sliderConfig", sliderConfig, (void *) docViewInfo);
	ViolaRegisterMessageHandler("searchOn", searchModeMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("searchOff", searchModeMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("saveAs", saveAsMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("historyDocument", newDocument, (void *) docViewInfo);
	ViolaRegisterMessageHandler("historyAdd", historyAddMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("modalErrorDialog", modalErrorDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("errorDialog", errorDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("warningDialog", warningDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("infoDialog", infoDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("promptDialog", promptDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("questionDialog", questionDialogMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("busyCursor", busyCursorMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("idleCursor", idleCursorMH, (void *) docViewInfo);
	ViolaRegisterMessageHandler("helpMessage", tmpMessageMH, (void *) docViewInfo);

	ViolaRegisterMessageHandler("clone", clonePageMH, (void *)docViewInfo);
	ViolaRegisterMessageHandler("back", historyBackUpMH, (void *)docViewInfo);
	ViolaRegisterMessageHandler("prev", historyPrevMH, (void *)docViewInfo);
	ViolaRegisterMessageHandler("next", historyNextMH, (void *)docViewInfo);
/*	ViolaRegisterMessageHandler("replaceHotlistItem", replaceHotlistItemMH, (void *)docViewInfo);
*/
    }
    
    putInBox(&docViews, docViewInfo);
	
    return(docViewInfo);
}


#ifdef UNUSED
Widget makeButtons(form, helpLabel, docViewInfo)
    Widget form, helpLabel;
    DocViewInfo *docViewInfo;
{
    Widget buttonBox, button;
    ClientData *clientData;
    XmString xms;
    int nbuttons = 5;

    buttonBox = XtVaCreateManagedWidget("buttonBox", xmFormWidgetClass, form,
					XmNbottomAttachment, XmATTACH_FORM,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNfractionBase, nbuttons,
					XmNlabelString, xms,
					XmNnavigationType, XmNONE,
					NULL);

    xms = XmStringCreateSimple("Back Up");
    button = XtVaCreateManagedWidget("Back Up", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 1,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    clientData = (ClientData *) calloc(1, sizeof(ClientData));
    clientData->data = (XtPointer) "Back Up";
    clientData->shellInfo = (void *) docViewInfo;
    XtAddCallback(button, XmNactivateCallback, 
		  navigateBackUp, (XtPointer) clientData);
    setHelp(button, helpLabel, "Backup to the previous document.");
    
    xms = XmStringCreateSimple("Previous");
    button = XtVaCreateManagedWidget("Previous", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 2,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    clientData = (ClientData *) calloc(1, sizeof(ClientData));
    clientData->data = (XtPointer) "Previous";
    clientData->shellInfo = (void *) docViewInfo;
    XtAddCallback(button, XmNactivateCallback, 
		  navigatePrev, (XtPointer) clientData);

    setHelp(button, helpLabel, "Go the the previous document but keep the current document in memory.");
   
    xms = XmStringCreateSimple("Next");
    button = XtVaCreateManagedWidget("Next", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 3,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    clientData = (ClientData *) calloc(1, sizeof(ClientData));
    clientData->data = (XtPointer) "Next";
    clientData->shellInfo = (void *) docViewInfo;
    XtAddCallback(button, XmNactivateCallback,
		  navigateNext, (XtPointer) clientData);

    setHelp(button, helpLabel, "Go to the next document in the history list.");

    xms = XmStringCreateSimple("Show Source");
    button = XtVaCreateManagedWidget("Show Source", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 4,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    XtAddCallback(button, XmNactivateCallback,
		  showSource, (XtPointer) docViewInfo);
    setHelp(button, helpLabel, 
	    "View the SGML source of the current document.");
    

    xms = XmStringCreateSimple("Clone Page");
    button = XtVaCreateManagedWidget("Clone Page", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 5,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    XtAddCallback(button, XmNactivateCallback,
		  clonePage, (XtPointer) docViewInfo);
    setHelp(button, helpLabel, 
	    "Duplicate the current document for navigation.");


    xms = XmStringCreateSimple("Clone App");
    button = XtVaCreateManagedWidget("Clone App", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 6,
				     XmNlabelString, xms,
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     XmNsensitive, FALSE,
				     NULL);
    XmStringFree(xms);
    XtAddCallback(button, XmNactivateCallback,
		  cloneApp, (XtPointer) docViewInfo);
    setHelp(button, helpLabel, "Duplicate the current document view in a new application window.");

    xms = XmStringCreateSimple("Close");
    button = XtVaCreateManagedWidget("Close", 
				     xmPushButtonWidgetClass, buttonBox, 
				     XmNleftAttachment, XmATTACH_WIDGET,
				     XmNleftWidget, button,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNlabelString, xms,
				     XmNsensitive, (docViewInfo->shell!=topWidget? TRUE: FALSE),
				     XmNtraversalOn, FALSE,
				     XmNnavigationType, XmNONE,
				     NULL);
    XmStringFree(xms);
    if (docViewInfo->shell != topWidget) {
	XtAddCallback(button, XmNactivateCallback,
		      closeThisShell, (XtPointer) docViewInfo);
	setHelp(button, helpLabel, "Close this particular cloned document viewer window.");
	}
	*/

    return(buttonBox);
}
#endif

void closeAppShell(docViewInfo)
    DocViewInfo *docViewInfo;
{
    /* Tell the Viola object to free itself. */
    if (docViewInfo->violaDocViewObj)
        sendMessage1(docViewInfo->violaDocViewObj, "freeSelf");

    ViolaDeleteMessageHandler("sliderConfig", sliderConfig, (void *) docViewInfo);
    ViolaDeleteMessageHandler("newDocument", newDocument, (void *) docViewInfo);
    ViolaDeleteMessageHandler("searchOn", searchModeMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("searchOff", searchModeMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("historyDocument", newDocument, (void *) docViewInfo);
    ViolaDeleteMessageHandler("historyAdd", historyAddMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("modalErrorDialog", modalErrorDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("errorDialog", errorDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("warningDialog", warningDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("infoDialog", infoDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("promptDialog", promptDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("questionDialog", questionDialogMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("busyCursor", busyCursorMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("idleCursor", idleCursorMH, (void *) docViewInfo);
    ViolaDeleteMessageHandler("helpMessage", tmpMessageMH, (void *) docViewInfo);

    XtRemoveTimeOut(docViewInfo->intervalID);
    
    if (docViewInfo->quitDlog) 
	XtDestroyWidget(docViewInfo->quitDlog); /* Is this necessary? (see next line) */
    XtDestroyWidget(docViewInfo->shell);
    freeDocViewInfo(docViewInfo);
}

/* already exists in viola 
char *makeString(oldstring)
    char *oldstring;
{
    char *newstring;

    if (!oldstring)
	return(NULL);
    newstring = malloc(strlen(oldstring) + 1);
    strcpy(newstring, oldstring);
    return(newstring);
}
*/

/*??*/
Boolean compareDocViewName(vkey, vdata)
    void *vkey, *vdata;
{
    char *name = (char *) vkey;
    DocViewInfo *dvData = (DocViewInfo *) vdata;

    if (!vkey || !vdata)
	return(FALSE);
    
    if (!strcmp(name, dvData->shellName))
	return(TRUE);
    return(FALSE);
}


Boolean compareDocViewShell(vkey, vdata)
    void *vkey, *vdata;
{
    Widget *shell = (Widget *) vkey;
    DocViewInfo *dvData = (DocViewInfo *) vdata;

    if (!vkey || !vdata)
	return(FALSE);
    
    if (*shell == dvData->shell)
	return(TRUE);
    return(FALSE);
}
    

void freeDocViewInfo(vdvi)
    void *vdvi;
{
    DocViewInfo *dvi = (DocViewInfo *) vdvi;

    if (dvi) {
	if (dvi->shellName)
	    free(dvi->shellName);
	if (dvi->docName)
	    free(dvi->docName);
	if (dvi->URL)
	    free(dvi->URL);
	if (dvi->violaDocViewName)
	    free(dvi->violaDocViewName);
	if (selectionOwner == dvi)
	    selectionOwner = NULL;
	free(dvi);
    }
}


/*
Boolean compareBookmarkName(key, data)
    void *key, *data;
{
    Bookmark *bmKey = (Bookmark *) key;
    Bookmark *bmData = (Bookmark *) data;

    if (!key || !data)
	return(FALSE);

    if (!strcpy(bmKey->name, bmData->name))
	return(TRUE);
    return(FALSE);
}


Boolean compareBookmarkDoc(key, data)
    void *key, *data;
{
    Bookmark *bmKey = (Bookmark *) key;
    Bookmark *bmData = (Bookmark *) data;

    if (!key || !data)
	return(FALSE);

    if (!strcpy(bmKey->docName, bmData->docName))
	return(TRUE);
    return(FALSE);
}


void freeBookmark(vbm)
    void *vbm;
{
    Bookmark *bm = (Bookmark *) vbm;

    if (bm) {
	if (bm->name)
	    free(bm->name);
	if (bm->docName)
	    free(bm->docName);
	free(bm);
    }
}
*/

void setHelp(widget, helpLabel, helpText)
    Widget widget, helpLabel;
    char *helpText;
{
    struct helpStruct *hdata = (struct helpStruct *) malloc (sizeof(struct helpStruct));
    hdata->helpLabel = helpLabel;
    
    if (helpText) {
	hdata->helpText = helpText;
	XtAddEventHandler(widget, EnterWindowMask, FALSE,
			  helpHandler, (XtPointer) hdata);
	XtAddEventHandler(widget, LeaveWindowMask, FALSE,
			  blankHelpHandler, (XtPointer) hdata);
    } else {
	hdata->helpText = "";
	XtAddEventHandler(widget, EnterWindowMask, FALSE,
			  helpHandler, (XtPointer) hdata);
	XtAddEventHandler(widget, LeaveWindowMask, FALSE,
			  blankHelpHandler, (XtPointer) hdata);
    }
}


/*
 * This routine closes stdout unless the "-debug" flag is on the command
 * line.  This hides Viola's excessive diagnostic output.
 */
void checkForDebugOutput(argc, argv)
    int argc;
    char *argv[];
{
    int i, fd, ofd;
    Boolean debug=TRUE;
    FILE *output;

    for (i=1; i<argc; i++)
	if (!strcmp("-silent", argv[i]))
	    debug = FALSE;

    if (!debug) {
	output = fopen("/dev/null", "w");
	if (output) {
	    ofd = fileno(output);
	    fd = fileno(stdout);
	    dup2(ofd, fd);
	    fd = fileno(stderr);
	    dup2(ofd, fd);
	} else {
	    fprintf(stderr, "Couldn't open /dev/null.\n");
	}
    }
}


/*
 * Constructs a Motif compound string with a large font title and a small
 * font subtitle.  The resultant label has two lines of text.
 */
XmString makeXMSTitle(title, subTitle)
    char *title, *subTitle;
{
    XmString titleXMS, subTitleXMS, resultXMS;

    if (subTitle && strlen(subTitle)) {
	titleXMS = XmStringSegmentCreate(title,
					 "titleFont",
					 XmSTRING_DIRECTION_L_TO_R,
					 (strlen(subTitle)? TRUE: FALSE));
	subTitleXMS = XmStringSegmentCreate(subTitle, "subTitleFont",
					    XmSTRING_DIRECTION_L_TO_R, FALSE);
	resultXMS = XmStringConcat(titleXMS, subTitleXMS);
	XmStringFree(subTitleXMS);
	XmStringFree(titleXMS);
    } else {
	/* When no subtitle, the title is actually the document URL. */
	resultXMS = XmStringCreateLtoR(title, "urlFont");
    }

    return(resultXMS);
}


void tmpMessageMH(arg, argc, clientData)
    char *arg[];
    int argc;
    void *clientData;
{
    XmString xms;
    DocViewInfo *dvi = (DocViewInfo *) clientData;

    if (argc != 3)
	return;

    if (dvi->cloneID != atol(arg[1]))
	return;

    showHelpMessage(dvi->msgLabel, arg[2]);
}
