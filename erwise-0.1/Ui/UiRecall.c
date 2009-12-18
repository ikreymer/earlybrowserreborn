static char *rcsid = "$Id$";


#include "UiIncludes.h"


static void uirecallfreeprevious(void);
static void uirecallsetitems(char **listitems, int nitems);

static Widget uicreaterecallform(void);
static Widget uicreaterecalllabel(Widget parent);
static Widget uicreaterecallopen(Widget parent);
static Widget uicreaterecallclose(Widget parent);
static Widget uicreaterecallseparator(Widget parent, Widget bottomwdg);
static Widget
 uicreaterecalllist(Widget parent, Widget topwdg,
		     Widget bottomwdg);
static void uirecallopencb(Widget wdg, caddr_t ignored,
			    XmListCallbackStruct * calldata);
static void uirecallclosecb(Widget wdg, caddr_t ignored,
			     XmListCallbackStruct * calldata);


static char *uitopaddress = (char *) NULL;
static char **uilistitems;
static int uinitems;
static void (*uirecallcallback) (char *topaddress, char *address,
				  char *parentaddress);


int UiDisplayRecallDialog(listitems, nitems, callback)
char **listitems;
int nitems;
void (*callback) (char *topaddress, char *address, char *parentaddress);
{
    uiRecallGfx_t *recallgfx = &uiTopLevel.RecallGfx;

    uirecallfreeprevious();

    uitopaddress = uiPageInfo.CurrentPage->Hierarchy->Address;
    uilistitems = listitems;
    uinitems = nitems;
    uirecallcallback = callback;

    if (recallgfx->FormWdg) {
	XtMapWidget(XtParent(recallgfx->FormWdg));

	uiWidgetPlacement(XtParent(recallgfx->FormWdg),
			  uiTopLevel.GlobalSettings.RecallPlacement);

	uirecallsetitems(listitems, nitems);

	return UI_OK;
    }
    recallgfx->FormWdg = uicreaterecallform();
    recallgfx->LabelWdg = uicreaterecalllabel(recallgfx->FormWdg);
    recallgfx->OpenWdg = uicreaterecallopen(recallgfx->FormWdg);
    recallgfx->CloseWdg = uicreaterecallclose(recallgfx->FormWdg);
    recallgfx->SeparatorWdg = uicreaterecallseparator(recallgfx->FormWdg,
						      recallgfx->OpenWdg);
    recallgfx->ListWdg = uicreaterecalllist(recallgfx->FormWdg,
					    recallgfx->LabelWdg,
					    recallgfx->SeparatorWdg);

    uirecallsetitems(listitems, nitems);

    XtManageChild(recallgfx->FormWdg);
    XtRealizeWidget(XtParent(recallgfx->FormWdg));

    uiWidgetPlacement(XtParent(recallgfx->FormWdg),
		      uiTopLevel.GlobalSettings.RecallPlacement);

    return UI_OK;
}


void uiRecallUpdateDialog()
{
    if (!uiPageInfo.CurrentPage && uiTopLevel.RecallGfx.FormWdg) {
	uirecallfreeprevious();
	uirecallsetitems((char **) NULL, 0);
	uitopaddress = (char *) NULL;
	XtUnmapWidget(XtParent(uiTopLevel.RecallGfx.FormWdg));
    }
}


static Widget
 uicreaterecallform()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("Recall",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_RECALL_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "RecallDialog", args, nargs);

    return formwdg;
}


static Widget
 uicreaterecalllabel(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg;

    labelstr = XmStringCreateSimple("List of documents");
    args = uiVaSetArgs(&nargs,
		       XmNlabelString, labelstr,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, UI_LIST_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    labelwdg = XmCreateLabelGadget(formwdg, "ListLabel", args, nargs);
    XtManageChild(labelwdg);
    XmStringFree(labelstr);

    return labelwdg;
}


static Widget
 uicreaterecallopen(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget openwdg;

    args = uiVaSetArgs(&nargs,
		       XmNleftAttachment, XmATTACH_POSITION,
		       XmNleftPosition, 5,
		       XmNrightAttachment, XmATTACH_POSITION,
		       XmNrightPosition, 40,
		       XmNbottomAttachment, XmATTACH_FORM,
		       XmNbottomOffset, UI_LIST_WDG_OFFSET, NULL);
    openwdg = XmCreatePushButtonGadget(formwdg, "Open", args, nargs);
    XtAddCallback(openwdg, XmNactivateCallback,
		  (XtCallbackProc) uirecallopencb, (caddr_t) NULL);
    XtManageChild(openwdg);

    return openwdg;
}


static Widget
 uicreaterecallclose(formwdg)
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
		       XmNbottomOffset, UI_LIST_WDG_OFFSET, NULL);
    closewdg = XmCreatePushButtonGadget(formwdg, "Close", args, nargs);
    XtAddCallback(closewdg, XmNactivateCallback,
		  (XtCallbackProc) uirecallclosecb, (caddr_t) NULL);
    XtManageChild(closewdg);

    return closewdg;
}


static Widget
 uicreaterecallseparator(formwdg, bottomwdg)
Widget formwdg;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;

    args = uiVaSetArgs(&nargs,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_LIST_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(formwdg, "ListSeparator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static Widget
 uicreaterecalllist(formwdg, topwdg, bottomwdg)
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
		       XmNtopOffset, UI_LIST_WDG_OFFSET,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_LIST_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_LIST_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_LIST_WDG_OFFSET, NULL);
    listwdg = XmCreateScrolledList(formwdg, "ListList", args, nargs);
    XtManageChild(listwdg);
    XtAddCallback(listwdg, XmNdefaultActionCallback,
		  (XtCallbackProc) uirecallopencb, (caddr_t) NULL);

    return listwdg;
}


void uirecallfreeprevious()
{
    if (uitopaddress && uinitems) {
	while (uinitems--)
	    uiFree(uilistitems[uinitems]);

	uiFree(uilistitems);
    }
}


static void uirecallsetitems(listitems, nitems)
char **listitems;
int nitems;
{
    Widget listwdg = uiTopLevel.RecallGfx.ListWdg;
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


static void uirecallopencb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    Widget listwdg = uiTopLevel.RecallGfx.ListWdg;
    int *poslist;
    int poscount;

    if (uinitems)
	if (XmListGetSelectedPos(listwdg, &poslist, &poscount)) {
	    uiDefineCursor(uiBusyCursor);
	    if (uiHelpOnActionCB) {
		(*uiHelpOnActionCB) ("Get page");
		uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
	    } else
		(*uirecallcallback) (uilistitems[poslist[0] - 1],
				     uitopaddress, (char *) NULL);

	    uiUndefineCursor();

	    XtFree(poslist);
	}
}


static void uirecallclosecb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    XtUnmapWidget(XtParent(uiTopLevel.RecallGfx.FormWdg));
}
