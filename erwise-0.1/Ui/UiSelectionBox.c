static char *rcsid = "$Id: UiSelectionBox.c,v 1.3 1992/03/26 18:13:50 kny Exp $";

#include "UiIncludes.h"


static Widget uicreatesbformdialog();
static Widget
 uicreatesbfsbox(Widget formwdg,
		  void (*callback) (char *nodename));
static Widget uicreatesbseparator(Widget formwdg, Widget fsboxwdg);
static Widget uicreatesbadd(Widget formwdg, Widget fsboxwdg,
			     Widget separatorwdg);
static Widget uicreatesbdelete(Widget formwdg, Widget fsboxwdg);
static Widget uicreatesblist(Widget formwdg, Widget deletewdg,
			      Widget separatorwdg);
static char *uifixselection(char *selection);

static void uiselectionboxokcb(Widget wdg, caddr_t callback,
			        XmPushButtonCallbackStruct * calldata);
static void uiselectionboxcancelcb(Widget wdg, caddr_t ignored,
				  XmPushButtonCallbackStruct * calldata);
static void uiselectionboxaddcb(Widget wdg, caddr_t ignored,
				 XmPushButtonCallbackStruct * calldata);
static void uiselectionboxdeletecb(Widget wdg, caddr_t ignored,
				  XmPushButtonCallbackStruct * calldata);
static void uiselectionboxclickcb(Widget wdg, caddr_t ignored,
				   XmListCallbackStruct * calldata);
static void uiselectionboxupdateconfig(void);


uiTopLevel_t uiTopLevel;
char **uiSelectionArray;


static int uidummy;


int UiDisplaySelectionBox(callback)
void (*callback) (char *nodename);
{
    uiSelectionBoxGfx_t *sbgfx = &uiTopLevel.SBGfx;
    XmString dummystr;
    int i = 0;

    if (sbgfx->FormWdg) {
	XtMapWidget(XtParent(sbgfx->FormWdg));

	return UI_OK;
    }
    sbgfx->FormWdg = uicreatesbformdialog();
    sbgfx->FSBoxWdg = uicreatesbfsbox(sbgfx->FormWdg, callback);
    sbgfx->SeparatorWdg = uicreatesbseparator(sbgfx->FormWdg, sbgfx->FSBoxWdg);
    sbgfx->AddWdg = uicreatesbadd(sbgfx->FormWdg, sbgfx->FSBoxWdg,
				  sbgfx->SeparatorWdg);
    sbgfx->DeleteWdg = uicreatesbdelete(sbgfx->FormWdg, sbgfx->FSBoxWdg);
    sbgfx->ListWdg = uicreatesblist(sbgfx->FormWdg, sbgfx->DeleteWdg,
				    sbgfx->SeparatorWdg);

    XtManageChild(sbgfx->FormWdg);
    XtRealizeWidget(XtParent(sbgfx->FormWdg));

    if (!uiSelectionArray) {
	dummystr = XmStringCreateSimple(" ");
	XmListAddItem(uiTopLevel.SBGfx.ListWdg, dummystr, 0);
	XtSetSensitive(uiTopLevel.SBGfx.ListWdg, FALSE);
	XmStringFree(dummystr);
	uidummy = TRUE;
    } else {
	while (uiSelectionArray[i]) {
	    dummystr = XmStringCreateSimple(uiSelectionArray[i++]);
	    XmListAddItem(uiTopLevel.SBGfx.ListWdg, dummystr, 0);
	    XmStringFree(dummystr);
	}
	uidummy = FALSE;
    }

    return UI_OK;
}


static Widget
 uicreatesbformdialog()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("SelectionBox",
				      topLevelShellWidgetClass,
				      NULL, 0);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "SelectionBox",
			   args, nargs);

    return formwdg;
}


static Widget
 uicreatesbfsbox(formwdg, callback)
Widget formwdg;
void (*callback) (char *nodename);
{
    ArgList args;
    Cardinal nargs;
    Widget fsboxwdg, textwdg, okwdg, cancelwdg;

    args = uiVaSetArgs(&nargs,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, 1,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, 1,
		       XmNbottomOffset, 1,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    fsboxwdg = XmCreateFileSelectionBox(formwdg, "FSBox",
					args, nargs);
    XtUnmanageChild(XmFileSelectionBoxGetChild(fsboxwdg,
					       XmDIALOG_HELP_BUTTON));

    textwdg = XmFileSelectionBoxGetChild(fsboxwdg, XmDIALOG_TEXT);
    XmTextSetString(textwdg, "");

    okwdg = XmFileSelectionBoxGetChild(fsboxwdg, XmDIALOG_OK_BUTTON);
    XtAddCallback(okwdg, XmNactivateCallback, uiselectionboxokcb,
		  (caddr_t) callback);

    cancelwdg = XmFileSelectionBoxGetChild(fsboxwdg, XmDIALOG_CANCEL_BUTTON);
    XtAddCallback(cancelwdg, XmNactivateCallback, uiselectionboxcancelcb,
		  (caddr_t) NULL);
    XtManageChild(fsboxwdg);

    return fsboxwdg;
}


static Widget
 uicreatesbseparator(formwdg, fsboxwdg)
Widget formwdg;
Widget fsboxwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget separatorwdg;

    args = uiVaSetArgs(&nargs,
		       XmNorientation, XmVERTICAL,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, fsboxwdg,
		       XmNleftOffset, 10,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNbottomAttachment, XmATTACH_FORM, NULL);
    separatorwdg = XmCreateSeparatorGadget(formwdg, "Separator",
					   args, nargs);
    XtManageChild(separatorwdg);

    return separatorwdg;
}


static Widget
 uicreatesbadd(formwdg, fsboxwdg, separatorwdg)
Widget formwdg;
Widget fsboxwdg;
Widget separatorwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget addwdg;

    args = uiVaSetArgs(&nargs,
		       XmNwidth, 70,
		       XmNleftOffset, 20,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNbottomWidget, fsboxwdg,
		       XmNbottomOffset, 20,
		       XmNleftWidget, separatorwdg, NULL);
    addwdg = XmCreatePushButtonGadget(formwdg, "Add",
				      args, nargs);
    XtManageChild(addwdg);
    XtAddCallback(addwdg, XmNactivateCallback, uiselectionboxaddcb,
		  (caddr_t) NULL);

    return addwdg;
}


static Widget
 uicreatesbdelete(formwdg, fsboxwdg)
Widget formwdg;
Widget fsboxwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget deletewdg;

    args = uiVaSetArgs(&nargs,
		       XmNwidth, 70,
		       XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		       XmNbottomWidget, fsboxwdg,
		       XmNbottomOffset, 20,
		       XmNrightOffset, 10,
		       XmNrightAttachment, XmATTACH_FORM, NULL);
    deletewdg = XmCreatePushButtonGadget(formwdg, "Delete",
					 args, nargs);
    XtManageChild(deletewdg);
    XtAddCallback(deletewdg, XmNactivateCallback, uiselectionboxdeletecb,
		  (caddr_t) NULL);

    return deletewdg;
}


static Widget
 uicreatesblist(formwdg, deletewdg, separatorwdg)
Widget formwdg;
Widget deletewdg;
Widget separatorwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget listwdg;

    args = uiVaSetArgs(&nargs,
		       XmNvisibleItemCount, 15,
		       XmNwidth, 200,
		       XmNselectionPolicy, XmSINGLE_SELECT,
		       XmNlistSizePolicy, XmCONSTANT,
		       XmNscrollBarDisplayPolicy, XmSTATIC,
		       XmNtopAttachment, XmATTACH_FORM,
		       XmNtopOffset, 11,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, deletewdg,
		       XmNbottomOffset, 20,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, 11,
		       XmNleftOffset, 20,
		       XmNleftAttachment, XmATTACH_WIDGET,
		       XmNleftWidget, separatorwdg, NULL);
    listwdg = XmCreateScrolledList(formwdg, "List",
				   args, nargs);
    XtManageChild(listwdg);
    XtAddCallback(listwdg, XmNdefaultActionCallback, uiselectionboxclickcb,
		  (caddr_t) NULL);

    return listwdg;
}


static char *
 uifixselection(selection)
char *selection;
{
    char *newselection;

    if (!selection)
	return selection;

    if (index(selection, ':'))
	return strdup(selection);

    newselection = uiMalloc(strlen(selection) + strlen("file:") + 1);
    strcpy(newselection, "file:");
    strcat(newselection, selection);

    return newselection;
}


static void uiselectionboxokcb(wdg, callback, calldata)
Widget wdg;
caddr_t callback;
XmPushButtonCallbackStruct *calldata;
{
    Widget textwdg;
    char *selection, *fixedselection;

    textwdg = XmFileSelectionBoxGetChild(uiTopLevel.SBGfx.FSBoxWdg,
					 XmDIALOG_TEXT);
    selection = XmTextGetString(textwdg);
    fixedselection = uifixselection(selection);
    XtUnmapWidget(XtParent(uiTopLevel.SBGfx.FormWdg));
    (*(void (*) (char *nodename)) callback) (fixedselection);

    XtFree(selection);
    XtFree(fixedselection);
}


static void uiselectionboxcancelcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmPushButtonCallbackStruct *calldata;
{
    XtUnmapWidget(XtParent(uiTopLevel.SBGfx.FormWdg));
}


static void uiselectionboxaddcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmPushButtonCallbackStruct *calldata;
{
    Widget textwdg;
    char *selection, *fixedselection;
    XmString selectionstr;
    int *poslist;
    int poscount;

    textwdg = XmFileSelectionBoxGetChild(uiTopLevel.SBGfx.FSBoxWdg,
					 XmDIALOG_TEXT);
    selection = XmTextGetString(textwdg);
    fixedselection = uifixselection(selection);

    if (fixedselection && fixedselection[0]) {
	selectionstr = XmStringCreateSimple(fixedselection);
	if (uidummy) {
	    XmListDeletePos(uiTopLevel.SBGfx.ListWdg, 1);
	    XtSetSensitive(uiTopLevel.SBGfx.ListWdg, TRUE);
	    uidummy = FALSE;
	}
	if (XmListGetSelectedPos(uiTopLevel.SBGfx.ListWdg, &poslist,
				 &poscount)) {
	    XmListDeselectPos(uiTopLevel.SBGfx.ListWdg, poslist[0]);
	    XmListAddItem(uiTopLevel.SBGfx.ListWdg, selectionstr, poslist[0]);
	    XmListSelectPos(uiTopLevel.SBGfx.ListWdg, poslist[0] + 1, FALSE);
	    XtFree(poslist);
	} else
	    XmListAddItem(uiTopLevel.SBGfx.ListWdg, selectionstr, 0);
	XmStringFree(selectionstr);
    }
    XtFree(selection);
    XtFree(fixedselection);

    uiselectionboxupdateconfig();
}


static void uiselectionboxdeletecb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmPushButtonCallbackStruct *calldata;
{
    int *poslist;
    int poscount;
    XmString dummystr;

    /* We should have only one selected item */
    if (XmListGetSelectedPos(uiTopLevel.SBGfx.ListWdg, &poslist, &poscount)) {
	XmListDeletePos(uiTopLevel.SBGfx.ListWdg, poslist[0]);
	XmListSelectPos(uiTopLevel.SBGfx.ListWdg, poslist[0], FALSE);
	XtFree(poslist);
    }
    if (!uiGetArg(uiTopLevel.SBGfx.ListWdg, XmNitemCount)) {
	dummystr = XmStringCreateSimple(" ");
	XmListAddItem(uiTopLevel.SBGfx.ListWdg, dummystr, 0);
	XmStringFree(dummystr);
	XtSetSensitive(uiTopLevel.SBGfx.ListWdg, FALSE);
	uidummy = TRUE;
    }
    uiselectionboxupdateconfig();
}


static void uiselectionboxclickcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    Widget textwdg;
    char *selection;

    XmStringGetLtoR(calldata->item, XmSTRING_DEFAULT_CHARSET, &selection);
    textwdg = XmFileSelectionBoxGetChild(uiTopLevel.SBGfx.FSBoxWdg,
					 XmDIALOG_TEXT);
    XmTextSetString(textwdg, selection);
    XtFree(selection);
}


static void uiselectionboxupdateconfig()
{
    Widget listwdg = uiTopLevel.SBGfx.ListWdg;
    int nitems, i;
    char **items;
    XmStringTable stringtable;
    void *table;

    nitems = uiGetArg(listwdg, XmNitemCount);
    items = (char **) uiMalloc((nitems + 1) * sizeof(*items));

    stringtable = (XmStringTable) uiGetArg(listwdg, XmNitems);
    for (i = 0; i < nitems; i++)
	XmStringGetLtoR(stringtable[i], XmSTRING_DEFAULT_CHARSET, &items[i]);
    items[nitems] = (char *) NULL;

    table = uiConfigPF((void *) NULL, C_DEFAULTS);
    (void) uiConfigSetPF(table, C_DEFAULTSTABLE, (void *) items);
}
