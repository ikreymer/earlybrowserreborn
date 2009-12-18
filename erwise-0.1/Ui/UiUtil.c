static char *rcsid = "$Id$";

#include "UiIncludes.h"


static void uiwarningokcb(Widget wdg, caddr_t callback,
			   XmListCallbackStruct * calldata);
static void uiwarningcancelcb(Widget wdg, caddr_t callback,
			       XmListCallbackStruct * calldata);

int UiDisplayWarningDialog(warning, callback)
char *warning;
void (*callback) (int button);
{
    Widget labelwdg;
    Widget separatorwdg;
    Widget okwdg;
    Widget cancelwdg;
    XmString warningstr;
    ArgList args;
    Cardinal nargs;
    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    Widget formwdg;
    Widget topwdg;
    char tmpstr[11];

    if (XQueryPointer(XtDisplay(uiTopLevel.TopGfx.TopWdg),
		      XtWindow(uiTopLevel.TopGfx.TopWdg), &root, &child,
		      &root_x, &root_y, &win_x, &win_y, &mask)) {
	sprintf(tmpstr, "+%d+%d", root_x - 50, root_y - 50);

	topwdg = XtCreateApplicationShell("Warning",
					  topLevelShellWidgetClass,
					  NULL, 0);
	XtVaSetValues(topwdg,
		      XmNgeometry, tmpstr, NULL);

	args = uiVaSetArgs(&nargs,
			   XmNresizePolicy, XmRESIZE_NONE,
			   XmNautoUnmanage, FALSE, NULL);
	formwdg = XmCreateForm(topwdg, "WarningDialog", args, nargs);

	warningstr = XmStringCreateSimple(warning);
	args = uiVaSetArgs(&nargs,
			   XmNlabelString, warningstr,
			   XmNtopAttachment, XmATTACH_FORM,
			   XmNtopOffset, UI_LIST_WDG_OFFSET,
			   XmNleftAttachment, XmATTACH_FORM,
			   XmNrightAttachment, XmATTACH_FORM, NULL);
	labelwdg = XmCreateLabelGadget(formwdg, "WarningLabel",
				       args, nargs);
	XtManageChild(labelwdg);

	args = uiVaSetArgs(&nargs,
			   XmNtopAttachment, XmATTACH_WIDGET,
			   XmNtopWidget, labelwdg,
			   XmNtopOffset, UI_LIST_WDG_OFFSET,
			   XmNleftAttachment, XmATTACH_FORM,
			   XmNrightAttachment, XmATTACH_FORM, NULL);
	separatorwdg = XmCreateSeparatorGadget(formwdg, "WarningSeparator",
					       args, nargs);
	XtManageChild(separatorwdg);

	args = uiVaSetArgs(&nargs,
			   XmNleftAttachment, XmATTACH_POSITION,
			   XmNleftPosition, 5,
			   XmNrightAttachment, XmATTACH_POSITION,
			   XmNrightPosition, 40,
			   XmNtopAttachment, XmATTACH_WIDGET,
			   XmNtopWidget, separatorwdg,
			   XmNtopOffset, UI_LIST_WDG_OFFSET,
			   XmNbottomAttachment, XmATTACH_FORM,
			   XmNbottomOffset, UI_LIST_WDG_OFFSET, NULL);
	okwdg = XmCreatePushButtonGadget(formwdg, "Ok", args, nargs);
	XtAddCallback(okwdg, XmNactivateCallback,
		      (XtCallbackProc) uiwarningokcb, (caddr_t) callback);
	XtManageChild(okwdg);

	if (callback) {
	    args = uiVaSetArgs(&nargs,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 60,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 95,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separatorwdg,
			       XmNtopOffset, UI_LIST_WDG_OFFSET,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNbottomOffset, UI_LIST_WDG_OFFSET, NULL);
	    cancelwdg = XmCreatePushButtonGadget(formwdg, "Cancel",
						 args, nargs);
	    XtAddCallback(cancelwdg, XmNactivateCallback,
			  (XtCallbackProc) uiwarningcancelcb,
			  (caddr_t) callback);
	    XtManageChild(cancelwdg);
	}
	XtManageChild(formwdg);
	XtRealizeWidget(XtParent(formwdg));

	return UI_OK;
    }
}


static void uiwarningokcb(wdg, callback, calldata)
Widget wdg;
caddr_t callback;
XmListCallbackStruct *calldata;
{
    int button = UI_OK;

    if (callback)
	(*(void (*) (int button)) callback) (UI_OK);

    XtUnmapWidget(XtParent(XtParent(wdg)));
}


static void uiwarningcancelcb(wdg, ignored, calldata)
Widget wdg;
caddr_t ignored;
XmListCallbackStruct *calldata;
{
    XtUnmapWidget(XtParent(XtParent(wdg)));
}
