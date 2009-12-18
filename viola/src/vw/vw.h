/*
 * vw.h
 *
 * Main header file for Concerto interface program.
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

#ifndef _SPIDER_H_
#define _SPIDER_H_

#include "../viola/mystrings.h"

#include <Xm/Xm.h>
#include "box.h"
#include "../viola/viola.h"


/* --- CONSTANTS ----------------------------------------------------------- */
#define VIOLA_OBJ_NAME "mvw"
#define SBAR_MAGNITUDE 100
#define GLOBE_TIME 30000
#define TITLE_ICON_WIDTH	50
#define TITLE_ICON_HEIGHT	50
#define TOOLBAR_HEIGHT 		38
#define SEARCH_HEIGHT		33
#define FORM_SPACING		4
#define PRINT_COMMAND		"/usr/ucb/lpr"

#define MAIL_COMMAND		"/usr/ucb/mail"
#define DEVELOPER_ADDRESS	"viola@ora.com"


/* --- GLOBALS ------------------------------------------------------------- */
extern XtAppContext appCon;
extern Widget topWidget;
extern Pixmap globes[];
extern Pixmap icon;
extern char *appName;
extern int nshells;
extern int toolBarHeight;

/*
 * This is here until I figure out a way to get around the Motif1.2.3 bug
 * that prevents resource conversion of a multiple font fontList.  This
 * thing prevents users from using resources to set the font for the title
 * label.
 */
extern XmFontList titleFontList;

extern int activeHelpLock;


/* --- DATA TYPES ---------------------------------------------------------- */
/*
 * Each top level shell with a browser interface is a "DocView".  Each
 * DocView has a corresponding DocViewInfo structure.  All of these are
 * contained in the global Box "docViews".
 */
typedef struct DocViewInfoStruct {
    char *shellName;
    char *docName;
    char *URL;
    long cloneID;  /* shell field used as cloneID ... but the
		      first shell always has cloneID of 0. */
    
    Widget shell;
    Widget topMostWidget;
    Widget title;
    Widget canvas;
    Widget quitDlog;
    Widget scrollBar;
    Widget msgLabel;		/* Label for temporary messages to user. */

    Widget titleIcon;
    XtIntervalId intervalID;
    int globeNum;

    Widget searchLabel;
    Widget searchText;

    Widget historyDlog;
    Widget historyListWidget;
    char **historyList;
    int historySize;
    int nHistoryItems;
    int currentHistoryItem;

    int editable;
    int editorDataChanged;
    Widget editorSaveButton;
    Widget textEditor;

    char *violaDocViewName;
    char *violaDocToolsName;

    VObj *violaDocViewObj;
    VObj *violaDocToolsObj;

    Window violaDocViewWindow;
    Window violaDocToolsWindow;

    Widget hotlistListWidget;
    Widget hotlistDlog;
    int nHotlistItems;
    int hotlistSize;
    int currentHotlistItem;

} DocViewInfo;
extern Box *docViews;


/*
 * For use in callbacks that require more info than is contained in the
 * DocViewInfo structure.
 */
typedef struct ClientDataStruct {
    void *data;
    void *shellInfo;
} ClientData;


Boolean compareDocViewName();
Boolean compareDocViewShell();
void freeDocViewInfo();


typedef struct BookmarkStruct {
    char *name;
    char *docName;
    Widget menuButton;
} Bookmark;

Boolean compareBookmarkName();
Boolean compareBookmarkDoc();
void freeBookmark();


/* --- PROTOTYPES ---------------------------------------------------------- */
DocViewInfo *makeBrowserInterface();
void closeAppShell();
char *makeString();
void setHelp();
XmString makeXMSTitle();
void tmpMessageMH();


/* --- MACROS -------------------------------------------------------------- */
#define mainViewInfo() getFromBox(&docViews, \
				  (void *) &topWidget, \
				  compareDocViewShell, \
				  TRUE)


/* This attempt at setting up a single FontList didn't work ... figure it out.
 * 
 * static String fallback_resources[] = {
 *   "*fontList: *cour*bold*-r-*-14-*=textFont\n\
 *   		*times*bold*-r-*-14-*=labelFont\n\
 *               *times*bold*-r-*-14-*=buttonFont\n\
 *   		*times*med*-r-*-14-*=menuFont\n\
 *		*times*bold*-r-*-14-*=menuButtonFont\n\
 *   		*times*bold*-r-*-24-*=titleFont",
 *   "*labelFontList: *times*bold*-r-*-14-*=labelFont",
 *   "*textFontList: *cour*bold*-r-*-14-*=textFont",
 *   "*buttonFontList: *cour*bold*-r-*-14-*=textFont",
 *   "*XmText*fontList: *cour*bold*-r-*-14-*=textFont",
 *   "*XmTextField*fontList: *cour*bold*-r-*-14-*=textFont",
 *   NULL,
 * };
 */


struct helpStruct {
    Widget helpLabel;
    char *helpText;
};


#endif _SPIDER_H_

#define makeString(s) saveString(s)
