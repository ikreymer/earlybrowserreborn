static char *rcsid = "$Id$";

#include "UiIncludes.h"

static Widget uicreatecpform();
static Widget
 uicreatecpbutton(Widget parentwdg, char *name, char *actionname,
		   int leftpos, int rightpos);
static Widget uicreatecpclosebutton(Widget parentwdg);
static Widget uicreatecplabel(Widget parentwdg, Widget rightwdg);
static Widget uicreatecplist(Widget parentwdg, Widget bottomwdg,
			      Widget topwdg);
static void uicpclosecb(char *address, HText_t * htext,
			 HTextObject_t * htextobject, void *parameter);
static void uicpclickcb(Widget wdg, caddr_t ignored,
			 XmListCallbackStruct * calldata);


uiTopLevel_t uiTopLevel;


int UiDisplayControlPanel()
{
    ArgList args;
    Cardinal nargs;
    uiTopLevelGfx_t *topgfx = &uiTopLevel.TopGfx;
    uiControlPanelGfx_t *controlpanelgfx = &uiTopLevel.ControlPanelGfx;

    if (controlpanelgfx->FormWdg) {
	XtMapWidget(XtParent(controlpanelgfx->FormWdg));

	uiWidgetPlacement(XtParent(controlpanelgfx->FormWdg),
			uiTopLevel.GlobalSettings.ControlPanelPlacement);

	return UI_OK;
    }
    args = uiVaSetArgs(&nargs,
		       XmNwidth, 280,
		       XmNheight, 180,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    controlpanelgfx->FormWdg = uicreatecpform();
    controlpanelgfx->SearchWdg = uicreatecpbutton(controlpanelgfx->FormWdg,
					      "Search", "Search", 0, 25);
    controlpanelgfx->HomeWdg = uicreatecpbutton(controlpanelgfx->FormWdg,
						"Home", "Home", 25, 50);
    controlpanelgfx->RecallWdg = uicreatecpbutton(controlpanelgfx->FormWdg,
					     "Recall", "Recall", 50, 75);
    controlpanelgfx->CloseWdg = uicreatecpbutton(controlpanelgfx->FormWdg,
						 "Close\nhierarchy",
					     "Close hierarchy", 75, 100);
    controlpanelgfx->DialogCloseWdg =
	uicreatecpclosebutton(controlpanelgfx->FormWdg);
    controlpanelgfx->LabelWdg =
	uicreatecplabel(controlpanelgfx->FormWdg,
			controlpanelgfx->DialogCloseWdg);
    controlpanelgfx->ListWdg = uicreatecplist(controlpanelgfx->FormWdg,
					      controlpanelgfx->SearchWdg,
					controlpanelgfx->DialogCloseWdg);
    XtManageChild(controlpanelgfx->FormWdg);
    XtRealizeWidget(XtParent(controlpanelgfx->FormWdg));

    uiWidgetPlacement(XtParent(controlpanelgfx->FormWdg),
		      uiTopLevel.GlobalSettings.ControlPanelPlacement);

    return UI_OK;
}


void uiControlPanelUpdateDialog()
{
    if (uiTopLevel.ControlPanelGfx.FormWdg) {
	if (uiPageInfo.CurrentPage) {
	} else
	    XtUnmapWidget(XtParent(uiTopLevel.ControlPanelGfx.FormWdg));
    }
}


static Widget
 uicreatecpform()
{
    ArgList args;
    Cardinal nargs;
    Widget formwdg;
    Widget topwdg;

    topwdg = XtCreateApplicationShell("ControlPanel",
				      topLevelShellWidgetClass,
				      NULL, 0);
    XtVaSetValues(topwdg,
		  XmNtitle, UI_CONTROL_TITLE, NULL);

    args = uiVaSetArgs(&nargs,
		       XmNresizePolicy, XmRESIZE_NONE,
		       XmNautoUnmanage, FALSE, NULL);
    formwdg = XmCreateForm(topwdg, "ControlPanel", args, nargs);

    return formwdg;
}


static Widget
 uicreatecpbutton(parentwdg, name, actionname, leftpos, rightpos)
Widget parentwdg;
char *name;
char *actionname;
int leftpos;
int rightpos;
{
    Widget tmpwdg;
    uiActionData_t *actiondata = uiMalloc(sizeof(*actiondata));

    tmpwdg = XmCreatePushButtonGadget(parentwdg, name,
				      (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNheight, 65,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_POSITION,
		  XmNleftPosition, leftpos,
		  XmNrightAttachment, XmATTACH_POSITION,
		  XmNrightPosition, rightpos, NULL);
    actiondata->ActionName = actionname;
    actiondata->Page = uiPageInfo.CurrentPage;
    XtAddCallback(tmpwdg, XmNactivateCallback,
	      (XtCallbackProc) uiDialogActivateCB, (caddr_t) actiondata);
    XtManageChild(tmpwdg);

    return tmpwdg;
}


static Widget
 uicreatecpclosebutton(parentwdg)
Widget parentwdg;
{
    Widget tmpwdg;
    static uiActionData_t actiondata;

    tmpwdg = XmCreatePushButtonGadget(parentwdg, "Close", (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNwidth, 70,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNbottomOffset, UI_CP_WDG_OFFSET,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNrightOffset, UI_CP_WDG_OFFSET, NULL);
    actiondata.ActionName = "CpClose";
    actiondata.Page = uiPageInfo.CurrentPage;
    XtAddCallback(tmpwdg, XmNactivateCallback,
	    (XtCallbackProc) uiDialogActivateCB, (caddr_t) & actiondata);
    UiAttachCallback("CpClose", uicpclosecb, "CpClose");

    XtManageChild(tmpwdg);

    return tmpwdg;
}


static Widget
 uicreatecplabel(parentwdg, rightwdg)
Widget parentwdg;
Widget rightwdg;
{
    Widget tmpwdg;

    tmpwdg = XmCreateLabelGadget(parentwdg, "ControlPanelLabel",
				 (ArgList) NULL, 0);
    XtVaSetValues(tmpwdg,
		  XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
		  XmNbottomWidget, rightwdg,
		  XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
		  XmNtopWidget, rightwdg,
		  XmNrightAttachment, XmATTACH_WIDGET,
		  XmNrightWidget, rightwdg,
		  XmNrightOffset, UI_CP_WDG_OFFSET,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNleftOffset, UI_CP_WDG_OFFSET, NULL);

    XtManageChild(tmpwdg);

    return tmpwdg;
}


static Widget
 uicreatecplist(formwdg, topwdg, bottomwdg)
Widget formwdg;
Widget topwdg;
Widget bottomwdg;
{
    ArgList args;
    Cardinal nargs;
    Widget listwdg;
    uiHierarchy_t *hierarchy = uiTopLevel.Hierarchies;
    XmString dummystr;

    args = uiVaSetArgs(&nargs,
		       XmNvisibleItemCount, 5,
		       XmNselectionPolicy, XmSINGLE_SELECT,
		       XmNlistSizePolicy, XmCONSTANT,
		       XmNscrollBarDisplayPolicy, XmSTATIC,
		       XmNtopAttachment, XmATTACH_WIDGET,
		       XmNtopWidget, topwdg,
		       XmNtopOffset, UI_CP_WDG_OFFSET,
		       XmNbottomAttachment, XmATTACH_WIDGET,
		       XmNbottomWidget, bottomwdg,
		       XmNbottomOffset, UI_CP_WDG_OFFSET,
		       XmNrightAttachment, XmATTACH_FORM,
		       XmNrightOffset, UI_CP_WDG_OFFSET,
		       XmNleftAttachment, XmATTACH_FORM,
		       XmNleftOffset, UI_CP_WDG_OFFSET, NULL);
    listwdg = XmCreateScrolledList(formwdg, "CpList", args, nargs);
    XtManageChild(listwdg);

    while (hierarchy) {
	dummystr = XmStringCreateSimple(hierarchy->Address);
	XmListAddItem(listwdg, dummystr, 0);
	XmStringFree(dummystr);
	hierarchy = hierarchy->Next;
    }

    return listwdg;
}


static void uicpclickcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    char *selection;

    XmStringGetLtoR(calldata->item, XmSTRING_DEFAULT_CHARSET, &selection);
}


static void uicpclosecb(address, htext, htextobject, parameter)
char *address;
HText_t *htext;
HTextObject_t *htextobject;
void *parameter;
{
    XtUnmapWidget(XtParent(uiTopLevel.ControlPanelGfx.FormWdg));
}
