/* $Id: UiProtos.h,v 1.4 1992/03/26 18:13:50 kny Exp kny $ */


/*
 *  Misc. prototypes (external to library)
 */

//extern int fprintf(FILE * stream, char *format,...);


/*
 *  UiInit.c
 */

extern uiTopLevel_t uiTopLevel;
extern char **uiSelectionArray;
extern Cursor uiBusyCursor;
extern Cursor uiArrowCursor;
extern void *(*uiConfigPF) (void *table, char *item);
extern void *(*uiConfigSetPF) (void *table, char *item, void *value);


/*
 *  UiControlPanel.c
 */


/*
 *  UiSelectionBox.c
 */


/*
 *  UiPage.c
 */

extern void uiPageUpdateWindow(uiPage_t * page);
extern void uiPageAttachCallbacks(void);
extern void uiPageDefineKeys(void);

extern uiPageInfo_t uiPageInfo;


/*
 *  UiSearch.c
 */

extern void uiSearchUpdateDialog(void);


/*
 *  UiPageSettings.c
 */

extern void uiPageSettingsUpdateDialog(void);


/*
 *  UiList.c
 */

extern void uiListUpdateDialog(uiPage_t * page);


/*
 *  UiRecall.c
 */

extern void uiRecallUpdateDialog(void);


/*
 *  UiMisc.c
 */

extern uiAction_t *uiFindAction(char *actionname);
extern uiKey_t *uiFindKey(char *keyname, int modifier);
extern uiVariable_t *uiFindVariable(char *varname);
extern int uiAddWidgetInfo(char *varname, Widget wdg, uiWdgType_t wdgtype);
extern void uiDialogActivateCB(Widget wdg, uiActionData_t * actiondata,
			        XmAnyCallbackStruct * calldata);
extern void uiDialogVariableCB(Widget wdg, caddr_t variablename,
			        XmAnyCallbackStruct * calldata);
extern void uiDefineCursor(Cursor cursor);
extern void uiUndefineCursor(void);
extern ArgList uiVaSetArgs(int *nargs, ...);
extern XtArgVal uiGetArg(Widget wdg, String resource);
extern void *uiMalloc(int size);
extern void *uiReAlloc(void *, int size);
extern void uiFree(void *ptr);
extern void uiDisplayWarning(char *text);
extern void uiDisplayFatal(char *text);
extern void uiWidgetPlacement(Widget wdg, int placement);
extern void (*uiHelpOnActionCB) (char *actionstring);
