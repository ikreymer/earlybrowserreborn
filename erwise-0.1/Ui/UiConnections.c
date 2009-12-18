static char *rcsid = "$Id$";

#include "UiIncludes.h"

static Widget uicreateconnectionsform();
static Widget uicreateconnectionslabel(Widget formwdg);
static Widget uicreateconnectionskill(Widget formwdg);
static Widget uicreateconnectionsclose(Widget formwdg);
static Widget uicreateconnectionsseparator(Widget formwdg, Widget bottomwdg);
static Widget
 uicreateconnectionslist(Widget formwdg, Widget topwdg,
			  Widget bottomwdg);
static void uiconnectionsfreeprevious(void);
static void uiconnectionssetitems(char **listitems, void **connections,
				   int nitems);
static void uiconnectionskillcb(Widget wdg, caddr_t ignored,
				 XmListCallbackStruct * calldata);
static void uiconnectionsclosecb(Widget wdg, caddr_t ignored,
				  XmListCallbackStruct * calldata);


static int uiconnectionsdisplayed = FALSE;
static char **uilistitems;
static void **uiconnections;
static int uinitems;
static void (*uiconnectionscallback) (void *connection);


int UiDisplayConnectionsDialog(listitems, connections, nitems, callback)
char **listitems;
void **connections;
int nitems;
void (*callback) (void *connection);
{
    uiConnectionsGfx_t *connectionsgfx = &uiTopLevel.ConnectionsGfx;

    uiconnectionsfreeprevious();

    uiconnectionsdisplayed = TRUE;
    uilistitems = listitems;
    uiconnections = connections;
    uinitems = nitems;
    uiconnectionscallback = callback;

    if (connectionsgfx->FormWdg) {
	XtMapWidget(XtParent(connectionsgfx->FormWdg));
	uiconnectionssetitems(listitems, connections, nitems);

	return UI_OK;
    }
    connectionsgfx->FormWdg = uicreateconnectionsform();
    connectionsgfx->LabelWdg =
	uicreateconnectionslabel(connectionsgfx->FormWdg);
    connectionsgfx->KillWdg =
	uicreateconnectionskill(connectionsgfx->FormWdg);
    connectionsgfx->CloseWdg =
	uicreateconnectionsclose(connectionsgfx->FormWdg);
    connectionsgfx->SeparatorWdg =
	uicreateconnectionsseparator(connectionsgfx->FormWdg,
				     connectionsgfx->KillWdg);
    connectionsgfx->ListWdg =
	uicreateconnectionslist(connectionsgfx->FormWdg,
				connectionsgfx->LabelWdg,
				connectionsgfx->SeparatorWdg);

    uiconnectionssetitems(listitems, connections, nitems);

    XtManageChild(connectionsgfx->FormWdg);
    XtRealizeWidget(XtParent(connectionsgfx->FormWdg));

    return UI_OK;
}


int UiConnectionsDialogDisplayed()
{
    return uiconnectionsdisplayed;
}


void uiConnectionsUpdateDialog()
{
    if (!uiPageInfo.CurrentPage && uiTopLevel.ConnectionsGfx.FormWdg) {
	uiconnectionsfreeprevious();
	uiconnectionssetitems((char **) NULL, (void **) NULL, 0);
	XtUnmapWidget(XtParent(uiTopLevel.ConnectionsGfx.FormWdg));
    }
}


static Widget
 uicreateconnectionsform()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("Connections",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_CONNECTIONS_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "ConnectionsDialog",
			   args, nargs);

    return formwdg;
}


static Widget
 uicreateconnectionslabel(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg;

    labelstr = XmStringCreateSimple("Open connections");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_CONNECTIONS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "ConnectionsLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    return labelwdg;
}


static Widget
 uicreateconnectionskill(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget killwdg;

    args = uiVaSetArgs(&nargs,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 40,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, UI_CONNECTIONS_WDG_OFFSET, NULL);
    killwdg = XmCreatePushButtonGadget(formwdg, "Kill", args, nargs);
    XtAddCallback(killwdg, XmNactivateCallback,
		  (XtCallbackProc) uiconnectionskillcb, (caddr_t) NULL);
    XtManageChild(killwdg);

    return killwdg;
}


static Widget
 uicreateconnectionsclose(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget closewdg;

    args = uiVaSetArgs(&nargs,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 60,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 95,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, UI_CONNECTIONS_WDG_OFFSET, NULL);
    closewdg = XmCreatePushButtonGadget(formwdg, "Close", args, nargs);
    XtAddCallback(closewdg, XmNactivateCallback,
		  (XtCallbackProc) uiconnectionsclosecb, (caddr_t) NULL);
    XtManageChild(closewdg);

    return closewdg;
}


static Widget
 uicreateconnectionsseparator(formwdg, bottomwdg)
Widget formwdg;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;

    args = uiVaSetArgs(&nargs,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_CONNECTIONS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(formwdg, "ConnectionsSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static Widget
 uicreateconnectionslist(formwdg, topwdg, bottomwdg)
Widget formwdg;
Widget topwdg;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget listwdg;

    args = uiVaSetArgs(&nargs,
		       XmNvisibleItemCount, 15,
		       XmNwidth, 300,
		       XmNselectionPolicy, XmSINGLE_SELECT,
		       XmNlistSizePolicy, XmCONSTANT,
		       XmNscrollBarDisplayPolicy, XmSTATIC,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_CONNECTIONS_WDG_OFFSET,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_CONNECTIONS_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_CONNECTIONS_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_CONNECTIONS_WDG_OFFSET, NULL);
    listwdg = XmCreateScrolledList(formwdg, "ConnectionsList", args, nargs);
    XtManageChild(listwdg);
    XtAddCallback(listwdg, XmNdefaultActionCallback,
		  (XtCallbackProc) uiconnectionskillcb, (caddr_t) NULL);

    return listwdg;
}


void uiconnectionsfreeprevious()
{
    if (uinitems)
	uiFree(uilistitems);
}


static void uiconnectionssetitems(listitems, connections, nitems)
char **listitems;
void **connections;
int nitems;
{
    Widget listwdg = uiTopLevel.ConnectionsGfx.ListWdg;
    int i;
    XmString *tmpstr = uiMalloc(nitems * sizeof(XmString));

    XmListDeleteAllItems(listwdg);
    if (nitems) {
	for (i = 0; i < nitems; i++)
	    tmpstr[i] = XmStringCreateSimple(listitems[i]);

	XmListAddItems(listwdg, tmpstr, nitems, 0);
	for (i = 0; i < nitems; i++)
	    XmStringFree(tmpstr[i]);
	uiFree((void *) tmpstr);
    }
}




static void uiconnectionskillcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    Widget connectionswdg = uiTopLevel.ConnectionsGfx.ListWdg;
    int *poslist;
    int poscount;
    char *parentaddress;

    if (uinitems)
	if (XmListGetSelectedPos(connectionswdg, &poslist, &poscount)) {
	    uiDefineCursor(uiBusyCursor);
	    if (uiHelpOnActionCB) {
		(*uiHelpOnActionCB) ("Kill connection");
		uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
	    } else
		(*uiconnectionscallback) (uiconnections[poslist[0] - 1]);

	    uiUndefineCursor();

	    XtFree(poslist);
	}
}


static void uiconnectionsclosecb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    XtUnmapWidget(XtParent(uiTopLevel.ConnectionsGfx.FormWdg));
    uiconnectionsdisplayed = FALSE;
}
