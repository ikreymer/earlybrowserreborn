static char *rcsid = "$Id: UiList.c,v 1.1 1992/03/26 18:13:50 kny Exp kny $";


#include "UiIncludes.h"


static void uilistfreeprevious(void);
static void uilistsetitems(char **listitems, int nitems);

static Widget uicreatelistform(void);
static Widget uicreatelistlabel(Widget parent);
static Widget uicreatelistopen(Widget parent);
static Widget uicreatelistclose(Widget parent);
static Widget uicreatelistseparator(Widget parent, Widget bottomwdg);
static Widget uicreatelistlist(Widget parent, Widget topwdg, Widget bottomwdg);
static void uilistopencb(Widget wdg, caddr_t ignored,
			  XmListCallbackStruct * calldata);
static void uilistclosecb(Widget wdg, caddr_t ignored,
			   XmListCallbackStruct * calldata);


static uiPage_t *uilistpage = (uiPage_t *) NULL;
static char **uilistitems;
static char **uiaddresses;
static int uinitems;
static void (*uilistcallback) (char *topaddress, char *address,
			        char *parentaddress);


int UiDisplayListDialog(listitems, addresses, nitems, callback)
char **listitems;
char **addresses;
int nitems;
void (*callback) (char *topaddress, char *address, char *parentaddress);
{
    uiListGfx_t *listgfx = &uiTopLevel.ListGfx;

    uilistfreeprevious();

    uilistpage = uiPageInfo.CurrentPage;
    uilistitems = listitems;
    uiaddresses = addresses;
    uinitems = nitems;
    uilistcallback = callback;

    if (listgfx->FormWdg) {
	XtMapWidget(XtParent(listgfx->FormWdg));

	uiWidgetPlacement(XtParent(listgfx->FormWdg),
			  uiTopLevel.GlobalSettings.ListPlacement);

	uilistsetitems(listitems, nitems);

	return UI_OK;
    }
    listgfx->FormWdg = uicreatelistform();
    listgfx->LabelWdg = uicreatelistlabel(listgfx->FormWdg);
    listgfx->OpenWdg = uicreatelistopen(listgfx->FormWdg);
    listgfx->CloseWdg = uicreatelistclose(listgfx->FormWdg);
    listgfx->SeparatorWdg = uicreatelistseparator(listgfx->FormWdg,
						  listgfx->OpenWdg);
    listgfx->ListWdg = uicreatelistlist(listgfx->FormWdg, listgfx->LabelWdg,
					listgfx->SeparatorWdg);

    uilistsetitems(listitems, nitems);

    XtManageChild(listgfx->FormWdg);
    XtRealizeWidget(XtParent(listgfx->FormWdg));

    uiWidgetPlacement(XtParent(listgfx->FormWdg),
		      uiTopLevel.GlobalSettings.ListPlacement);

    return UI_OK;
}


void uiListUpdateDialog(page)
uiPage_t *page;
{
    if (uiTopLevel.ListGfx.FormWdg && (page == uilistpage)) {
	uilistfreeprevious();
	uilistsetitems((char **) NULL, 0);
	uilistpage = (uiPage_t *) NULL;
	XtUnmapWidget(XtParent(uiTopLevel.ListGfx.FormWdg));
    }
}


static Widget
 uicreatelistform()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("List",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_LIST_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "ListDialog", args, nargs);

    return formwdg;
}


static Widget
 uicreatelistlabel(formwdg)
Widget formwdg;
{
    ArgList args;
    Cardinal nargs;
    XmString labelstr;
    Widget labelwdg;

    labelstr = XmStringCreateSimple("List of references");
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
 uicreatelistopen(formwdg)
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
		  (XtCallbackProc) uilistopencb, (caddr_t) NULL);
    XtManageChild(openwdg);

    return openwdg;
}


static Widget
 uicreatelistclose(formwdg)
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
		  (XtCallbackProc) uilistclosecb, (caddr_t) NULL);
    XtManageChild(closewdg);

    return closewdg;
}


static Widget
 uicreatelistseparator(formwdg, bottomwdg)
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
 uicreatelistlist(formwdg, topwdg, bottomwdg)
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
		  (XtCallbackProc) uilistopencb, (caddr_t) NULL);

    return listwdg;
}


void uilistfreeprevious()
{
    if (uilistpage && uinitems) {
	uiFree((void *) uiaddresses);

	while (uinitems--)
	    uiFree(uilistitems[uinitems]);

	uiFree(uilistitems);
    }
}


static void uilistsetitems(listitems, nitems)
char **listitems;
int nitems;
{
    Widget listwdg = uiTopLevel.ListGfx.ListWdg;
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


static void uilistopencb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    Widget listwdg = uiTopLevel.ListGfx.ListWdg;
    int *poslist;
    int poscount;
    char *parentaddress;

    if (uinitems)
	if (XmListGetSelectedPos(listwdg, &poslist, &poscount)) {
	    uiDefineCursor(uiBusyCursor);
	    if (uiHelpOnActionCB) {
		(*uiHelpOnActionCB) ("Get page");
		uiHelpOnActionCB = (void (*) (char *actionstring)) NULL;
	    } else {
		parentaddress =
		    HTAnchor_address((HTAnchor *)
				     uilistpage->HText->node_anchor);
		(*uilistcallback) (uiaddresses[poslist[0] - 1],
				   uilistpage->Hierarchy->Address,
				   parentaddress);
	    }

	    uiUndefineCursor();

	    XtFree(poslist);
	}
}


static void uilistclosecb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    XtUnmapWidget(XtParent(uiTopLevel.ListGfx.FormWdg));
}
